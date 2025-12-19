#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "widgets/InstallPathWidget.h"
#include "widgets/ModListWidget.h"
#include "utils/FoxholeDetector.h"
#include "utils/ModManager.h"
#include <QSettings>
#include <QShowEvent>
#include <QTimer>
#include <QtConcurrent>
#include <QFutureWatcher>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_installPathWidget(new InstallPathWidget(this))
    , m_modListWidget(new ModListWidget(this))
    , m_modManager(new ModManager(this))
    , m_modLoadWatcher(new QFutureWatcher<bool>(this))
    , m_unregisteredModsWatcher(new QFutureWatcher<void>(this))
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/icon.png"));
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    centralWidget()->setStyleSheet(R"(
        QWidget#centralwidget {
            background-color: #1e1e1e;
            border-bottom-left-radius: 8px;
            border-bottom-right-radius: 8px;
        }
    )");

    setupTitleBar();
    setupInstallPath();
    setupModList();

    connect(m_modLoadWatcher, &QFutureWatcher<bool>::finished,
            this, &MainWindow::onModsLoadComplete);
    connect(m_unregisteredModsWatcher, &QFutureWatcher<void>::finished,
            this, &MainWindow::onUnregisteredModsDetectionComplete);

    QSize windowSize(1000, 700);
    setMinimumSize(windowSize);
    setMaximumSize(windowSize);
    resize(windowSize);
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

void MainWindow::setupTitleBar() {
    ui->titleBar->setTitle("TrenchKit - Foxhole Mod Manager");
    ui->titleBar->setIcon(QIcon(":/icon.png"));

    connect(ui->titleBar, &TitleBar::minimizeClicked, this, &MainWindow::onMinimizeClicked);
    connect(ui->titleBar, &TitleBar::closeClicked, this, &MainWindow::onCloseClicked);
}

void MainWindow::onMinimizeClicked() {
    showMinimized();
}

void MainWindow::onCloseClicked() {
    close();
}

void MainWindow::setupInstallPath() {
    ui->leftBox->addWidget(m_installPathWidget);
    connect(m_installPathWidget, &InstallPathWidget::validPathSelected,
            this, &MainWindow::onInstallPathChanged);
}

void MainWindow::setupModList() {
    ui->middleBox->addWidget(m_modListWidget);
    m_modListWidget->setModManager(m_modManager);
}

void MainWindow::loadSettings() {
    QSettings settings("TrenchKit", "FoxholeModManager");

    m_modListWidget->setLoadingState(true, "Loading mods");

    QFuture<bool> modLoadFuture = QtConcurrent::run([this]() {
        return m_modManager->loadMods();
    });
    m_modLoadWatcher->setFuture(modLoadFuture);

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
    }
}

void MainWindow::onUnregisteredModsDetectionComplete() {
    m_modListWidget->setLoadingState(false);
}
