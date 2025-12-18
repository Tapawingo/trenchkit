#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "InstallPathWidget.h"
#include "FoxholeDetector.h"
#include <QSettings>

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
    loadSettings();

    // Apply window styling
    centralWidget()->setStyleSheet(R"(
        QWidget#centralwidget {
            background-color: #1e1e1e;
            border-bottom-left-radius: 8px;
            border-bottom-right-radius: 8px;
        }
    )");
}

MainWindow::~MainWindow() {
    saveSettings();
    delete ui;
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

    if (!savedPath.isEmpty() && FoxholeDetector::isValidInstallPath(savedPath)) {
        // Use saved path if it's still valid
        m_installPathWidget->setInstallPath(savedPath);
    } else {
        // Try to auto-detect
        QString detectedPath = FoxholeDetector::detectInstallPath();
        if (!detectedPath.isEmpty()) {
            m_installPathWidget->setInstallPath(detectedPath);
        }
    }

    // Restore window geometry
    if (settings.contains("windowGeometry")) {
        restoreGeometry(settings.value("windowGeometry").toByteArray());
    } else {
        // Default size
        resize(1000, 700);
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
