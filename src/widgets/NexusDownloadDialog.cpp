#include "NexusDownloadDialog.h"
#include "NexusFileSelectionDialog.h"
#include "../utils/NexusModsClient.h"
#include "../utils/NexusModsAuth.h"
#include "../utils/NexusUrlParser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QStackedWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

NexusDownloadDialog::NexusDownloadDialog(NexusModsClient *client,
                                         NexusModsAuth *auth,
                                         QWidget *parent)
    : QDialog(parent)
    , m_client(client)
    , m_auth(auth)
    , m_stack(new QStackedWidget(this))
    , m_urlEdit(nullptr)
    , m_downloadButton(nullptr)
    , m_cancelButton(nullptr)
    , m_authenticateButton(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_authStatusLabel(nullptr)
{
    setupUi();

    connect(m_client, &NexusModsClient::modInfoReceived, this, &NexusDownloadDialog::onModInfoReceived);
    connect(m_client, &NexusModsClient::modFilesReceived, this, &NexusDownloadDialog::onModFilesReceived);
    connect(m_client, &NexusModsClient::downloadLinkReceived, this, &NexusDownloadDialog::onDownloadLinkReceived);
    connect(m_client, &NexusModsClient::downloadProgress, this, &NexusDownloadDialog::onDownloadProgress);
    connect(m_client, &NexusModsClient::downloadFinished, this, &NexusDownloadDialog::onDownloadFinished);
    connect(m_client, &NexusModsClient::errorOccurred, this, &NexusDownloadDialog::onError);

    connect(m_auth, &NexusModsAuth::authenticationStarted, this, &NexusDownloadDialog::onAuthStarted);
    connect(m_auth, &NexusModsAuth::authenticationComplete, this, &NexusDownloadDialog::onAuthComplete);
    connect(m_auth, &NexusModsAuth::authenticationFailed, this, &NexusDownloadDialog::onAuthFailed);
}

void NexusDownloadDialog::setupUi() {
    setWindowTitle(QStringLiteral("Download from Nexus Mods"));
    setMinimumSize(500, 300);

    auto *mainLayout = new QVBoxLayout(this);

    m_stack->addWidget(createInputPage());
    m_stack->addWidget(createAuthPage());
    m_stack->addWidget(createDownloadPage());

    mainLayout->addWidget(m_stack);

    showInputPage();
}

QWidget* NexusDownloadDialog::createInputPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *instructionLabel = new QLabel(
        QStringLiteral("Enter Nexus Mods URL for a Foxhole mod:"),
        page
    );
    layout->addWidget(instructionLabel);

    m_urlEdit = new QLineEdit(page);
    m_urlEdit->setPlaceholderText(QStringLiteral("https://www.nexusmods.com/foxhole/mods/..."));
    layout->addWidget(m_urlEdit);

    layout->addStretch();

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_downloadButton = new QPushButton(QStringLiteral("Download"), page);
    m_cancelButton = new QPushButton(QStringLiteral("Cancel"), page);

    connect(m_downloadButton, &QPushButton::clicked, this, &NexusDownloadDialog::onDownloadClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &NexusDownloadDialog::onCancelClicked);

    buttonLayout->addWidget(m_downloadButton);
    buttonLayout->addWidget(m_cancelButton);

    layout->addLayout(buttonLayout);

    return page;
}

QWidget* NexusDownloadDialog::createAuthPage() {
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *instructionLabel = new QLabel(
        QStringLiteral("Authentication required to access Nexus Mods API."),
        page
    );
    layout->addWidget(instructionLabel);

    m_authStatusLabel = new QLabel(QStringLiteral("Click Authenticate to begin..."), page);
    layout->addWidget(m_authStatusLabel);

    layout->addStretch();

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_authenticateButton = new QPushButton(QStringLiteral("Authenticate"), page);
    auto *authCancelButton = new QPushButton(QStringLiteral("Cancel"), page);

    connect(m_authenticateButton, &QPushButton::clicked, this, &NexusDownloadDialog::onAuthenticateClicked);
    connect(authCancelButton, &QPushButton::clicked, this, &NexusDownloadDialog::onCancelClicked);

    buttonLayout->addWidget(m_authenticateButton);
    buttonLayout->addWidget(authCancelButton);

    layout->addLayout(buttonLayout);

    return page;
}

QWidget* NexusDownloadDialog::createDownloadPage() {
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
    connect(cancelDownloadButton, &QPushButton::clicked, this, &NexusDownloadDialog::onCancelClicked);

    buttonLayout->addWidget(cancelDownloadButton);
    layout->addLayout(buttonLayout);

    return page;
}

void NexusDownloadDialog::showInputPage() {
    m_stack->setCurrentIndex(InputPage);
}

void NexusDownloadDialog::showAuthPage() {
    m_authStatusLabel->setText(QStringLiteral("Click Authenticate to begin..."));
    m_authenticateButton->setEnabled(true);
    m_stack->setCurrentIndex(AuthPage);
}

void NexusDownloadDialog::showDownloadPage() {
    m_progressBar->setValue(0);
    m_statusLabel->setText(QStringLiteral("Preparing download..."));
    m_stack->setCurrentIndex(DownloadPage);
}

void NexusDownloadDialog::onDownloadClicked() {
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Error"), QStringLiteral("Please enter a URL"));
        return;
    }

    auto result = NexusUrlParser::parseUrl(url);
    if (!result.isValid) {
        QMessageBox::warning(this, QStringLiteral("Invalid URL"), result.error);
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

void NexusDownloadDialog::onCancelClicked() {
    m_client->cancelDownload();
    m_auth->cancelAuthentication();
    reject();
}

void NexusDownloadDialog::onAuthenticateClicked() {
    m_authenticateButton->setEnabled(false);
    m_authStatusLabel->setText(QStringLiteral("Connecting to authentication server..."));
    m_auth->startAuthentication();
}

void NexusDownloadDialog::onAuthStarted(const QString &browserUrl) {
    m_authStatusLabel->setText(QStringLiteral("Opening browser for authentication...\n\nIf browser doesn't open, visit:\n") + browserUrl);
    QDesktopServices::openUrl(QUrl(browserUrl));
}

void NexusDownloadDialog::onAuthComplete(const QString &apiKey) {
    m_client->setApiKey(apiKey);
    m_authStatusLabel->setText(QStringLiteral("Authentication successful!"));
    startDownloadProcess();
}

void NexusDownloadDialog::onAuthFailed(const QString &error) {
    m_authStatusLabel->setText(QStringLiteral("Authentication failed: ") + error);
    m_authenticateButton->setEnabled(true);
}

void NexusDownloadDialog::startDownloadProcess() {
    showDownloadPage();

    m_statusLabel->setText(QStringLiteral("Fetching mod information..."));
    m_client->getModInfo(m_currentModId);
}

void NexusDownloadDialog::onModInfoReceived(const QString &author, const QString &description, const QString &version) {
    m_author = author;
    m_description = description;
    m_version = version;

    if (m_currentFileId.isEmpty()) {
        m_statusLabel->setText(QStringLiteral("Fetching file list..."));
        m_client->getModFiles(m_currentModId);
    } else {
        m_statusLabel->setText(QStringLiteral("Getting download link..."));
        m_client->getDownloadLink(m_currentModId, m_currentFileId);
    }
}

void NexusDownloadDialog::onModFilesReceived(const QList<NexusFileInfo> &files) {
    if (files.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Error"), QStringLiteral("No files found for this mod"));
        showInputPage();
        return;
    }

    if (files.size() == 1) {
        m_currentFileId = files.first().id;
    } else {
        NexusFileSelectionDialog selectionDialog(files, QStringLiteral("Mod Files"), this);
        if (selectionDialog.exec() != QDialog::Accepted) {
            showInputPage();
            return;
        }

        m_currentFileId = selectionDialog.getSelectedFileId();
        if (m_currentFileId.isEmpty()) {
            showInputPage();
            return;
        }
    }

    m_statusLabel->setText(QStringLiteral("Getting download link..."));
    m_client->getDownloadLink(m_currentModId, m_currentFileId);
}

void NexusDownloadDialog::onDownloadLinkReceived(const QString &url) {
    QString fileName = QStringLiteral("nexus_mod_%1_%2.tmp").arg(m_currentModId, m_currentFileId);
    QString savePath = generateTempPath(fileName);

    m_statusLabel->setText(QStringLiteral("Downloading file..."));
    m_client->downloadFile(QUrl(url), savePath);
}

void NexusDownloadDialog::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        int percentage = static_cast<int>((received * 100) / total);
        m_progressBar->setValue(percentage);

        qint64 receivedMB = received / (1024 * 1024);
        qint64 totalMB = total / (1024 * 1024);
        m_statusLabel->setText(QStringLiteral("Downloading: %1 MB / %2 MB").arg(receivedMB).arg(totalMB));
    }
}

void NexusDownloadDialog::onDownloadFinished(const QString &savePath) {
    m_downloadedPath = savePath;
    m_statusLabel->setText(QStringLiteral("Download complete!"));
    m_progressBar->setValue(100);
    accept();
}

void NexusDownloadDialog::onError(const QString &error) {
    if (error == QStringLiteral("PREMIUM_REQUIRED")) {
        QString modUrl = QStringLiteral("https://www.nexusmods.com/foxhole/mods/%1?tab=files&file_id=%2")
                            .arg(m_currentModId, m_currentFileId);

        auto reply = QMessageBox::information(
            this,
            QStringLiteral("Premium Required"),
            QStringLiteral("Direct downloads via API require a Nexus Mods Premium account.\n\n"
                         "Your browser will open to the download page where you can download the file manually.\n\n"
                         "The mod will be installed with Nexus metadata for future updates."),
            QMessageBox::Ok | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Ok) {
            QDesktopServices::openUrl(QUrl(modUrl));

            auto selectReply = QMessageBox::information(
                this,
                QStringLiteral("Select Downloaded File"),
                QStringLiteral("Please download the file from your browser.\n\n"
                             "Once the download is complete, click OK to locate the file."),
                QMessageBox::Ok | QMessageBox::Cancel
            );

            if (selectReply == QMessageBox::Ok) {
                QString filePath = QFileDialog::getOpenFileName(
                    this,
                    QStringLiteral("Select Downloaded File"),
                    QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
                    QStringLiteral("Mod Files (*.pak *.zip);;All Files (*.*)")
                );

                if (!filePath.isEmpty()) {
                    m_downloadedPath = filePath;
                    accept();
                } else {
                    reject();
                }
            } else {
                reject();
            }
        } else {
            reject();
        }
    } else {
        QMessageBox::warning(this, QStringLiteral("Error"), error);
        showInputPage();
    }
}

QString NexusDownloadDialog::generateTempPath(const QString &fileName) const {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(tempDir).filePath(fileName);
}
