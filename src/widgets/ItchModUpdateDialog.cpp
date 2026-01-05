#include "ItchModUpdateDialog.h"
#include "../utils/ModManager.h"
#include "../utils/ItchClient.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

ItchModUpdateDialog::ItchModUpdateDialog(const ModInfo &mod,
                                       const ItchUpdateInfo &updateInfo,
                                       ModManager *modManager,
                                       ItchClient *itchClient,
                                       QWidget *parent)
    : QDialog(parent)
    , m_mod(mod)
    , m_updateInfo(updateInfo)
    , m_modManager(modManager)
    , m_itchClient(itchClient)
{
    setupUi();

    connect(m_itchClient, &ItchClient::downloadLinkReceived,
            this, &ItchModUpdateDialog::onDownloadLinkReceived);
    connect(m_itchClient, &ItchClient::downloadProgress,
            this, &ItchModUpdateDialog::onDownloadProgress);
    connect(m_itchClient, &ItchClient::downloadFinished,
            this, &ItchModUpdateDialog::onDownloadFinished);
    connect(m_itchClient, &ItchClient::errorOccurred,
            this, &ItchModUpdateDialog::onError);

    startDownload();
}

void ItchModUpdateDialog::setupUi() {
    setWindowTitle(QStringLiteral("Update Mod"));
    setMinimumWidth(450);

    auto *layout = new QVBoxLayout(this);

    m_titleLabel = new QLabel(QStringLiteral("Updating: %1").arg(m_mod.name), this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 12pt;"));
    layout->addWidget(m_titleLabel);

    QString versionText = QStringLiteral("%1 → %2")
        .arg(m_updateInfo.currentVersion)
        .arg(m_updateInfo.availableVersion);
    m_versionLabel = new QLabel(versionText, this);
    layout->addWidget(m_versionLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    layout->addWidget(m_progressBar);

    m_statusLabel = new QLabel(QStringLiteral("Preparing download..."), this);
    layout->addWidget(m_statusLabel);

    layout->addStretch();

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &ItchModUpdateDialog::onCancelClicked);
    layout->addWidget(m_buttonBox);
}

void ItchModUpdateDialog::startDownload() {
    m_statusLabel->setText(QStringLiteral("Getting download link..."));
    m_itchClient->getDownloadLink(m_updateInfo.availableUploadId);
}

void ItchModUpdateDialog::onDownloadLinkReceived(const QString &url) {
    if (url.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Error"),
                           QStringLiteral("Failed to get download link."));
        reject();
        return;
    }

    QString fileName = QStringLiteral("update_%1").arg(m_mod.fileName);
    QString tempPath = generateTempPath(fileName);

    m_statusLabel->setText(QStringLiteral("Downloading..."));
    m_itchClient->downloadFile(QUrl(url), tempPath);
}

void ItchModUpdateDialog::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        int percentage = static_cast<int>((received * 100) / total);
        m_progressBar->setValue(percentage);

        QString status = QStringLiteral("Downloading... %1 MB / %2 MB")
            .arg(received / (1024.0 * 1024.0), 0, 'f', 2)
            .arg(total / (1024.0 * 1024.0), 0, 'f', 2);
        m_statusLabel->setText(status);
    } else {
        m_progressBar->setRange(0, 0);
        m_statusLabel->setText(QStringLiteral("Downloading..."));
    }
}

void ItchModUpdateDialog::onDownloadFinished(const QString &savePath) {
    m_downloadedPath = savePath;
    m_statusLabel->setText(QStringLiteral("Installing update..."));
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(100);

    if (!m_modManager->replaceMod(m_mod.id, savePath, m_updateInfo.availableVersion,
                                  m_updateInfo.availableUploadId, m_updateInfo.availableUploadDate)) {
        QMessageBox::warning(this, QStringLiteral("Error"),
                           QStringLiteral("Failed to install update."));
        reject();
        return;
    }

    QMessageBox::information(this, QStringLiteral("Success"),
                            QStringLiteral("Mod updated successfully!"));
    accept();
}

void ItchModUpdateDialog::onError(const QString &error) {
    QMessageBox::warning(this, QStringLiteral("Error"), error);
    reject();
}

void ItchModUpdateDialog::onCancelClicked() {
    m_itchClient->cancelDownload();
    reject();
}

QString ItchModUpdateDialog::generateTempPath(const QString &fileName) const {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString uniqueName = QStringLiteral("itch_update_%1_%2")
        .arg(m_mod.itchGameId)
        .arg(fileName);

    return QDir(tempDir).filePath(uniqueName);
}
