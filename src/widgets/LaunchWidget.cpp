#include "LaunchWidget.h"
#include "../utils/ModManager.h"
#include "../utils/Theme.h"
#include <QToolButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QFile>

LaunchWidget::LaunchWidget(QWidget *parent)
    : QWidget(parent)
    , m_launchButton(new QToolButton(this))
{
    setupUi();
    setupConnections();
}

void LaunchWidget::setModManager(ModManager *modManager) {
    m_modManager = modManager;
}

void LaunchWidget::setFoxholeInstallPath(const QString &path) {
    m_foxholeInstallPath = path;
}

void LaunchWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN
    );
    layout->setSpacing(Theme::Spacing::CONTAINER_SPACING);

    m_launchButton->setText("Play with mods");
    m_launchButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_launchButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_launchButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QMenu *menu = new QMenu(this);
    QAction *playWithMods = menu->addAction("Play with mods");
    QAction *playWithoutMods = menu->addAction("Play without mods");

    connect(playWithMods, &QAction::triggered, this, [this, playWithMods]() {
        m_launchButton->setText(playWithMods->text());
    });

    connect(playWithoutMods, &QAction::triggered, this, [this, playWithoutMods]() {
        m_launchButton->setText(playWithoutMods->text());
    });

    m_launchButton->setMenu(menu);

    layout->addWidget(m_launchButton);

    setLayout(layout);
}

void LaunchWidget::setupConnections() {
    connect(m_launchButton, &QToolButton::clicked, this, [this]() {
        if (m_launchButton->text() == "Play with mods") {
            onLaunchWithMods();
        } else {
            onLaunchWithoutMods();
        }
    });
}

void LaunchWidget::onLaunchWithMods() {
    QString exePath = getFoxholeExecutablePath();

    if (exePath.isEmpty()) {
        emit errorOccurred("Foxhole executable not found. Please check your installation path.");
        return;
    }

    if (!QProcess::startDetached(exePath, QStringList(), m_foxholeInstallPath)) {
        emit errorOccurred("Failed to launch Foxhole");
    } else {
        emit gameLaunched(true);
    }
}

void LaunchWidget::onLaunchWithoutMods() {
    if (!m_modManager) {
        emit errorOccurred("Mod manager not initialized");
        return;
    }

    QString exePath = getFoxholeExecutablePath();

    if (exePath.isEmpty()) {
        emit errorOccurred("Foxhole executable not found. Please check your installation path.");
        return;
    }

    QList<ModInfo> mods = m_modManager->getMods();
    QStringList enabledModIds;

    for (const ModInfo &mod : mods) {
        if (mod.enabled) {
            enabledModIds.append(mod.id);
        }
    }

    if (!enabledModIds.isEmpty()) {
        auto reply = QMessageBox::question(
            this,
            "Launch without mods",
            QString("This will temporarily disable %1 enabled mod(s).\n\n"
                    "They will be automatically restored when the game closes.\n\n"
                    "Continue?").arg(enabledModIds.count()),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply != QMessageBox::Yes) {
            return;
        }

        m_modsToRestore = enabledModIds;

        for (const QString &modId : enabledModIds) {
            m_modManager->disableMod(modId);
        }
    }

    if (m_gameProcess) {
        m_gameProcess->deleteLater();
    }

    m_gameProcess = new QProcess(this);
    m_gameProcess->setWorkingDirectory(m_foxholeInstallPath);
    m_gameProcess->setProgram(exePath);

    connect(m_gameProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &LaunchWidget::onGameProcessFinished);

    m_gameProcess->start();

    if (!m_gameProcess->waitForStarted(5000)) {
        emit errorOccurred("Failed to launch Foxhole");

        for (const QString &modId : m_modsToRestore) {
            m_modManager->enableMod(modId);
        }
        m_modsToRestore.clear();
    } else {
        emit gameLaunched(false);
    }
}

void LaunchWidget::onGameProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    if (!m_modsToRestore.isEmpty() && m_modManager) {
        int modCount = m_modsToRestore.count();
        for (const QString &modId : m_modsToRestore) {
            m_modManager->enableMod(modId);
        }

        emit modsRestored(modCount);

        QMessageBox::information(this, "Mods Restored",
            QString("Restored %1 mod(s) that were disabled for vanilla gameplay.")
            .arg(modCount));

        m_modsToRestore.clear();
    }
}

QString LaunchWidget::getFoxholeExecutablePath() const {
    if (m_foxholeInstallPath.isEmpty()) {
        return QString();
    }

    QStringList possibleExes = {
        "War-Win64-Shipping.exe",
        "Foxhole.exe",
        "War.exe"
    };

    QDir rootDir(m_foxholeInstallPath);
    for (const QString &exe : possibleExes) {
        QString path = rootDir.filePath(exe);
        if (QFile::exists(path)) {
            return path;
        }
    }

    QDir warDir(m_foxholeInstallPath);
    if (warDir.cd("War") && warDir.cd("Binaries") && warDir.cd("Win64")) {
        for (const QString &exe : possibleExes) {
            QString path = warDir.filePath(exe);
            if (QFile::exists(path)) {
                return path;
            }
        }
    }

    return QString();
}
