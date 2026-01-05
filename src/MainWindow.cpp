#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "widgets/InstallPathWidget.h"
#include "widgets/ProfileManagerWidget.h"
#include "widgets/ModListWidget.h"
#include "widgets/RightPanelWidget.h"
#include "widgets/ActivityLogWidget.h"
#include "widgets/BackupWidget.h"
#include "widgets/LaunchWidget.h"
#include "widgets/SettingsWidget.h"
#include "utils/FoxholeDetector.h"
#include "utils/UpdateArchiveExtractor.h"
#include "utils/ModManager.h"
#include "utils/ProfileManager.h"
#include "utils/Theme.h"
#include <QMessageBox>
#include <QSettings>
#include <QShowEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QEvent>
#include <QTimer>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QProcess>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QMetaObject>

namespace {
QString defaultUpdatesDir() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return QDir(base).filePath("updates");
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_installPathWidget(new InstallPathWidget(this))
    , m_profileManagerWidget(new ProfileManagerWidget(this))
    , m_modListWidget(new ModListWidget(this))
    , m_rightPanelWidget(new RightPanelWidget(this))
    , m_modManager(new ModManager(this))
    , m_profileManager(new ProfileManager(this))
    , m_modLoadWatcher(new QFutureWatcher<bool>(this))
    , m_unregisteredModsWatcher(new QFutureWatcher<void>(this))
    , m_updater(new UpdaterService("Tapawingo", "TrenchKit", this))
    , m_nexusClient(new NexusModsClient(this))
    , m_nexusAuth(new NexusModsAuth(this))
    , m_itchClient(new ItchClient(this))
    , m_itchAuth(new ItchAuth(this))
    , m_modUpdateService(new ModUpdateService(m_modManager, m_nexusClient, this))
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/icon.png"));
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setStyleSheet(Theme::getStyleSheet());

    m_backgroundTexture = QPixmap(":/tex_main.png");

    setupTitleBar();
    setupInstallPath();
    setupProfileManager();
    setupModList();
    setupRightPanel();
    setupSettingsOverlay();

    connect(m_updater, &UpdaterService::updateAvailable,
            this, &MainWindow::onUpdateAvailable);
    connect(m_updater, &UpdaterService::upToDate,
            this, &MainWindow::onUpdateUpToDate);
    connect(m_updater, &UpdaterService::downloadProgress,
            this, &MainWindow::onUpdateDownloadProgress);
    connect(m_updater, &UpdaterService::downloadFinished,
            this, &MainWindow::onUpdateDownloadFinished);
    connect(m_updater, &UpdaterService::errorOccurred,
            this, &MainWindow::onUpdateCheckError);
    connect(m_updater, &UpdaterService::checkingStarted, this, [this]() {
        if (m_settingsWidget) {
            m_settingsWidget->setCheckStatus("Checking...");
        }
    });

    ui->hbox->setStretch(0, 1);
    ui->hbox->setStretch(1, 2);
    ui->hbox->setStretch(2, 1);

    ui->centralwidget->installEventFilter(this);
    ui->centralwidget->setAutoFillBackground(true);

    connect(m_modLoadWatcher, &QFutureWatcher<bool>::finished,
            this, &MainWindow::onModsLoadComplete);
    connect(m_unregisteredModsWatcher, &QFutureWatcher<void>::finished,
            this, &MainWindow::onUnregisteredModsDetectionComplete);

    QSize windowSize(1000, 700);
    setMinimumSize(windowSize);
    setMaximumSize(windowSize);
    resize(windowSize);

    // Add initial log entry
    ActivityLogWidget *log = m_rightPanelWidget->getActivityLog();
    log->addLogEntry("TrenchKit Started", ActivityLogWidget::LogLevel::Info);

    QTimer::singleShot(0, this, &MainWindow::startUpdateCheck);
}

MainWindow::~MainWindow() {
    saveSettings();
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);

    if (m_firstShow) {
        m_firstShow = false;
        QTimer::singleShot(0, this, &MainWindow::loadSettings);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->centralwidget) {
        if (event->type() == QEvent::Paint) {
            QPaintEvent *paintEvent = static_cast<QPaintEvent*>(event);
            Q_UNUSED(paintEvent);
            QPainter painter(ui->centralwidget);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);

            if (!m_backgroundTexture.isNull()) {
                QRect widgetRect = ui->centralwidget->rect();

                if (widgetRect.size() != m_lastSize) {
                    m_cachedScaledTexture = m_backgroundTexture.scaled(
                        widgetRect.size(),
                        Qt::KeepAspectRatioByExpanding,
                        Qt::SmoothTransformation
                    );
                    m_lastSize = widgetRect.size();
                }

                int x = (widgetRect.width() - m_cachedScaledTexture.width()) / 2;
                int y = (widgetRect.height() - m_cachedScaledTexture.height()) / 2;

                painter.drawPixmap(x, y, m_cachedScaledTexture);
            }

            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setupTitleBar() {
    ui->titleBar->setTitle("TrenchKit - Foxhole Mod Manager");
    ui->titleBar->setIcon(QIcon(":/icon.png"));
    ui->titleBar->setUpdateVisible(false);

    connect(ui->titleBar, &TitleBar::minimizeClicked, this, &MainWindow::onMinimizeClicked);
    connect(ui->titleBar, &TitleBar::closeClicked, this, &MainWindow::onCloseClicked);
    connect(ui->titleBar, &TitleBar::updateClicked, this, &MainWindow::onUpdateClicked);
    connect(ui->titleBar, &TitleBar::settingsClicked, this, &MainWindow::onSettingsClicked);
}

void MainWindow::onMinimizeClicked() {
    showMinimized();
}

void MainWindow::onCloseClicked() {
    close();
}

void MainWindow::startUpdateCheck() {
    if (!m_updater) return;
    if (!m_settingsWidget || !m_settingsWidget->applyStoredSettings()) {
        return;
    }
    m_updater->checkForUpdates();
}

void MainWindow::setupInstallPath() {
    ui->leftBox->addWidget(m_installPathWidget);
    connect(m_installPathWidget, &InstallPathWidget::validPathSelected,
            this, &MainWindow::onInstallPathChanged);
}

void MainWindow::setupProfileManager() {
    m_profileManager->setModManager(m_modManager);
    m_profileManagerWidget->setProfileManager(m_profileManager);
    ui->leftBox->addWidget(m_profileManagerWidget);

    ActivityLogWidget *log = m_rightPanelWidget->getActivityLog();

    connect(m_profileManagerWidget, &ProfileManagerWidget::profileLoadRequested,
            this, [this, log](const QString &profileId) {
        m_modListWidget->refreshModList();
        log->addLogEntry("Profile Loaded", ActivityLogWidget::LogLevel::Info);
    });

    connect(m_profileManager, &ProfileManager::errorOccurred,
            this, [](const QString &error) {
        QMessageBox::warning(nullptr, "Profile Error", error);
    });
}

void MainWindow::setupModList() {
    ui->middleBox->addWidget(m_modListWidget);
    m_modListWidget->setModManager(m_modManager);
    m_modListWidget->setNexusServices(m_nexusClient, m_nexusAuth);
    m_modListWidget->setItchServices(m_itchClient, m_itchAuth);
    m_modListWidget->setUpdateService(m_modUpdateService);
}

void MainWindow::setupRightPanel() {
    ui->rightBox->addWidget(m_rightPanelWidget);
    m_rightPanelWidget->setModManager(m_modManager);

    connect(m_modListWidget, &ModListWidget::modSelectionChanged,
            m_rightPanelWidget, &RightPanelWidget::onModSelectionChanged);

    connect(m_rightPanelWidget, &RightPanelWidget::addModRequested,
            m_modListWidget, &ModListWidget::onAddModClicked);
    connect(m_rightPanelWidget, &RightPanelWidget::removeModRequested,
            m_modListWidget, &ModListWidget::onRemoveModClicked);
    connect(m_rightPanelWidget, &RightPanelWidget::moveUpRequested,
            m_modListWidget, &ModListWidget::onMoveUpClicked);
    connect(m_rightPanelWidget, &RightPanelWidget::moveDownRequested,
            m_modListWidget, &ModListWidget::onMoveDownClicked);

    connect(m_installPathWidget, &InstallPathWidget::validPathSelected,
            m_rightPanelWidget, &RightPanelWidget::setFoxholeInstallPath);

    connect(m_rightPanelWidget, &RightPanelWidget::errorOccurred,
            this, [](const QString &error) {
        QMessageBox::warning(nullptr, "Error", error);
    });

    // Connect activity log
    ActivityLogWidget *log = m_rightPanelWidget->getActivityLog();

    // ModListWidget logging
    connect(m_modListWidget, &ModListWidget::modAdded, this, [log](const QString &name) {
        log->addLogEntry("Mod Added: " + name, ActivityLogWidget::LogLevel::Success);
    });
    connect(m_modListWidget, &ModListWidget::modRemoved, this, [log](const QString &name) {
        log->addLogEntry("Mod Removed: " + name, ActivityLogWidget::LogLevel::Info);
    });
    connect(m_modListWidget, &ModListWidget::modReordered, this, [log]() {
        log->addLogEntry("Mods Reordered", ActivityLogWidget::LogLevel::Info);
    });

    // BackupWidget logging
    connect(m_rightPanelWidget->findChild<BackupWidget*>(), &BackupWidget::backupCreated,
            this, [log](int fileCount) {
        log->addLogEntry(QString("Backup Created: %1 files").arg(fileCount),
                        ActivityLogWidget::LogLevel::Success);
    });
    connect(m_rightPanelWidget->findChild<BackupWidget*>(), &BackupWidget::backupRestored,
            this, [log](const QString &backupName) {
        log->addLogEntry("Backup Restored: " + backupName, ActivityLogWidget::LogLevel::Success);
    });

    // LaunchWidget logging
    connect(m_rightPanelWidget->findChild<LaunchWidget*>(), &LaunchWidget::gameLaunched,
            this, [log](bool withMods) {
        QString mode = withMods ? "With Mods" : "Vanilla";
        log->addLogEntry("Game Launched: " + mode, ActivityLogWidget::LogLevel::Info);
    });
    connect(m_rightPanelWidget->findChild<LaunchWidget*>(), &LaunchWidget::modsRestored,
            this, [log](int modCount) {
        log->addLogEntry(QString("Mods Restored: %1 mods").arg(modCount),
                        ActivityLogWidget::LogLevel::Success);
    });
}

void MainWindow::setupSettingsOverlay() {
    m_settingsPage = ui->settingsPage;
    if (!m_settingsPage) {
        return;
    }
    auto *overlayLayout = ui->settingsLayout;
    if (!overlayLayout) {
        overlayLayout = new QVBoxLayout(m_settingsPage);
    }
    m_settingsWidget = new SettingsWidget(m_settingsPage, m_updater);
    m_settingsWidget->setNexusServices(m_nexusClient, m_nexusAuth);
    overlayLayout->addWidget(m_settingsWidget);

    connect(m_settingsWidget, &SettingsWidget::cancelRequested,
            this, &MainWindow::hideSettingsOverlay);
    connect(m_settingsWidget, &SettingsWidget::settingsApplied,
            this, &MainWindow::onSettingsApplied);
    connect(m_settingsWidget, &SettingsWidget::manualCheckRequested, this, [this]() {
        if (m_updater) {
            m_updater->checkForUpdates();
        }
    });
}

void MainWindow::loadSettings() {
    QSettings settings("TrenchKit", "FoxholeModManager");

    m_modListWidget->setLoadingState(true, "Loading mods");

    QFuture<bool> modLoadFuture = QtConcurrent::run([this]() {
        return m_modManager->loadMods();
    });
    m_modLoadWatcher->setFuture(modLoadFuture);

    QFuture<bool> profileLoadFuture = QtConcurrent::run([this]() {
        return m_profileManager->loadProfiles();
    });

    auto *profileWatcher = new QFutureWatcher<bool>(this);
    connect(profileWatcher, &QFutureWatcher<bool>::finished, this, [this, profileWatcher]() {
        bool success = profileWatcher->result();
        profileWatcher->deleteLater();

        if (success) {
            m_profileManagerWidget->refreshProfileList();
        }
    });
    profileWatcher->setFuture(profileLoadFuture);

    QString savedPath = settings.value("foxholeInstallPath").toString();

    if (!savedPath.isEmpty()) {
        m_modManager->setInstallPath(savedPath);

        QFuture<bool> validationFuture = QtConcurrent::run([savedPath]() {
            return FoxholeDetector::isValidInstallPath(savedPath);
        });

        auto *watcher = new QFutureWatcher<bool>(this);
        connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, savedPath]() {
            bool isValid = watcher->result();
            watcher->deleteLater();

            if (isValid) {
                m_installPathWidget->setInstallPath(savedPath);

                m_modListWidget->setLoadingState(true, "Detecting mods");
                QFuture<void> detectionFuture = QtConcurrent::run([this]() {
                    m_modManager->detectUnregisteredMods();
                });
                m_unregisteredModsWatcher->setFuture(detectionFuture);
            } else {
                m_installPathWidget->startAutoDetection();
            }
        });
        watcher->setFuture(validationFuture);
    } else {
        m_installPathWidget->startAutoDetection();
    }
}

void MainWindow::saveSettings() {
    QSettings settings("TrenchKit", "FoxholeModManager");
    settings.setValue("foxholeInstallPath", m_installPathWidget->installPath());
}

void MainWindow::onInstallPathChanged(const QString &path) {
    QSettings settings("TrenchKit", "FoxholeModManager");
    settings.setValue("foxholeInstallPath", path);

    m_modManager->setInstallPath(path);

    ActivityLogWidget *log = m_rightPanelWidget->getActivityLog();
    log->addLogEntry("Install Path Updated", ActivityLogWidget::LogLevel::Info);

    m_modListWidget->setLoadingState(true, "Detecting mods");
    QFuture<void> detectionFuture = QtConcurrent::run([this]() {
        m_modManager->detectUnregisteredMods();
    });
    m_unregisteredModsWatcher->setFuture(detectionFuture);
}

void MainWindow::onModsLoadComplete() {
    bool success = m_modLoadWatcher->result();

    m_modListWidget->setLoadingState(false);

    if (success) {
        m_modListWidget->refreshModList();

        if (m_modUpdateService) {
            QTimer::singleShot(0, this, [this]() {
                m_modUpdateService->checkAllModsForUpdates();
            });
        }
    }
}

void MainWindow::onUnregisteredModsDetectionComplete() {
    m_modListWidget->setLoadingState(false);
}

void MainWindow::onSettingsClicked() {
    showSettingsOverlay();
}

void MainWindow::onSettingsApplied(bool autoCheck) {
    m_updateAvailable = false;
    ui->titleBar->setUpdateVisible(false);
    hideSettingsOverlay();
    if (autoCheck && m_updater) {
        m_updater->checkForUpdates();
    }
}

void MainWindow::onUpdateClicked() {
    if (!m_updateAvailable) {
        QMessageBox::information(this, "Up To Date", "No updates are available.");
        return;
    }
    beginUpdateDownload();
}

void MainWindow::onUpdateCheckError(const QString &message) {
    qWarning() << "Updater:" << message;
    ActivityLogWidget *log = m_rightPanelWidget->getActivityLog();
    log->addLogEntry("Update check failed", ActivityLogWidget::LogLevel::Warning);
    if (m_settingsWidget) {
        m_settingsWidget->setCheckStatus("Update check failed");
    }
    if (m_updateDialog) {
        closeUpdateDialog();
        QMessageBox::warning(this, "Update Error", message);
    }
}

void MainWindow::onUpdateAvailable(const UpdaterService::ReleaseInfo &release) {
    m_updateRelease = release;
    m_updateAvailable = true;
    ui->titleBar->setUpdateVisible(true);
    ActivityLogWidget *log = m_rightPanelWidget->getActivityLog();
    log->addLogEntry("Update available", ActivityLogWidget::LogLevel::Info);
    if (m_settingsWidget) {
        m_settingsWidget->setCheckStatus(QString("Update available: %1").arg(release.version.toString()));
    }
}

void MainWindow::onUpdateUpToDate(const UpdaterService::ReleaseInfo &latest) {
    Q_UNUSED(latest);
    m_updateAvailable = false;
    ui->titleBar->setUpdateVisible(false);
    if (m_settingsWidget) {
        m_settingsWidget->setCheckStatus("Up to date");
    }
}

void MainWindow::onUpdateDownloadProgress(qint64 received, qint64 total) {
    if (!m_updateDialog) return;
    if (total > 0) {
        int percent = static_cast<int>((received * 100) / total);
        m_updateDialog->setRange(0, 100);
        m_updateDialog->setValue(percent);
        const double receivedMb = received / 1024.0 / 1024.0;
        const double totalMb = total / 1024.0 / 1024.0;
        m_updateDialog->setLabelText(
            QString("Downloading update (%1 / %2 MB)")
                .arg(receivedMb, 0, 'f', 1)
                .arg(totalMb, 0, 'f', 1));
        if (!m_updateInstallStarted && received >= total && !m_pendingUpdatePath.isEmpty()) {
            QMetaObject::invokeMethod(this, [this]() {
                if (!m_pendingUpdatePath.isEmpty()) {
                    onUpdateDownloadFinished(m_pendingUpdatePath);
                }
            }, Qt::QueuedConnection);
            return;
        }
    } else {
        m_updateDialog->setRange(0, 0);
        m_updateDialog->setLabelText("Downloading update...");
    }
}

void MainWindow::onUpdateDownloadFinished(const QString &savePath) {
    if (m_updateInstallStarted) {
        return;
    }
    m_updateInstallStarted = true;
    if (m_updateDialog) {
        m_updateDialog->setCancelButton(nullptr);
        m_updateDialog->setRange(0, 0);
        m_updateDialog->setLabelText("Installing update...");
    }
    const QString updatesDir = m_settingsWidget
        ? m_settingsWidget->resolvedDownloadDir()
        : defaultUpdatesDir();

    QString version = m_updateRelease.version.toString();
    if (version.isEmpty()) {
        version = m_updateRelease.tagName;
        if (version.startsWith('v') || version.startsWith('V')) {
            version = version.mid(1);
        }
    }

    QString error;
    if (!stageUpdate(savePath, version, updatesDir, &error)) {
        QMessageBox::warning(this, "Update Error", error);
        return;
    }

    const QString stagingDir = QDir(updatesDir)
                                   .filePath(QString("staging/%1").arg(version));
    launchUpdater(stagingDir, updatesDir);
}

void MainWindow::beginUpdateDownload() {
    const QString assetName = selectUpdateAssetName();
    if (assetName.isEmpty()) {
        QMessageBox::warning(this, "Update Error", "No compatible update asset was found.");
        return;
    }

    UpdaterService::Asset chosen;
    bool found = false;
    for (const auto &asset : m_updateRelease.assets) {
        if (asset.name.compare(assetName, Qt::CaseInsensitive) == 0) {
            chosen = asset;
            found = true;
            break;
        }
    }
    if (!found) {
        QMessageBox::warning(this, "Update Error", "No compatible update asset was found.");
        return;
    }

    const QString updatesDir = m_settingsWidget
        ? m_settingsWidget->resolvedDownloadDir()
        : defaultUpdatesDir();
    const QString savePath = QDir(updatesDir).filePath(chosen.name);

    QFileInfo existing(savePath);
    m_updateInstallStarted = false;
    m_pendingUpdatePath = savePath;
    m_pendingUpdateSize = chosen.sizeBytes;
    if (existing.exists() && chosen.sizeBytes > 0 && existing.size() == chosen.sizeBytes) {
        onUpdateDownloadFinished(savePath);
        return;
    }

    showUpdateDialog();
    m_updater->downloadAsset(chosen, savePath);
}

void MainWindow::showUpdateDialog() {
    if (m_updateDialog) {
        m_updateDialog->close();
        m_updateDialog->deleteLater();
    }
    m_updateDialog = new QProgressDialog("Downloading update...", "Cancel", 0, 100, this);
    m_updateDialog->setWindowTitle("Update Download");
    m_updateDialog->setWindowModality(Qt::ApplicationModal);
    m_updateDialog->setAutoClose(false);
    m_updateDialog->setAutoReset(false);
    connect(m_updateDialog, &QProgressDialog::canceled, this, [this]() {
        if (m_updater) {
            m_updater->cancelDownload();
        }
        closeUpdateDialog();
    });
    m_updateDialog->show();
}

void MainWindow::closeUpdateDialog() {
    if (!m_updateDialog) return;
    m_updateDialog->close();
    m_updateDialog->deleteLater();
    m_updateDialog = nullptr;
}

QString MainWindow::selectUpdateAssetName() const {
    QString platform;
#if defined(Q_OS_WIN)
    platform = "windows";
#elif defined(Q_OS_LINUX)
    platform = "linux";
#else
    return QString();
#endif

    QString version = m_updateRelease.version.toString();
    if (version.isEmpty()) {
        version = m_updateRelease.tagName;
        if (version.startsWith('v') || version.startsWith('V')) {
            version = version.mid(1);
        }
    }

    if (version.isEmpty()) {
        return QString();
    }

    return QString("%1-%2.zip").arg(platform, version);
}

bool MainWindow::stageUpdate(const QString &archivePath,
                             const QString &version,
                             const QString &updatesDir,
                             QString *error) {
    const QString stagingDir = QDir(updatesDir).filePath(QString("staging/%1").arg(version));

    QDir dir(stagingDir);
    if (dir.exists() && !dir.removeRecursively()) {
        if (error) {
            *error = "Failed to clear existing staging directory.";
        }
        return false;
    }
    if (!dir.mkpath(".")) {
        if (error) {
            *error = "Failed to create staging directory.";
        }
        return false;
    }

    QString extractError;
    if (!UpdateArchiveExtractor::extractZip(archivePath, stagingDir, &extractError)) {
        if (error) {
            *error = QString("Failed to extract update: %1").arg(extractError);
        }
        return false;
    }

    return true;
}

void MainWindow::launchUpdater(const QString &stagingDir, const QString &updatesDir) {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString exeName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();

#if defined(Q_OS_WIN)
    const QString updaterExe = QDir(appDir).filePath("updater.exe");
#else
    const QString updaterExe = QDir(appDir).filePath("updater");
#endif

    if (!QFileInfo::exists(updaterExe)) {
        QMessageBox::warning(this, "Update Error", "Updater helper was not found.");
        return;
    }

    QStringList args;
    args << "--install"
         << "--app-dir" << appDir
         << "--new-dir" << stagingDir
         << "--updates-dir" << updatesDir
         << "--exe-name" << exeName;

    qInfo() << "Updater: launching helper" << updaterExe;
    qint64 pid = 0;
    const bool started = QProcess::startDetached(updaterExe, args, appDir, &pid);
    if (!started) {
        QMessageBox::warning(this, "Update Error", "Failed to launch updater helper.");
        return;
    }
    QCoreApplication::quit();
}

void MainWindow::showSettingsOverlay() {
    if (!m_settingsPage || !ui->bodyStack || !ui->mainPage) return;
    if (m_settingsWidget) {
        m_settingsWidget->loadSettingsIntoUi();
    }
    ui->bodyStack->setCurrentWidget(m_settingsPage);
}

void MainWindow::hideSettingsOverlay() {
    if (!m_settingsPage || !ui->bodyStack || !ui->mainPage) return;
    ui->bodyStack->setCurrentWidget(ui->mainPage);
}
