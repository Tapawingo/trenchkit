#include "InstallPathWidget.h"
#include "GradientFrame.h"
#include "../utils/FoxholeDetector.h"
#include "../utils/Theme.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QtConcurrent>
#include <QStyle>

InstallPathWidget::InstallPathWidget(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(new QLabel(this))
    , m_pathLineEdit(new QLineEdit(this))
    , m_browseButton(new QPushButton(this))
    , m_statusLabel(new QLabel(this))
    , m_layout(new QVBoxLayout(this))
    , m_detectionWatcher(new QFutureWatcher<QString>(this))
    , m_loadingTimer(new QTimer(this))
{
    setupUi();
    setupConnections();
}

void InstallPathWidget::setupUi() {
    GradientFrame *frame = new GradientFrame(this);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);


    // Title
    m_titleLabel->setText("Foxhole Installation");
    m_titleLabel->setObjectName("installPathTitle");

    // Path input
    m_pathLineEdit->setPlaceholderText("Select Foxhole installation folder...");
    m_pathLineEdit->setObjectName("installPathInput");
    m_pathLineEdit->setFocusPolicy(Qt::ClickFocus);

    // Browse button
    QIcon browseIcon(":/icon_edit.png");
    m_browseButton->setObjectName("installPathBrowse");
    m_browseButton->setText("");
    m_browseButton->setIcon(browseIcon);
    m_browseButton->setFixedSize(32, 32);
    m_browseButton->setIconSize(QSize(16, 16));
    m_browseButton->setCursor(Qt::PointingHandCursor);

    // Status label
    m_statusLabel->setText("");
    m_statusLabel->setObjectName("installPathStatus");
    m_statusLabel->setWordWrap(true);

    // Layout
    auto *pathLayout = new QHBoxLayout();
    pathLayout->addWidget(m_pathLineEdit);
    pathLayout->addWidget(m_browseButton);

    m_layout->setContentsMargins(
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN
    );
    m_layout->setSpacing(Theme::Spacing::CONTAINER_SPACING);
    frameLayout->addWidget(m_titleLabel);
    frameLayout->addLayout(pathLayout);
    frameLayout->addWidget(m_statusLabel);
    m_layout->addWidget(frame);

    setLayout(m_layout);
}

void InstallPathWidget::setupConnections() {
    connect(m_browseButton, &QPushButton::clicked, this, &InstallPathWidget::onBrowseClicked);
    connect(m_pathLineEdit, &QLineEdit::textChanged, this, &InstallPathWidget::onPathEdited);
    connect(m_detectionWatcher, &QFutureWatcher<QString>::finished, this, &InstallPathWidget::onDetectionComplete);
    connect(m_loadingTimer, &QTimer::timeout, this, &InstallPathWidget::updateLoadingAnimation);
}

QString InstallPathWidget::installPath() const {
    return m_currentPath;
}

void InstallPathWidget::setInstallPath(const QString &path) {
    m_pathLineEdit->setText(path);
    validatePath(path);
}

bool InstallPathWidget::isValidPath() const {
    return m_isValid;
}

void InstallPathWidget::onBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Foxhole Installation Folder",
        m_currentPath.isEmpty() ? QDir::homePath() : m_currentPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        setInstallPath(dir);
    }
}

void InstallPathWidget::onPathEdited(const QString &text) {
    validatePath(text);
}

void InstallPathWidget::validatePath(const QString &path) {
    m_currentPath = path;
    m_isValid = checkFoxholeInstallation(path);

    if (path.isEmpty()) {
        m_statusLabel->setText("");
        m_statusLabel->setProperty("status", "");
        style()->unpolish(m_statusLabel);
        style()->polish(m_statusLabel);
    } else if (m_isValid) {
        m_statusLabel->setText("✓ Valid Foxhole installation");
        m_statusLabel->setProperty("status", "valid");
        style()->unpolish(m_statusLabel);
        style()->polish(m_statusLabel);
        emit validPathSelected(path);
    } else {
        m_statusLabel->setText("✗ Invalid Foxhole installation");
        m_statusLabel->setProperty("status", "invalid");
        style()->unpolish(m_statusLabel);
        style()->polish(m_statusLabel);
    }

    emit pathChanged(path);
}

bool InstallPathWidget::checkFoxholeInstallation(const QString &path) const {
    if (path.isEmpty()) {
        return false;
    }

    QDir dir(path);
    if (!dir.exists()) {
        return false;
    }

    // List of possible Foxhole executables
    QStringList possibleExes = {
        "Foxhole.exe",
        "FoxholeClient.exe",
        "FoxholeClient-Win64-Shipping.exe"
    };

    // Check for executable in root directory
    for (const QString &exe : possibleExes) {
        if (QFile::exists(dir.filePath(exe))) {
            return true;
        }
    }

    // Check for Foxhole executable in War/Binaries/Win64/
    QDir warDir = dir;
    if (warDir.cd("War") && warDir.cd("Binaries") && warDir.cd("Win64")) {
        for (const QString &exe : possibleExes) {
            if (QFile::exists(warDir.filePath(exe))) {
                return true;
            }
        }
    }

    // Check for other Foxhole-specific directories
    if (QDir(dir.filePath("War/Content/Paks")).exists() ||
        QDir(dir.filePath("War/Content/Movies")).exists()) {
        return true;
    }

    return false;
}

void InstallPathWidget::startAutoDetection() {
    setLoadingState(true);

    // Run detection in background thread
    QFuture<QString> future = QtConcurrent::run([]() {
        return FoxholeDetector::detectInstallPath();
    });

    m_detectionWatcher->setFuture(future);
}

void InstallPathWidget::onDetectionComplete() {
    QString detectedPath = m_detectionWatcher->result();

    setLoadingState(false);

    if (!detectedPath.isEmpty()) {
        setInstallPath(detectedPath);
    }

    emit detectionFinished(detectedPath);
}

void InstallPathWidget::updateLoadingAnimation() {
    m_loadingDots = (m_loadingDots + 1) % 4;
    QString dots(m_loadingDots, '.');
    m_statusLabel->setText("Detecting Foxhole installation" + dots);
}

void InstallPathWidget::setLoadingState(bool loading) {
    if (loading) {
        m_loadingDots = 0;
        m_statusLabel->setText("Detecting Foxhole installation.");
        m_statusLabel->setProperty("status", "loading");
        style()->unpolish(m_statusLabel);
        style()->polish(m_statusLabel);
        m_pathLineEdit->setEnabled(false);
        m_browseButton->setEnabled(false);
        m_loadingTimer->start(500);
    } else {
        m_loadingTimer->stop();
        m_pathLineEdit->setEnabled(true);
        m_browseButton->setEnabled(true);
    }
}
