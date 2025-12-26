#include "ModUpdateDialog.h"
#include "../utils/ModManager.h"
#include "../utils/NexusModsClient.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

ModUpdateDialog::ModUpdateDialog(const ModInfo &mod,
                                 const ModUpdateInfo &updateInfo,
                                 ModManager *modManager,
                                 NexusModsClient *nexusClient,
                                 QWidget *parent)
    : QDialog(parent)
    , m_mod(mod)
    , m_updateInfo(updateInfo)
    , m_modManager(modManager)
    , m_nexusClient(nexusClient)
{
    setupUi();

    connect(m_nexusClient, &NexusModsClient::downloadLinkReceived,
            this, &ModUpdateDialog::onDownloadLinkReceived);
    connect(m_nexusClient, &NexusModsClient::downloadProgress,
            this, &ModUpdateDialog::onDownloadProgress);
    connect(m_nexusClient, &NexusModsClient::downloadFinished,
            this, &ModUpdateDialog::onDownloadFinished);
    connect(m_nexusClient, &NexusModsClient::errorOccurred,
            this, &ModUpdateDialog::onError);

    QTimer::singleShot(0, this, &ModUpdateDialog::startDownload);
}

void ModUpdateDialog::setupUi() {
    setWindowTitle(QStringLiteral("Updating Mod"));
    setMinimumSize(400, 150);

    auto *layout = new QVBoxLayout(this);

    m_titleLabel = new QLabel(QStringLiteral("Updating: %1").arg(m_mod.name), this);
    m_titleLabel->setWordWrap(true);
    layout->addWidget(m_titleLabel);

    m_versionLabel = new QLabel(QStringLiteral("Version %1 → %2")
                                   .arg(m_updateInfo.currentVersion,
                                        m_updateInfo.availableVersion),
                               this);
    layout->addWidget(m_versionLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    layout->addWidget(m_progressBar);

    m_statusLabel = new QLabel(QStringLiteral("Preparing download..."), this);
    layout->addWidget(m_statusLabel);

    layout->addStretch();

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &ModUpdateDialog::onCancelClicked);
    layout->addWidget(m_buttonBox);
}

void ModUpdateDialog::startDownload() {
    if (!m_nexusClient || !m_modManager) {
        QMessageBox::critical(this, QStringLiteral("Error"),
                            QStringLiteral("Missing required services"));
        reject();
        return;
    }

    m_statusLabel->setText(QStringLiteral("Getting download link..."));
    m_nexusClient->getDownloadLink(m_mod.nexusModId, m_updateInfo.availableFileId);
}

void ModUpdateDialog::onDownloadLinkReceived(const QString &url) {
    QString fileName = QStringLiteral("update_%1_%2.tmp")
                          .arg(m_mod.nexusModId, m_updateInfo.availableFileId);
    QString savePath = generateTempPath(fileName);

    m_statusLabel->setText(QStringLiteral("Downloading..."));
    m_nexusClient->downloadFile(QUrl(url), savePath);
}

void ModUpdateDialog::onDownloadProgress(qint64 received, qint64 total) {
    if (total > 0) {
        int percentage = static_cast<int>((received * 100) / total);
        m_progressBar->setValue(percentage);

        qint64 receivedMB = received / (1024 * 1024);
        qint64 totalMB = total / (1024 * 1024);
        m_statusLabel->setText(QStringLiteral("Downloading: %1 MB / %2 MB")
                                  .arg(receivedMB).arg(totalMB));
    }
}

void ModUpdateDialog::onDownloadFinished(const QString &savePath) {
    m_downloadedPath = savePath;
    m_statusLabel->setText(QStringLiteral("Installing update..."));
    m_progressBar->setValue(100);

    if (!m_modManager->replaceMod(m_mod.id, savePath,
                                  m_updateInfo.availableVersion,
                                  m_updateInfo.availableFileId)) {
        QFile::remove(savePath);
        QMessageBox::critical(this, QStringLiteral("Error"),
                            QStringLiteral("Failed to install mod update"));
        reject();
        return;
    }

    QFile::remove(savePath);

    m_statusLabel->setText(QStringLiteral("Update complete!"));
    QMessageBox::information(this, QStringLiteral("Success"),
                           QStringLiteral("Mod updated successfully to version %1")
                               .arg(m_updateInfo.availableVersion));
    accept();
}

void ModUpdateDialog::onError(const QString &error) {
    m_statusLabel->setText(QStringLiteral("Error: %1").arg(error));

    if (error == QStringLiteral("PREMIUM_REQUIRED")) {
        QString modUrl = QStringLiteral("https://www.nexusmods.com/foxhole/mods/%1?tab=files&file_id=%2")
                            .arg(m_mod.nexusModId, m_updateInfo.availableFileId);

        auto reply = QMessageBox::information(
            this,
            QStringLiteral("Premium Required"),
            QStringLiteral("Direct updates via API require a Nexus Mods Premium account.\n\n"
                         "Your browser will open to the download page where you can download the file manually.\n\n"
                         "The update will be installed with existing metadata preserved."),
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
                    m_statusLabel->setText(QStringLiteral("Installing update..."));
                    m_progressBar->setValue(100);

                    if (!m_modManager->replaceMod(m_mod.id, filePath,
                                                  m_updateInfo.availableVersion,
                                                  m_updateInfo.availableFileId)) {
                        QFile::remove(filePath);
                        QMessageBox::critical(this, QStringLiteral("Error"),
                                            QStringLiteral("Failed to install mod update"));
                        reject();
                        return;
                    }

                    m_statusLabel->setText(QStringLiteral("Update complete!"));
                    QMessageBox::information(this, QStringLiteral("Success"),
                                           QStringLiteral("Mod updated successfully to version %1")
                                               .arg(m_updateInfo.availableVersion));
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
        QMessageBox::critical(this, QStringLiteral("Error"), error);
        reject();
    }
}

void ModUpdateDialog::onCancelClicked() {
    m_nexusClient->cancelDownload();
    reject();
}

QString ModUpdateDialog::generateTempPath(const QString &fileName) const {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(tempDir).filePath(fileName);
}
