#include "NexusDownloadModalContent.h"
#include "FileSelectionModalContent.h"
#include "../MessageModal.h"
#include "../ModalManager.h"
#include "../../utils/NexusModsClient.h"
#include "../../utils/NexusModsAuth.h"
#include "../../utils/NexusUrlParser.h"
#include "../../utils/Theme.h"
#include <algorithm>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

NexusDownloadModalContent::NexusDownloadModalContent(NexusModsClient *client,
                                                     NexusModsAuth *auth,
                                                     ModalManager *modalManager,
                                                     QWidget *parent)
    : BaseModalContent(parent)
    , m_client(client)
    , m_auth(auth)
    , m_modalManager(modalManager)
    , m_currentDownloadIndex(0)
{
    setTitle("Download from Nexus Mods");
    setupUi();
    setPreferredSize(QSize(500, 350));

    connect(m_client, &NexusModsClient::modInfoReceived, this, &NexusDownloadModalContent::onModInfoReceived);
    connect(m_client, &NexusModsClient::modFilesReceived, this, &NexusDownloadModalContent::onModFilesReceived);
    connect(m_client, &NexusModsClient::downloadLinkReceived, this, &NexusDownloadModalContent::onDownloadLinkReceived);
    connect(m_client, &NexusModsClient::downloadProgress, this, &NexusDownloadModalContent::onDownloadProgress);
    connect(m_client, &NexusModsClient::downloadFinished, this, &NexusDownloadModalContent::onDownloadFinished);
    connect(m_client, &NexusModsClient::errorOccurred, this, &NexusDownloadModalContent::onError);

    connect(m_auth, &NexusModsAuth::authenticationStarted, this, &NexusDownloadModalContent::onAuthStarted, Qt::UniqueConnection);
    connect(m_auth, &NexusModsAuth::authenticationComplete, this, &NexusDownloadModalContent::onAuthComplete, Qt::UniqueConnection);
    connect(m_auth, &NexusModsAuth::authenticationFailed, this, &NexusDownloadModalContent::onAuthFailed, Qt::UniqueConnection);
}

void NexusDownloadModalContent::setupUi() {
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createInputPage());
    m_stack->addWidget(createAuthPage());
    m_stack->addWidget(createDownloadPage());

    bodyLayout()->addWidget(m_stack);

    m_downloadButton = new QPushButton("Download", this);
    m_downloadButton->setDefault(true);
    connect(m_downloadButton, &QPushButton::clicked, this, &NexusDownloadModalContent::onDownloadClicked);
    footerLayout()->addWidget(m_downloadButton);

    m_authenticateButton = new QPushButton("Authenticate", this);
    m_authenticateButton->setDefault(true);
    connect(m_authenticateButton, &QPushButton::clicked, this, &NexusDownloadModalContent::onAuthenticateClicked);
    footerLayout()->addWidget(m_authenticateButton);

    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setAutoDefault(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &NexusDownloadModalContent::reject);
    footerLayout()->addWidget(m_cancelButton);

    showInputPage();
}

QWidget* NexusDownloadModalContent::createInputPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *instructionLabel = new QLabel("Enter Nexus Mods URL for a Foxhole mod:", page);
    instructionLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                                   .arg(Theme::Colors::TEXT_SECONDARY));
    layout->addWidget(instructionLabel);

    m_urlEdit = new QLineEdit(page);
    m_urlEdit->setPlaceholderText("https://www.nexusmods.com/foxhole/mods/...");
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &NexusDownloadModalContent::onDownloadClicked);
    layout->addWidget(m_urlEdit);

    layout->addStretch();

    return page;
}

QWidget* NexusDownloadModalContent::createAuthPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *instructionLabel = new QLabel("Authentication required to access Nexus Mods API.", page);
    instructionLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                                   .arg(Theme::Colors::TEXT_SECONDARY));
    layout->addWidget(instructionLabel);

    m_authStatusLabel = new QLabel("Click Authenticate to begin...", page);
    m_authStatusLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 12px; }")
                                    .arg(Theme::Colors::TEXT_MUTED));
    m_authStatusLabel->setWordWrap(true);
    layout->addWidget(m_authStatusLabel);

    layout->addStretch();

    return page;
}

QWidget* NexusDownloadModalContent::createDownloadPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    m_statusLabel = new QLabel("Preparing download...", page);
    m_statusLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                                .arg(Theme::Colors::TEXT_SECONDARY));
    layout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(page);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    layout->addWidget(m_progressBar);

    layout->addStretch();

    return page;
}

void NexusDownloadModalContent::showInputPage() {
    m_stack->setCurrentIndex(InputPage);
    updateFooterButtons();
}

void NexusDownloadModalContent::showAuthPage() {
    m_authStatusLabel->setText("Click Authenticate to begin...");
    m_authenticateButton->setEnabled(true);
    m_stack->setCurrentIndex(AuthPage);
    updateFooterButtons();
}

void NexusDownloadModalContent::showDownloadPage() {
    m_progressBar->setValue(0);
    m_statusLabel->setText("Preparing download...");
    m_stack->setCurrentIndex(DownloadPage);
    updateFooterButtons();
}

void NexusDownloadModalContent::updateFooterButtons() {
    int currentPage = m_stack->currentIndex();

    m_downloadButton->setVisible(currentPage == InputPage);
    m_authenticateButton->setVisible(currentPage == AuthPage);
    m_cancelButton->setVisible(true);
}

void NexusDownloadModalContent::onDownloadClicked() {
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        MessageModal::warning(m_modalManager, "Error", "Please enter a URL");
        return;
    }

    auto result = NexusUrlParser::parseUrl(url);
    if (!result.isValid) {
        MessageModal::warning(m_modalManager, "Invalid URL", result.error);
        return;
    }

    m_currentModId = result.modId;
    m_currentFileId = result.fileId;
    m_pendingUrl = url;

    if (!m_client->hasApiKey()) {
        showAuthPage();
        return;
    }

    startDownloadProcess();
}

void NexusDownloadModalContent::onAuthenticateClicked() {
    m_authenticateButton->setEnabled(false);
    m_authStatusLabel->setText("Connecting to authentication server...");
    m_auth->startAuthentication();
}

void NexusDownloadModalContent::onAuthStarted(const QString &browserUrl) {
    m_authStatusLabel->setText(QString("Opening browser for authentication...\n\nIf browser doesn't open, visit:\n%1").arg(browserUrl));
    QDesktopServices::openUrl(QUrl(browserUrl));
}

void NexusDownloadModalContent::onAuthComplete(const QString &apiKey) {
    m_client->setApiKey(apiKey);
    m_authStatusLabel->setText("Authentication successful!");
    startDownloadProcess();
}

void NexusDownloadModalContent::onAuthFailed(const QString &error) {
    m_authStatusLabel->setText(QString("Authentication failed: %1").arg(error));
    m_authenticateButton->setEnabled(true);
}

void NexusDownloadModalContent::startDownloadProcess() {
    showDownloadPage();
    m_statusLabel->setText("Fetching mod information...");
    m_client->getModInfo(m_currentModId);
}

void NexusDownloadModalContent::onModInfoReceived(const QString &author, const QString &description, const QString &version) {
    m_author = author;
    m_description = description;

    m_statusLabel->setText("Fetching file list...");
    m_client->getModFiles(m_currentModId);
}

void NexusDownloadModalContent::onModFilesReceived(const QList<NexusFileInfo> &files) {
    QList<NexusFileInfo> filtered;
    for (const NexusFileInfo &file : files) {
        if (file.categoryName.toUpper() != "ARCHIVED") {
            filtered.append(file);
        }
    }

    if (filtered.isEmpty()) {
        MessageModal::warning(m_modalManager, "Error", "No files found for this mod");
        showInputPage();
        return;
    }

    std::sort(filtered.begin(), filtered.end(), [this](const NexusFileInfo &a, const NexusFileInfo &b) {
        auto getPriority = [](const QString &category) -> int {
            QString cat = category.toUpper();
            if (cat == "MAIN") return 1;
            if (cat == "UPDATE") return 2;
            if (cat == "OPTIONAL") return 3;
            if (cat == "MISCELLANEOUS") return 4;
            if (cat == "OLD_VERSION") return 5;
            return 6;
        };

        int priorityA = getPriority(a.categoryName);
        int priorityB = getPriority(b.categoryName);

        if (priorityA != priorityB) {
            return priorityA < priorityB;
        }

        return a.uploadedTime > b.uploadedTime;
    });

    QList<FileItem> items;
    for (const NexusFileInfo &file : filtered) {
        QString displayText = file.name;

        if (!file.version.isEmpty()) {
            displayText += QString(" (v%1)").arg(file.version);
        }

        if (file.sizeBytes > 0) {
            displayText += QString(" - %1").arg(formatFileSize(file.sizeBytes));
        }

        if (!file.categoryName.isEmpty()) {
            displayText += QString(" [%1]").arg(file.categoryName);
        }

        items.append({file.id, displayText});
    }

    auto *selectionModal = new FileSelectionModalContent(
        items,
        QString("Select Files - %1").arg(m_author),
        "Multiple files found. Select one or more (Ctrl+Click or Shift+Click):",
        true
    );

    connect(selectionModal, &FileSelectionModalContent::accepted, this, [this, selectionModal, filtered]() {
        m_selectedFileIds = selectionModal->getSelectedIds();
        if (m_selectedFileIds.isEmpty()) {
            showInputPage();
            return;
        }

        m_selectedFiles.clear();
        for (const QString &fileId : m_selectedFileIds) {
            for (const NexusFileInfo &file : filtered) {
                if (file.id == fileId) {
                    m_selectedFiles.append(file);
                    break;
                }
            }
        }

        m_currentDownloadIndex = 0;
        m_downloadedFilePaths.clear();
        m_downloadedFiles.clear();
        startNextDownload();
    });

    connect(selectionModal, &FileSelectionModalContent::rejected, this, [this]() {
        showInputPage();
    });

    m_modalManager->showModal(selectionModal);
}

void NexusDownloadModalContent::onDownloadLinkReceived(const QString &url) {
    QString fileName = QString("nexus_mod_%1_%2.tmp").arg(m_currentModId, m_currentFileId);
    QString savePath = generateTempPath(fileName);

    m_statusLabel->setText("Downloading file...");
    m_client->downloadFile(QUrl(url), savePath);
}

void NexusDownloadModalContent::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        int percentage = static_cast<int>((received * 100) / total);
        m_progressBar->setValue(percentage);

        qint64 receivedMB = received / (1024 * 1024);
        qint64 totalMB = total / (1024 * 1024);

        QString statusText = QString("Downloading: %1 MB / %2 MB").arg(receivedMB).arg(totalMB);
        if (m_selectedFiles.size() > 1) {
            statusText += QString(" (File %1 of %2)").arg(m_currentDownloadIndex + 1).arg(m_selectedFiles.size());
        }
        m_statusLabel->setText(statusText);
    }
}

void NexusDownloadModalContent::onDownloadFinished(const QString &savePath) {
    m_downloadedFilePaths.append(savePath);

    if (m_currentDownloadIndex < m_selectedFiles.size()) {
        m_downloadedFiles.append(m_selectedFiles[m_currentDownloadIndex]);
    }

    m_currentDownloadIndex++;

    if (m_currentDownloadIndex < m_selectedFiles.size()) {
        startNextDownload();
    } else {
        m_statusLabel->setText("All downloads complete!");
        m_progressBar->setValue(100);
        accept();
    }
}

void NexusDownloadModalContent::onError(const QString &error) {
    if (error == "PREMIUM_REQUIRED") {
        if (m_selectedFiles.size() > 1) {
            MessageModal::warning(m_modalManager, "Error",
                "Direct downloads via API require a Nexus Mods Premium account.\n\n"
                "Multi-file downloads are not supported for manual downloads.\n"
                "Please download files individually.");
            showInputPage();
            return;
        }

        QString modUrl = QString("https://www.nexusmods.com/foxhole/mods/%1?tab=files&file_id=%2")
                            .arg(m_currentModId, m_currentFileId);

        auto *modal = new MessageModal(
            "Premium Required",
            "Direct downloads via API require a Nexus Mods Premium account.\n\n"
            "Your browser will open to the download page where you can download the file manually.\n\n"
            "The mod will be installed with Nexus metadata for future updates.",
            MessageModal::Information,
            MessageModal::Ok | MessageModal::Cancel
        );

        connect(modal, &MessageModal::finished, this, [this, modUrl, modal]() {
            if (modal->clickedButton() == MessageModal::Ok) {
                QDesktopServices::openUrl(QUrl(modUrl));

                auto *selectModal = new MessageModal(
                    "Select Downloaded File",
                    "Please download the file from your browser.\n\n"
                    "Once the download is complete, click OK to locate the file.",
                    MessageModal::Information,
                    MessageModal::Ok | MessageModal::Cancel
                );

                connect(selectModal, &MessageModal::finished, this, [this, selectModal]() {
                    if (selectModal->clickedButton() == MessageModal::Ok) {
                        QString filePath = QFileDialog::getOpenFileName(
                            this,
                            "Select Downloaded File",
                            QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
                            "Mod Files (*.pak *.zip *.rar *.7z *.tar.gz *.tar.bz2 *.tar.xz);;"
                            "Pak Files (*.pak);;"
                            "Archive Files (*.zip *.rar *.7z *.tar.gz *.tar.bz2 *.tar.xz);;"
                            "All Files (*.*)"
                        );

                        if (!filePath.isEmpty()) {
                            m_downloadedFilePaths.clear();
                            m_downloadedFilePaths.append(filePath);
                            m_downloadedFiles.clear();
                            if (!m_selectedFiles.isEmpty()) {
                                m_downloadedFiles.append(m_selectedFiles.first());
                            }
                            accept();
                        } else {
                            reject();
                        }
                    } else {
                        reject();
                    }
                });

                m_modalManager->showModal(selectModal);
            } else {
                reject();
            }
        });

        m_modalManager->showModal(modal);
    } else {
        MessageModal::warning(m_modalManager, "Error", error);
        showInputPage();
    }
}

QString NexusDownloadModalContent::generateTempPath(const QString &fileName) const {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(tempDir).filePath(fileName);
}

void NexusDownloadModalContent::startNextDownload() {
    if (m_currentDownloadIndex >= m_selectedFiles.size()) {
        return;
    }

    const NexusFileInfo &file = m_selectedFiles[m_currentDownloadIndex];
    m_currentFileId = file.id;

    m_progressBar->setValue(0);

    if (m_selectedFiles.size() > 1) {
        m_statusLabel->setText(QString("Getting download link for file %1 of %2...")
            .arg(m_currentDownloadIndex + 1).arg(m_selectedFiles.size()));
    } else {
        m_statusLabel->setText("Getting download link...");
    }

    m_client->getDownloadLink(m_currentModId, m_currentFileId);
}

QString NexusDownloadModalContent::formatFileSize(qint64 bytes) const {
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 2);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 2);
    } else {
        return QString("%1 bytes").arg(bytes);
    }
}
