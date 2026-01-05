#pragma once

#include <QDialog>
#include <QString>
#include "../utils/ModInfo.h"
#include "../utils/ItchUpdateInfo.h"

class ModManager;
class ItchClient;
class QLabel;
class QProgressBar;
class QPushButton;
class QDialogButtonBox;

class ItchModUpdateDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItchModUpdateDialog(const ModInfo &mod,
                                const ItchUpdateInfo &updateInfo,
                                ModManager *modManager,
                                ItchClient *itchClient,
                                QWidget *parent = nullptr);
    ~ItchModUpdateDialog() override = default;

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
    ItchUpdateInfo m_updateInfo;
    ModManager *m_modManager;
    ItchClient *m_itchClient;

    QLabel *m_titleLabel;
    QLabel *m_versionLabel;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QDialogButtonBox *m_buttonBox;

    QString m_downloadedPath;
};
