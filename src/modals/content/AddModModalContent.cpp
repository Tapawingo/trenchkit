#include "AddModModalContent.h"
#include "FileSelectionModalContent.h"
#include "NexusDownloadModalContent.h"
#include "ItchDownloadModalContent.h"
#include "../MessageModal.h"
#include "../ModalManager.h"
#include "../../utils/ModManager.h"
#include "../../utils/NexusModsClient.h"
#include "../../utils/NexusModsAuth.h"
#include "../../utils/ItchClient.h"
#include "../../utils/ItchAuth.h"
#include "../../utils/ArchiveExtractor.h"
#include "../../utils/Theme.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>

AddModModalContent::AddModModalContent(ModManager *modManager,
                                       NexusModsClient *nexusClient,
                                       NexusModsAuth *nexusAuth,
                                       ItchClient *itchClient,
                                       ItchAuth *itchAuth,
                                       ModalManager *modalManager,
                                       QWidget *parent)
    : BaseModalContent(parent)
    , m_modManager(modManager)
    , m_nexusClient(nexusClient)
    , m_nexusAuth(nexusAuth)
    , m_itchClient(itchClient)
    , m_itchAuth(itchAuth)
    , m_modalManager(modalManager)
{
    setTitle("Add Mod");
    setupUi();
    setPreferredSize(QSize(350, 280));

    if (!m_modManager) {
        m_fromFileButton->setEnabled(false);
        m_fromNexusButton->setEnabled(false);
        m_fromItchButton->setEnabled(false);
    } else {
        m_fromNexusButton->setEnabled(m_nexusClient && m_nexusAuth);
        m_fromItchButton->setEnabled(m_itchClient && m_itchAuth);
    }
}

void AddModModalContent::setupUi() {
    bodyLayout()->setSpacing(16);

    m_fromFileButton = new QPushButton("From File", this);
    m_fromFileButton->setMinimumHeight(40);
    m_fromFileButton->setCursor(Qt::PointingHandCursor);
    connect(m_fromFileButton, &QPushButton::clicked, this, &AddModModalContent::onFromFileClicked);
    bodyLayout()->addWidget(m_fromFileButton);

    m_fromNexusButton = new QPushButton("From Nexus Mods", this);
    m_fromNexusButton->setMinimumHeight(40);
    m_fromNexusButton->setCursor(Qt::PointingHandCursor);
    connect(m_fromNexusButton, &QPushButton::clicked, this, &AddModModalContent::onFromNexusClicked);
    bodyLayout()->addWidget(m_fromNexusButton);

    m_fromItchButton = new QPushButton("From itch.io", this);
    m_fromItchButton->setMinimumHeight(40);
    m_fromItchButton->setCursor(Qt::PointingHandCursor);
    connect(m_fromItchButton, &QPushButton::clicked, this, &AddModModalContent::onFromItchClicked);
    bodyLayout()->addWidget(m_fromItchButton);

    bodyLayout()->addStretch();

    auto *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setCursor(Qt::PointingHandCursor);
    connect(cancelButton, &QPushButton::clicked, this, &AddModModalContent::reject);
    footerLayout()->addWidget(cancelButton);
}

void AddModModalContent::onFromFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Mod File",
        QString(),
        "Mod Files (*.pak *.zip);;Pak Files (*.pak);;Zip Files (*.zip);;All Files (*.*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    if (filePath.endsWith(".zip", Qt::CaseInsensitive)) {
        handleZipFile(filePath);
    } else {
        handlePakFile(filePath);
    }
}

void AddModModalContent::handleZipFile(const QString &zipPath, const QString &nexusModId, const QString &nexusFileId,
                                       const QString &author, const QString &description, const QString &version, const QString &itchGameId, const QDateTime &uploadDate) {
    ArchiveExtractor extractor;
    auto result = extractor.extractPakFiles(zipPath);

    if (!result.success) {
        MessageModal::warning(m_modalManager, "Error", result.error);
        return;
    }

    if (result.pakFiles.isEmpty()) {
        MessageModal::warning(m_modalManager, "Error", "No .pak files found in archive");
        ArchiveExtractor::cleanupTempDir(result.tempDir);
        return;
    }

    QStringList selectedPaks;

    if (result.pakFiles.size() == 1) {
        selectedPaks.append(result.pakFiles.first());

        for (const QString &pakPath : selectedPaks) {
            handlePakFile(pakPath, nexusModId, nexusFileId, author, description, version, itchGameId, QString(), uploadDate);
        }

        ArchiveExtractor::cleanupTempDir(result.tempDir);

        if (!selectedPaks.isEmpty()) {
            accept();
        }
    } else {
        QStringList fileNames;
        for (const QString &path : result.pakFiles) {
            fileNames.append(QFileInfo(path).fileName());
        }

        auto *fileModal = new FileSelectionModalContent(fileNames, QFileInfo(zipPath).fileName(), true);
        connect(fileModal, &FileSelectionModalContent::accepted, this, [this, fileModal, result, nexusModId, nexusFileId, author, description, version, itchGameId, uploadDate]() {
            QStringList selectedFileNames = fileModal->getSelectedFiles();
            QStringList selectedPaks;

            for (const QString &fileName : selectedFileNames) {
                for (const QString &path : result.pakFiles) {
                    if (QFileInfo(path).fileName() == fileName) {
                        selectedPaks.append(path);
                        break;
                    }
                }
            }

            for (const QString &pakPath : selectedPaks) {
                handlePakFile(pakPath, nexusModId, nexusFileId, author, description, version, itchGameId, QString(), uploadDate);
            }

            ArchiveExtractor::cleanupTempDir(result.tempDir);

            if (!selectedPaks.isEmpty()) {
                accept();
            }
        });
        connect(fileModal, &FileSelectionModalContent::rejected, this, [result]() {
            ArchiveExtractor::cleanupTempDir(result.tempDir);
        });
        m_modalManager->showModal(fileModal);
    }
}

void AddModModalContent::handlePakFile(const QString &pakPath, const QString &nexusModId, const QString &nexusFileId,
                                       const QString &author, const QString &description, const QString &version, const QString &itchGameId, const QString &customModName, const QDateTime &uploadDate) {
    QString modName;
    if (!customModName.isEmpty()) {
        modName = customModName;
    } else {
        QFileInfo fileInfo(pakPath);
        modName = fileInfo.baseName();
    }

    if (!m_modManager->addMod(pakPath, modName, nexusModId, nexusFileId, author, description, version, itchGameId, uploadDate)) {
        MessageModal::warning(m_modalManager, "Error", "Failed to add mod: " + modName);
    } else {
        emit modAdded(modName);
    }
}

void AddModModalContent::onFromNexusClicked() {
    if (!m_nexusClient || !m_nexusAuth) {
        return;
    }

    auto *nexusModal = new NexusDownloadModalContent(m_nexusClient, m_nexusAuth, m_modalManager);
    connect(nexusModal, &NexusDownloadModalContent::accepted, this, [this, nexusModal]() {
        QStringList filePaths = nexusModal->getDownloadedFilePaths();
        if (filePaths.isEmpty()) {
            return;
        }

        QString nexusModId = nexusModal->getModId();
        QString author = nexusModal->getAuthor();
        QString description = nexusModal->getDescription();
        QList<NexusFileInfo> files = nexusModal->getDownloadedFiles();

        for (int i = 0; i < filePaths.size(); ++i) {
            const QString &filePath = filePaths[i];

            QString nexusFileId;
            QString version;
            if (i < files.size()) {
                nexusFileId = files[i].id;
                version = files[i].version;
            }

            if (filePath.endsWith(".zip", Qt::CaseInsensitive)) {
                handleZipFile(filePath, nexusModId, nexusFileId, author, description, version);
            } else {
                handlePakFile(filePath, nexusModId, nexusFileId, author, description, version);
            }

            if (filePath.contains("nexus_mod_")) {
                QFile::remove(filePath);
            }
        }

        accept();
    });
    m_modalManager->showModal(nexusModal);
}

void AddModModalContent::onFromItchClicked() {
    if (!m_itchClient || !m_itchAuth) {
        return;
    }

    auto *itchModal = new ItchDownloadModalContent(m_itchClient, m_itchAuth, m_modalManager);
    connect(itchModal, &ItchDownloadModalContent::accepted, this, [this, itchModal]() {
        QStringList filePaths = itchModal->getDownloadedFilePaths();
        if (filePaths.isEmpty()) {
            return;
        }

        QString itchGameId = itchModal->getGameId();
        QString author = itchModal->getAuthor();
        QString gameTitle = itchModal->getGameTitle();
        QList<ItchUploadInfo> uploads = itchModal->getDownloadedUploads();

        for (int i = 0; i < filePaths.size(); ++i) {
            const QString &filePath = filePaths[i];
            QFileInfo fileInfo(filePath);
            QString fileName = fileInfo.fileName();
            QString modName;

            if (fileName.startsWith("itch_game_")) {
                int secondUnderscore = fileName.indexOf('_', 10);
                int thirdUnderscore = fileName.indexOf('_', secondUnderscore + 1);
                if (thirdUnderscore != -1) {
                    modName = fileName.mid(thirdUnderscore + 1);
                    modName = QFileInfo(modName).baseName();
                }
            }

            QDateTime uploadDate;
            if (i < uploads.size()) {
                uploadDate = uploads[i].updatedAt.isValid() ? uploads[i].updatedAt : uploads[i].createdAt;
            }

            if (filePath.endsWith(".zip", Qt::CaseInsensitive)) {
                handleZipFile(filePath, "", "", author, gameTitle, "", itchGameId, uploadDate);
            } else {
                handlePakFile(filePath, "", "", author, gameTitle, "", itchGameId, modName, uploadDate);
            }

            if (filePath.contains("itch_game_")) {
                QFile::remove(filePath);
            }
        }

        accept();
    });
    m_modalManager->showModal(itchModal);
}
