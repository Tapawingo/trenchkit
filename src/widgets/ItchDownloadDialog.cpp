#include "ItchDownloadDialog.h"
#include "ItchFileSelectionDialog.h"
#include "../utils/ItchClient.h"
#include "../utils/ItchAuth.h"
#include "../utils/ItchUrlParser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QStackedWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QRegularExpression>

ItchDownloadDialog::ItchDownloadDialog(ItchClient *client,
                                       ItchAuth *auth,
                                       QWidget *parent)
    : QDialog(parent)
    , m_client(client)
    , m_auth(auth)
    , m_stack(new QStackedWidget(this))
    , m_urlEdit(nullptr)
    , m_apiKeyEdit(nullptr)
    , m_downloadButton(nullptr)
    , m_cancelButton(nullptr)
    , m_submitApiKeyButton(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_authInstructionLabel(nullptr)
{
    setupUi();

    connect(m_client, &ItchClient::gameIdReceived, this, &ItchDownloadDialog::onGameIdReceived);
    connect(m_client, &ItchClient::uploadsReceived, this, &ItchDownloadDialog::onUploadsReceived);
    connect(m_client, &ItchClient::downloadLinkReceived, this, &ItchDownloadDialog::onDownloadLinkReceived);
    connect(m_client, &ItchClient::downloadProgress, this, &ItchDownloadDialog::onDownloadProgress);
    connect(m_client, &ItchClient::downloadFinished, this, &ItchDownloadDialog::onDownloadFinished);
    connect(m_client, &ItchClient::errorOccurred, this, &ItchDownloadDialog::onError);
}

void ItchDownloadDialog::setupUi() {
    setWindowTitle(QStringLiteral("Download from itch.io"));
    setMinimumSize(500, 300);

    auto *mainLayout = new QVBoxLayout(this);

    m_stack->addWidget(createInputPage());
    m_stack->addWidget(createAuthPage());
    m_stack->addWidget(createDownloadPage());

    mainLayout->addWidget(m_stack);

    showInputPage();
}

QWidget* ItchDownloadDialog::createInputPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *instructionLabel = new QLabel(
        QStringLiteral("Enter itch.io URL:"),
        page
    );
    layout->addWidget(instructionLabel);

    m_urlEdit = new QLineEdit(page);
    m_urlEdit->setPlaceholderText(QStringLiteral("https://creator.itch.io/game-name"));
    layout->addWidget(m_urlEdit);

    layout->addStretch();

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_downloadButton = new QPushButton(QStringLiteral("Download"), page);
    m_cancelButton = new QPushButton(QStringLiteral("Cancel"), page);

    connect(m_downloadButton, &QPushButton::clicked, this, &ItchDownloadDialog::onDownloadClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ItchDownloadDialog::onCancelClicked);

    buttonLayout->addWidget(m_downloadButton);
    buttonLayout->addWidget(m_cancelButton);

    layout->addLayout(buttonLayout);

    return page;
}

QWidget* ItchDownloadDialog::createAuthPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *titleLabel = new QLabel(
        QStringLiteral("<b>API Key Required</b>"),
        page
    );
    layout->addWidget(titleLabel);

    m_authInstructionLabel = new QLabel(page);
    m_authInstructionLabel->setTextFormat(Qt::RichText);
    m_authInstructionLabel->setOpenExternalLinks(true);
    m_authInstructionLabel->setWordWrap(true);
    m_authInstructionLabel->setText(
        QStringLiteral("To download from itch.io, you need an API key.<br><br>"
                      "1. Visit <a href='https://itch.io/user/settings/api-keys'>https://itch.io/user/settings/api-keys</a><br>"
                      "2. Click \"Generate new API key\"<br>"
                      "3. Copy the key and paste it below:")
    );
    layout->addWidget(m_authInstructionLabel);

    m_apiKeyEdit = new QLineEdit(page);
    m_apiKeyEdit->setPlaceholderText(QStringLiteral("Paste your API key here..."));
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_apiKeyEdit);

    layout->addStretch();

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_submitApiKeyButton = new QPushButton(QStringLiteral("Submit"), page);
    auto *authCancelButton = new QPushButton(QStringLiteral("Cancel"), page);

    connect(m_submitApiKeyButton, &QPushButton::clicked, this, &ItchDownloadDialog::onApiKeySubmit);
    connect(authCancelButton, &QPushButton::clicked, this, &ItchDownloadDialog::onCancelClicked);

    buttonLayout->addWidget(m_submitApiKeyButton);
    buttonLayout->addWidget(authCancelButton);

    layout->addLayout(buttonLayout);

    return page;
}

QWidget* ItchDownloadDialog::createDownloadPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    m_statusLabel = new QLabel(QStringLiteral("Preparing download..."), page);
    layout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(page);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    layout->addWidget(m_progressBar);

    layout->addStretch();

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    auto *cancelDownloadButton = new QPushButton(QStringLiteral("Cancel"), page);
    connect(cancelDownloadButton, &QPushButton::clicked, this, &ItchDownloadDialog::onCancelClicked);

    buttonLayout->addWidget(cancelDownloadButton);
    layout->addLayout(buttonLayout);

    return page;
}

void ItchDownloadDialog::showInputPage() {
    m_stack->setCurrentIndex(InputPage);
}

void ItchDownloadDialog::showAuthPage() {
    m_stack->setCurrentIndex(AuthPage);
}

void ItchDownloadDialog::showDownloadPage() {
    m_progressBar->setValue(0);
    m_statusLabel->setText(QStringLiteral("Preparing download..."));
    m_stack->setCurrentIndex(DownloadPage);
}

void ItchDownloadDialog::onDownloadClicked() {
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Error"), QStringLiteral("Please enter a URL"));
        return;
    }

    auto result = ItchUrlParser::parseUrl(url);
    if (!result.isValid) {
        QMessageBox::warning(this, QStringLiteral("Invalid URL"), result.error);
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

void ItchDownloadDialog::onCancelClicked() {
    m_client->cancelDownload();
    reject();
}

void ItchDownloadDialog::onApiKeySubmit() {
    QString apiKey = m_apiKeyEdit->text().trimmed();
    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Error"), QStringLiteral("Please enter an API key"));
        return;
    }

    m_client->setApiKey(apiKey);
    m_submitApiKeyButton->setEnabled(false);
    m_authInstructionLabel->setText(QStringLiteral("API key saved. Starting download..."));

    // Start download process after brief moment
    QTimer::singleShot(500, this, &ItchDownloadDialog::startDownloadProcess);
}

void ItchDownloadDialog::startDownloadProcess() {
    showDownloadPage();
    m_statusLabel->setText(QStringLiteral("Fetching game information..."));

    m_client->getGameId(m_pendingCreator, m_pendingGameName);
}

void ItchDownloadDialog::startNextDownload() {
    if (m_currentDownloadIndex >= m_pendingUploads.size()) {
        return;
    }

    const ItchUploadInfo &upload = m_pendingUploads[m_currentDownloadIndex];
    m_currentUploadId = upload.id;

    if (m_pendingUploads.size() > 1) {
        m_statusLabel->setText(QStringLiteral("Getting download link for file %1 of %2...")
            .arg(m_currentDownloadIndex + 1)
            .arg(m_pendingUploads.size()));
    } else {
        m_statusLabel->setText(QStringLiteral("Getting download link..."));
    }

    m_client->getDownloadLink(m_currentUploadId);
}

void ItchDownloadDialog::onGameIdReceived(const QString &gameId, const QString &title, const QString &author) {
    m_currentGameId = gameId;
    m_gameTitle = title;
    m_author = author;

    m_statusLabel->setText(QStringLiteral("Fetching file list..."));
    m_client->getGameUploads(gameId);
}

void ItchDownloadDialog::onUploadsReceived(const QList<ItchUploadInfo> &uploads) {
    if (uploads.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("No Files"),
                           QStringLiteral("No files available for this game."));
        showInputPage();
        return;
    }

    if (uploads.size() == 1) {
        m_pendingUploads = {uploads.first()};
        m_currentDownloadIndex = 0;
        m_downloadedPaths.clear();
        showDownloadPage();
        startNextDownload();
        return;
    }

    ItchFileSelectionDialog fileDialog(uploads, m_gameTitle, this);
    if (fileDialog.exec() != QDialog::Accepted) {
        showInputPage();
        return;
    }

    QStringList selectedIds = fileDialog.getSelectedUploadIds();
    if (selectedIds.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Error"),
                           QStringLiteral("No files selected."));
        showInputPage();
        return;
    }

    m_pendingUploads.clear();
    for (const QString &id : selectedIds) {
        for (const ItchUploadInfo &upload : uploads) {
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
}

void ItchDownloadDialog::onDownloadLinkReceived(const QString &url) {
    if (url.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Error"),
                           QStringLiteral("Failed to get download link."));
        showInputPage();
        return;
    }

    const ItchUploadInfo &upload = m_pendingUploads[m_currentDownloadIndex];
    QString fileName = upload.filename;

    if (fileName.isEmpty()) {
        QString cleanTitle = m_gameTitle;
        cleanTitle.replace(QRegularExpression(QStringLiteral("[^a-zA-Z0-9_-]")), QStringLiteral("_"));
        fileName = cleanTitle + QStringLiteral(".pak");
    }

    QString tempPath = generateTempPath(fileName);

    m_statusLabel->setText(QStringLiteral("Downloading file..."));
    m_client->downloadFile(QUrl(url), tempPath);
}

void ItchDownloadDialog::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        int percentage = static_cast<int>((received * 100) / total);
        m_progressBar->setValue(percentage);

        QString status;
        if (m_pendingUploads.size() > 1) {
            status = QStringLiteral("Downloading file %1 of %2... %3 MB / %4 MB")
                .arg(m_currentDownloadIndex + 1)
                .arg(m_pendingUploads.size())
                .arg(received / (1024.0 * 1024.0), 0, 'f', 2)
                .arg(total / (1024.0 * 1024.0), 0, 'f', 2);
        } else {
            status = QStringLiteral("Downloading... %1 MB / %2 MB")
                .arg(received / (1024.0 * 1024.0), 0, 'f', 2)
                .arg(total / (1024.0 * 1024.0), 0, 'f', 2);
        }
        m_statusLabel->setText(status);
    } else {
        m_progressBar->setRange(0, 0);
        if (m_pendingUploads.size() > 1) {
            m_statusLabel->setText(QStringLiteral("Downloading file %1 of %2...")
                .arg(m_currentDownloadIndex + 1)
                .arg(m_pendingUploads.size()));
        } else {
            m_statusLabel->setText(QStringLiteral("Downloading..."));
        }
    }
}

void ItchDownloadDialog::onDownloadFinished(const QString &savePath) {
    m_downloadedPath = savePath;
    m_downloadedPaths.append(savePath);
    emit downloadComplete(savePath);

    m_currentDownloadIndex++;

    if (m_currentDownloadIndex < m_pendingUploads.size()) {
        m_progressBar->setValue(0);
        startNextDownload();
    } else {
        if (m_pendingUploads.size() > 1) {
            m_statusLabel->setText(QStringLiteral("All downloads complete! (%1 files)")
                .arg(m_pendingUploads.size()));
        } else {
            m_statusLabel->setText(QStringLiteral("Download complete!"));
        }
        m_progressBar->setValue(100);
        accept();
    }
}

void ItchDownloadDialog::onError(const QString &error) {
    QMessageBox::warning(this, QStringLiteral("Error"), error);
    showInputPage();
}

QString ItchDownloadDialog::generateTempPath(const QString &fileName) const {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString uniqueName = QStringLiteral("itch_game_%1_%2_%3")
        .arg(m_currentGameId)
        .arg(m_currentUploadId)
        .arg(fileName);

    return QDir(tempDir).filePath(uniqueName);
}
