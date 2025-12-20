#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "widgets/InstallPathWidget.h"
#include "widgets/ProfileManagerWidget.h"
#include "widgets/ModListWidget.h"
#include "widgets/RightPanelWidget.h"
#include "utils/FoxholeDetector.h"
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
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/icon.png"));
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setStyleSheet(Theme::getStyleSheet());

    setupTitleBar();
    setupInstallPath();
    setupProfileManager();
    setupModList();
    setupRightPanel();

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
    if (watched == ui->centralwidget && event->type() == QEvent::Paint) {
        QPaintEvent *paintEvent = static_cast<QPaintEvent*>(event);
        QPainter painter(ui->centralwidget);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        QPixmap texture(":/tex_main.png");
        if (!texture.isNull()) {
            QRect widgetRect = ui->centralwidget->rect();
            QPixmap scaledTexture = texture.scaled(widgetRect.size(),
                                                   Qt::KeepAspectRatioByExpanding,
                                                   Qt::SmoothTransformation);

            int x = (widgetRect.width() - scaledTexture.width()) / 2;
            int y = (widgetRect.height() - scaledTexture.height()) / 2;

            painter.drawPixmap(x, y, scaledTexture);
        }

        return true;
    }

    return QMainWindow::eventFilter(watched, event);
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

void MainWindow::setupProfileManager() {
    m_profileManager->setModManager(m_modManager);
    m_profileManagerWidget->setProfileManager(m_profileManager);
    ui->leftBox->addWidget(m_profileManagerWidget);

    connect(m_profileManagerWidget, &ProfileManagerWidget::profileLoadRequested,
            m_modListWidget, &ModListWidget::refreshModList);

    connect(m_profileManager, &ProfileManager::errorOccurred,
            this, [](const QString &error) {
        QMessageBox::warning(nullptr, "Profile Error", error);
    });
}

void MainWindow::setupModList() {
    ui->middleBox->addWidget(m_modListWidget);
    m_modListWidget->setModManager(m_modManager);
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
