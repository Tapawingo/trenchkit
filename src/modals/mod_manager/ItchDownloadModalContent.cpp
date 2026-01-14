#include "ItchDownloadModalContent.h"
#include "FileSelectionModalContent.h"
#include "common/modals/MessageModal.h"
#include <algorithm>
#include "common/modals/ModalManager.h"
#include "core/api/ItchClient.h"
#include "core/api/ItchAuth.h"
#include "core/utils/ItchUrlParser.h"
#include "core/utils/Theme.h"
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QRegularExpression>

ItchDownloadModalContent::ItchDownloadModalContent(ItchClient *client,
                                                   ItchAuth *auth,
                                                   ModalManager *modalManager,
                                                   QWidget *parent)
    : BaseModalContent(parent)
    , m_client(client)
    , m_auth(auth)
    , m_modalManager(modalManager)
{
    setTitle("Download from itch.io");
    setupUi();
    setPreferredSize(QSize(500, 350));

    connect(m_client, &ItchClient::gameIdReceived, this, &ItchDownloadModalContent::onGameIdReceived);
    connect(m_client, &ItchClient::uploadsReceived, this, &ItchDownloadModalContent::onUploadsReceived);
    connect(m_client, &ItchClient::downloadLinkReceived, this, &ItchDownloadModalContent::onDownloadLinkReceived);
    connect(m_client, &ItchClient::downloadProgress, this, &ItchDownloadModalContent::onDownloadProgress);
    connect(m_client, &ItchClient::downloadFinished, this, &ItchDownloadModalContent::onDownloadFinished);
    connect(m_client, &ItchClient::errorOccurred, this, &ItchDownloadModalContent::onError);
}

void ItchDownloadModalContent::setupUi() {
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createInputPage());
    m_stack->addWidget(createAuthPage());
    m_stack->addWidget(createDownloadPage());

    bodyLayout()->addWidget(m_stack);

    m_downloadButton = new QPushButton("Download", this);
    m_downloadButton->setDefault(true);
    connect(m_downloadButton, &QPushButton::clicked, this, &ItchDownloadModalContent::onDownloadClicked);
    footerLayout()->addWidget(m_downloadButton);

    m_submitApiKeyButton = new QPushButton("Submit", this);
    m_submitApiKeyButton->setDefault(true);
    connect(m_submitApiKeyButton, &QPushButton::clicked, this, &ItchDownloadModalContent::onApiKeySubmit);
    footerLayout()->addWidget(m_submitApiKeyButton);

    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setAutoDefault(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &ItchDownloadModalContent::reject);
    footerLayout()->addWidget(m_cancelButton);

    showInputPage();
}

QWidget* ItchDownloadModalContent::createInputPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *instructionLabel = new QLabel("Enter itch.io URL:", page);
    instructionLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                                   .arg(Theme::Colors::TEXT_SECONDARY));
    layout->addWidget(instructionLabel);

    m_urlEdit = new QLineEdit(page);
    m_urlEdit->setPlaceholderText("https://creator.itch.io/game-name");
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &ItchDownloadModalContent::onDownloadClicked);
    layout->addWidget(m_urlEdit);

    layout->addStretch();

    return page;
}

QWidget* ItchDownloadModalContent::createAuthPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *titleLabel = new QLabel("<b>API Key Required</b>", page);
    titleLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                             .arg(Theme::Colors::TEXT_SECONDARY));
    layout->addWidget(titleLabel);

    m_authInstructionLabel = new QLabel(page);
    m_authInstructionLabel->setTextFormat(Qt::RichText);
    m_authInstructionLabel->setOpenExternalLinks(true);
    m_authInstructionLabel->setWordWrap(true);
    m_authInstructionLabel->setText(
        "To download from itch.io, you need an API key.<br><br>"
        "1. Visit <a href='https://itch.io/user/settings/api-keys'>https://itch.io/user/settings/api-keys</a><br>"
        "2. Click \"Generate new API key\"<br>"
        "3. Copy the key and paste it below:"
    );
    m_authInstructionLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 12px; }")
                                         .arg(Theme::Colors::TEXT_SECONDARY));
    layout->addWidget(m_authInstructionLabel);

    m_apiKeyEdit = new QLineEdit(page);
    m_apiKeyEdit->setPlaceholderText("Paste your API key here...");
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    connect(m_apiKeyEdit, &QLineEdit::returnPressed, this, &ItchDownloadModalContent::onApiKeySubmit);
    layout->addWidget(m_apiKeyEdit);

    layout->addStretch();

    return page;
}

QWidget* ItchDownloadModalContent::createDownloadPage() {
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

void ItchDownloadModalContent::showInputPage() {
    m_stack->setCurrentIndex(InputPage);
    updateFooterButtons();
}

void ItchDownloadModalContent::showAuthPage() {
    m_stack->setCurrentIndex(AuthPage);
    updateFooterButtons();
}

void ItchDownloadModalContent::showDownloadPage() {
    m_progressBar->setValue(0);
    m_statusLabel->setText("Preparing download...");
    m_stack->setCurrentIndex(DownloadPage);
    updateFooterButtons();
}

void ItchDownloadModalContent::updateFooterButtons() {
    int currentPage = m_stack->currentIndex();

    m_downloadButton->setVisible(currentPage == InputPage);
    m_submitApiKeyButton->setVisible(currentPage == AuthPage);
    m_cancelButton->setVisible(true);
}

void ItchDownloadModalContent::onDownloadClicked() {
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        MessageModal::warning(m_modalManager, "Error", "Please enter a URL");
        return;
    }

    auto result = ItchUrlParser::parseUrl(url);
    if (!result.isValid) {
        MessageModal::warning(m_modalManager, "Invalid URL", result.error);
        return;
    }

    m_pendingCreator = result.creator;
    m_pendingGameName = result.gameName;

    if (!m_client->hasApiKey()) {
        showAuthPage();
        return;
    }

    startDownloadProcess();
}

void ItchDownloadModalContent::onApiKeySubmit() {
    QString apiKey = m_apiKeyEdit->text().trimmed();
    if (apiKey.isEmpty()) {
        MessageModal::warning(m_modalManager, "Error", "Please enter an API key");
        return;
    }

    m_client->setApiKey(apiKey);
    m_submitApiKeyButton->setEnabled(false);
    m_authInstructionLabel->setText("API key saved. Starting download...");

    QTimer::singleShot(500, this, &ItchDownloadModalContent::startDownloadProcess);
}

void ItchDownloadModalContent::startDownloadProcess() {
    showDownloadPage();
    m_statusLabel->setText("Fetching game information...");

    m_client->getGameId(m_pendingCreator, m_pendingGameName);
}

void ItchDownloadModalContent::startNextDownload() {
    if (m_currentDownloadIndex >= m_pendingUploads.size()) {
        return;
    }

    const ItchUploadInfo &upload = m_pendingUploads[m_currentDownloadIndex];
    m_currentUploadId = upload.id;

    if (m_pendingUploads.size() > 1) {
        m_statusLabel->setText(QString("Getting download link for file %1 of %2...")
            .arg(m_currentDownloadIndex + 1)
            .arg(m_pendingUploads.size()));
    } else {
        m_statusLabel->setText("Getting download link...");
    }

    m_client->getDownloadLink(m_currentUploadId);
}

void ItchDownloadModalContent::onGameIdReceived(const QString &gameId, const QString &title, const QString &author) {
    m_currentGameId = gameId;
    m_gameTitle = title;
    m_author = author;

    m_statusLabel->setText("Fetching file list...");
    m_client->getGameUploads(gameId);
}

void ItchDownloadModalContent::onUploadsReceived(const QList<ItchUploadInfo> &uploads) {
    if (uploads.isEmpty()) {
        MessageModal::warning(m_modalManager, "No Files", "No files available for this game.");
        showInputPage();
        return;
    }

    QList<ItchUploadInfo> sorted = uploads;
    std::sort(sorted.begin(), sorted.end(), [](const ItchUploadInfo &a, const ItchUploadInfo &b) {
        return a.createdAt > b.createdAt;
    });

    if (sorted.size() == 1) {
        m_pendingUploads = {sorted.first()};
        m_currentDownloadIndex = 0;
        m_downloadedPaths.clear();
        showDownloadPage();
        startNextDownload();
        return;
    }

    QList<FileItem> items;
    for (const ItchUploadInfo &upload : sorted) {
        QString displayText = upload.filename;

        if (!upload.displayName.isEmpty()) {
            displayText = upload.displayName + QString(" (%1)").arg(upload.filename);
        }

        if (upload.sizeBytes > 0) {
            displayText += QString(" - %1").arg(formatFileSize(upload.sizeBytes));
        }

        if (upload.createdAt.isValid()) {
            displayText += QString(" [%1]").arg(upload.createdAt.toString("yyyy-MM-dd"));
        }

        items.append({upload.id, displayText});
    }

    auto *fileModal = new FileSelectionModalContent(
        items,
        QString("Select Files - %1").arg(m_gameTitle),
        "Multiple files found. Select one or more (Ctrl+Click or Shift+Click):",
        true
    );

    connect(fileModal, &FileSelectionModalContent::accepted, this, [this, fileModal, sorted]() {
        QStringList selectedIds = fileModal->getSelectedIds();
        if (selectedIds.isEmpty()) {
            MessageModal::warning(m_modalManager, "Error", "No files selected.");
            showInputPage();
            return;
        }

        m_pendingUploads.clear();
        for (const QString &id : selectedIds) {
            for (const ItchUploadInfo &upload : sorted) {
                if (upload.id == id) {
                    m_pendingUploads.append(upload);
                    break;
                }
            }
        }

        m_currentDownloadIndex = 0;
        m_downloadedPaths.clear();

        showDownloadPage();
        startNextDownload();
    });

    connect(fileModal, &FileSelectionModalContent::rejected, this, [this]() {
        showInputPage();
    });

    m_modalManager->showModal(fileModal);
}

void ItchDownloadModalContent::onDownloadLinkReceived(const QString &url) {
    if (url.isEmpty()) {
        MessageModal::warning(m_modalManager, "Error", "Failed to get download link.");
        showInputPage();
        return;
    }

    const ItchUploadInfo &upload = m_pendingUploads[m_currentDownloadIndex];
    QString fileName = upload.filename;

    if (fileName.isEmpty()) {
        QString cleanTitle = m_gameTitle;
        cleanTitle.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
        fileName = cleanTitle + ".pak";
    }

    QString tempPath = generateTempPath(fileName);

    m_statusLabel->setText("Downloading file...");
    m_client->downloadFile(QUrl(url), tempPath);
}

void ItchDownloadModalContent::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        int percentage = static_cast<int>((received * 100) / total);
        m_progressBar->setValue(percentage);

        QString status;
        if (m_pendingUploads.size() > 1) {
            status = QString("Downloading file %1 of %2... %3 MB / %4 MB")
                .arg(m_currentDownloadIndex + 1)
                .arg(m_pendingUploads.size())
                .arg(received / (1024.0 * 1024.0), 0, 'f', 2)
                .arg(total / (1024.0 * 1024.0), 0, 'f', 2);
        } else {
            status = QString("Downloading... %1 MB / %2 MB")
                .arg(received / (1024.0 * 1024.0), 0, 'f', 2)
                .arg(total / (1024.0 * 1024.0), 0, 'f', 2);
        }
        m_statusLabel->setText(status);
    } else {
        m_progressBar->setRange(0, 0);
        if (m_pendingUploads.size() > 1) {
            m_statusLabel->setText(QString("Downloading file %1 of %2...")
                .arg(m_currentDownloadIndex + 1)
                .arg(m_pendingUploads.size()));
        } else {
            m_statusLabel->setText("Downloading...");
        }
    }
}

void ItchDownloadModalContent::onDownloadFinished(const QString &savePath) {
    m_downloadedPath = savePath;
    m_downloadedPaths.append(savePath);
    emit downloadComplete(savePath);

    m_currentDownloadIndex++;

    if (m_currentDownloadIndex < m_pendingUploads.size()) {
        m_progressBar->setValue(0);
        startNextDownload();
    } else {
        if (m_pendingUploads.size() > 1) {
            m_statusLabel->setText(QString("All downloads complete! (%1 files)")
                .arg(m_pendingUploads.size()));
        } else {
            m_statusLabel->setText("Download complete!");
        }
        m_progressBar->setValue(100);
        accept();
    }
}

void ItchDownloadModalContent::onError(const QString &error) {
    MessageModal::warning(m_modalManager, "Error", error);
    showInputPage();
}

QString ItchDownloadModalContent::generateTempPath(const QString &fileName) const {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString uniqueName = QString("itch_game_%1_%2_%3")
        .arg(m_currentGameId)
        .arg(m_currentUploadId)
        .arg(fileName);

    return QDir(tempDir).filePath(uniqueName);
}

QString ItchDownloadModalContent::formatFileSize(qint64 bytes) const {
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
