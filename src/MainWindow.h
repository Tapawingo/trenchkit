#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFutureWatcher>
#include <QPointer>
#include "utils/UpdaterService.h"
#include "utils/NexusModsClient.h"
#include "utils/NexusModsAuth.h"
#include "utils/ItchClient.h"
#include "utils/ItchAuth.h"
#include "utils/ModUpdateService.h"
#include "utils/ItchModUpdateService.h"
#include "modals/ModalManager.h"

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
class QShortcut;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    ModalManager* modalManager() { return m_modalManager; }

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
    QString findProfileImportPath() const;

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
    NexusModsClient *m_nexusClient = nullptr;
    NexusModsAuth *m_nexusAuth = nullptr;
    ItchClient *m_itchClient = nullptr;
    ItchAuth *m_itchAuth = nullptr;
    ModUpdateService *m_modUpdateService = nullptr;
    ItchModUpdateService *m_itchUpdateService = nullptr;
    UpdaterService::ReleaseInfo m_updateRelease;
    bool m_updateAvailable = false;
    bool m_updateInstallStarted = false;
    QPointer<QProgressDialog> m_updateDialog;
    QString m_pendingUpdatePath;
    qint64 m_pendingUpdateSize = -1;
    QString m_pendingProfileImportPath;
    QWidget *m_settingsPage = nullptr;
    SettingsWidget *m_settingsWidget = nullptr;
    ModalManager *m_modalManager = nullptr;
    QShortcut *m_globalSearchShortcut = nullptr;

    QPixmap m_backgroundTexture;
    QPixmap m_cachedScaledTexture;
    QSize m_lastSize;
};

#endif // MAINWINDOW_H
