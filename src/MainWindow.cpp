#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "widgets/InstallPathWidget.h"
#include "utils/FoxholeDetector.h"
#include <QSettings>
#include <QShowEvent>
#include <QTimer>
#include <QtConcurrent>
#include <QFutureWatcher>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_installPathWidget(new InstallPathWidget(this))
{
    ui->setupUi(this);

    // Set window icon
    setWindowIcon(QIcon(":/icon.png"));

    // Make window frameless for custom titlebar
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setupTitleBar();
    setupInstallPath();

    // Apply window styling
    centralWidget()->setStyleSheet(R"(
        QWidget#centralwidget {
            background-color: #1e1e1e;
            border-bottom-left-radius: 8px;
            border-bottom-right-radius: 8px;
        }
    )");

    // Restore window geometry from settings immediately (lightweight)
    QSettings settings("TrenchKit", "FoxholeModManager");
    if (settings.contains("windowGeometry")) {
        restoreGeometry(settings.value("windowGeometry").toByteArray());
    } else {
        resize(1000, 700);
    }
}

MainWindow::~MainWindow() {
    saveSettings();
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);

    // Only load settings on first show to avoid blocking window appearance
    if (m_firstShow) {
        m_firstShow = false;
        // Use QTimer::singleShot to defer loading until event loop starts
        QTimer::singleShot(0, this, &MainWindow::loadSettings);
    }
}

void MainWindow::setupTitleBar() {
    // Set titlebar text and icon
    ui->titleBar->setTitle("TrenchKit - Foxhole Mod Manager");
    ui->titleBar->setIcon(QIcon(":/icon.png"));

    // Connect titlebar signals to window operations
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
    // Add install path widget to leftBox
    ui->leftBox->addWidget(m_installPathWidget);

    // Connect signal
    connect(m_installPathWidget, &InstallPathWidget::validPathSelected,
            this, &MainWindow::onInstallPathChanged);
}

void MainWindow::loadSettings() {
    QSettings settings("TrenchKit", "FoxholeModManager");

    // Try to load saved installation path
    QString savedPath = settings.value("foxholeInstallPath").toString();

    if (!savedPath.isEmpty()) {
        // Validate the saved path asynchronously
        QFuture<bool> validationFuture = QtConcurrent::run([savedPath]() {
            return FoxholeDetector::isValidInstallPath(savedPath);
        });

        // Use QFutureWatcher for the validation result
        auto *watcher = new QFutureWatcher<bool>(this);
        connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, savedPath]() {
            bool isValid = watcher->result();
            watcher->deleteLater();

            if (isValid) {
                // Use saved path if it's still valid
                m_installPathWidget->setInstallPath(savedPath);
            } else {
                // Invalid saved path, try to auto-detect
                m_installPathWidget->startAutoDetection();
            }
        });
        watcher->setFuture(validationFuture);
    } else {
        // No saved path, try to auto-detect asynchronously
        m_installPathWidget->startAutoDetection();
    }
}

void MainWindow::saveSettings() {
    QSettings settings("TrenchKit", "FoxholeModManager");

    // Save installation path
    settings.setValue("foxholeInstallPath", m_installPathWidget->installPath());

    // Save window geometry
    settings.setValue("windowGeometry", saveGeometry());
}

void MainWindow::onInstallPathChanged(const QString &path) {
    // Save immediately when a valid path is selected
    QSettings settings("TrenchKit", "FoxholeModManager");
    settings.setValue("foxholeInstallPath", path);
}
