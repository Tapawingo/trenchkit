#pragma once

#include <QDialog>
#include <QString>
#include "../utils/ModInfo.h"
#include "../utils/ModUpdateInfo.h"

class ModManager;
class NexusModsClient;
class QLabel;
class QProgressBar;
class QPushButton;
class QDialogButtonBox;

class ModUpdateDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModUpdateDialog(const ModInfo &mod,
                            const ModUpdateInfo &updateInfo,
                            ModManager *modManager,
                            NexusModsClient *nexusClient,
                            QWidget *parent = nullptr);
    ~ModUpdateDialog() override = default;

private slots:
    void onDownloadLinkReceived(const QString &url);
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished(const QString &savePath);
    void onError(const QString &error);
    void onCancelClicked();

private:
    void setupUi();
    void startDownload();
    QString generateTempPath(const QString &fileName) const;

    ModInfo m_mod;
    ModUpdateInfo m_updateInfo;
    ModManager *m_modManager;
    NexusModsClient *m_nexusClient;

    QLabel *m_titleLabel;
    QLabel *m_versionLabel;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QDialogButtonBox *m_buttonBox;

    QString m_downloadedPath;
};
