#include "AddModDialog.h"
#include "FileSelectionDialog.h"
#include "NexusDownloadDialog.h"
#include "../utils/ModManager.h"
#include "../utils/NexusModsClient.h"
#include "../utils/NexusModsAuth.h"
#include "../utils/ArchiveExtractor.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>

AddModDialog::AddModDialog(ModManager *modManager,
                           NexusModsClient *nexusClient,
                           NexusModsAuth *nexusAuth,
                           QWidget *parent)
    : QDialog(parent)
    , m_modManager(modManager)
    , m_nexusClient(nexusClient)
    , m_nexusAuth(nexusAuth)
    , m_fromFileButton(new QPushButton("From File", this))
    , m_fromNexusButton(new QPushButton("From Nexus Mods", this))
    , m_fromItchButton(new QPushButton("From itch.io", this))
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Cancel, this))
{
    setupUi();

    connect(m_fromFileButton, &QPushButton::clicked, this, &AddModDialog::onFromFileClicked);
    connect(m_fromNexusButton, &QPushButton::clicked, this, &AddModDialog::onFromNexusClicked);
    connect(m_fromItchButton, &QPushButton::clicked, this, &AddModDialog::onFromItchClicked);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (!m_modManager) {
        m_fromFileButton->setEnabled(false);
        m_fromNexusButton->setEnabled(false);
        m_fromItchButton->setEnabled(false);
    } else {
        m_fromNexusButton->setEnabled(m_nexusClient && m_nexusAuth);
    }
}

void AddModDialog::setupUi() {
    setWindowTitle("Add Mod");
    setMinimumWidth(300);
    setMinimumHeight(100);

    auto *layout = new QVBoxLayout(this);

    layout->addWidget(m_fromFileButton);
    layout->addWidget(m_fromNexusButton);

    m_fromItchButton->setEnabled(false);
    layout->addWidget(m_fromItchButton);

    layout->addStretch();
    layout->addWidget(m_buttonBox);
}

void AddModDialog::onFromFileClicked() {
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

void AddModDialog::handleZipFile(const QString &zipPath, const QString &nexusModId, const QString &nexusFileId,
                                  const QString &author, const QString &description, const QString &version) {
    ArchiveExtractor extractor;
    auto result = extractor.extractPakFiles(zipPath);

    if (!result.success) {
        QMessageBox::warning(this, "Error", result.error);
        return;
    }

    if (result.pakFiles.isEmpty()) {
        QMessageBox::warning(this, "Error", "No .pak files found in archive");
        ArchiveExtractor::cleanupTempDir(result.tempDir);
        return;
    }

    QStringList selectedPaks;

    if (result.pakFiles.size() == 1) {
        selectedPaks.append(result.pakFiles.first());
    } else {
        QStringList fileNames;
        for (const QString &path : result.pakFiles) {
            fileNames.append(QFileInfo(path).fileName());
        }

        FileSelectionDialog fileDialog(fileNames, QFileInfo(zipPath).fileName(), true, this);
        if (fileDialog.exec() == QDialog::Accepted) {
            QStringList selectedFileNames = fileDialog.getSelectedFiles();

            for (const QString &fileName : selectedFileNames) {
                for (const QString &path : result.pakFiles) {
                    if (QFileInfo(path).fileName() == fileName) {
                        selectedPaks.append(path);
                        break;
                    }
                }
            }
        }
    }

    for (const QString &pakPath : selectedPaks) {
        handlePakFile(pakPath, nexusModId, nexusFileId, author, description, version);
    }

    ArchiveExtractor::cleanupTempDir(result.tempDir);

    if (!selectedPaks.isEmpty()) {
        accept();
    }
}

void AddModDialog::handlePakFile(const QString &pakPath, const QString &nexusModId, const QString &nexusFileId,
                                  const QString &author, const QString &description, const QString &version) {
    QFileInfo fileInfo(pakPath);
    QString modName = fileInfo.baseName();

    if (!m_modManager->addMod(pakPath, modName, nexusModId, nexusFileId, author, description, version)) {
        QMessageBox::warning(this, "Error", "Failed to add mod: " + modName);
    } else {
        emit modAdded(modName);
    }
}

void AddModDialog::onFromNexusClicked() {
    if (!m_nexusClient || !m_nexusAuth) {
        return;
    }

    NexusDownloadDialog dialog(m_nexusClient, m_nexusAuth, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString filePath = dialog.getDownloadedFilePath();
    if (filePath.isEmpty()) {
        return;
    }

    QString nexusModId = dialog.getModId();
    QString nexusFileId = dialog.getFileId();
    QString author = dialog.getAuthor();
    QString description = dialog.getDescription();
    QString version = dialog.getVersion();

    if (filePath.endsWith(".zip", Qt::CaseInsensitive)) {
        handleZipFile(filePath, nexusModId, nexusFileId, author, description, version);
    } else {
        handlePakFile(filePath, nexusModId, nexusFileId, author, description, version);
    }

    if (filePath.contains(QStringLiteral("nexus_mod_"))) {
        QFile::remove(filePath);
    }

    accept();
}

void AddModDialog::onFromItchClicked() {
    // Future implementation
}
