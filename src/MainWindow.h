#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFutureWatcher>
#include <QPointer>
#include "utils/UpdaterService.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class InstallPathWidget;
class ModListWidget;
class RightPanelWidget;
class ProfileManagerWidget;
class ModManager;
class ProfileManager;
class QProgressDialog;
class SettingsWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onMinimizeClicked();
    void onCloseClicked();
    void onInstallPathChanged(const QString &path);
    void onModsLoadComplete();
    void onUnregisteredModsDetectionComplete();
    void onUpdateClicked();
    void onUpdateCheckError(const QString &message);
    void onUpdateAvailable(const UpdaterService::ReleaseInfo &release);
    void onUpdateUpToDate(const UpdaterService::ReleaseInfo &latest);
    void onUpdateDownloadProgress(qint64 received, qint64 total);
    void onUpdateDownloadFinished(const QString &savePath);
    void onSettingsClicked();

private:
    void setupTitleBar();
    void setupInstallPath();
    void setupProfileManager();
    void setupModList();
    void setupRightPanel();
    void setupSettingsOverlay();
    void loadSettings();
    void saveSettings();
    void startUpdateCheck();
    void beginUpdateDownload();
    void showUpdateDialog();
    void closeUpdateDialog();
    QString selectUpdateAssetName() const;
    bool stageUpdate(const QString &archivePath, const QString &version, const QString &updatesDir, QString *error);
    void launchUpdater(const QString &stagingDir, const QString &updatesDir);
    void showSettingsOverlay();
    void hideSettingsOverlay();
    void onSettingsApplied(bool autoCheck);

    Ui::MainWindow *ui;
    InstallPathWidget *m_installPathWidget;
    ProfileManagerWidget *m_profileManagerWidget;
    ModListWidget *m_modListWidget;
    RightPanelWidget *m_rightPanelWidget;
    ModManager *m_modManager;
    ProfileManager *m_profileManager;
    QFutureWatcher<bool> *m_modLoadWatcher;
    QFutureWatcher<void> *m_unregisteredModsWatcher;
    bool m_firstShow = true;
    UpdaterService *m_updater = nullptr;
    UpdaterService::ReleaseInfo m_updateRelease;
    bool m_updateAvailable = false;
    bool m_updateInstallStarted = false;
    QPointer<QProgressDialog> m_updateDialog;
    QString m_pendingUpdatePath;
    qint64 m_pendingUpdateSize = -1;
    QWidget *m_settingsPage = nullptr;
    SettingsWidget *m_settingsWidget = nullptr;

    QPixmap m_backgroundTexture;
    QPixmap m_cachedScaledTexture;
    QSize m_lastSize;
};

#endif // MAINWINDOW_H
