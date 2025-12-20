#include "RightPanelWidget.h"
#include "GradientFrame.h"
#include "../utils/ModManager.h"
#include "../utils/Theme.h"
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>

RightPanelWidget::RightPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void RightPanelWidget::setModManager(ModManager *modManager) {
    m_modManager = modManager;
}

void RightPanelWidget::setFoxholeInstallPath(const QString &path) {
    m_foxholeInstallPath = path;
}

void RightPanelWidget::onModSelectionChanged(int selectedRow, int totalMods) {
    m_selectedRow = selectedRow;
    m_totalMods = totalMods;

    bool hasSelection = selectedRow >= 0;
    bool isFirstRow = selectedRow == 0;
    bool isLastRow = selectedRow == totalMods - 1;

    m_removeButton->setEnabled(hasSelection);
    m_moveUpButton->setEnabled(hasSelection && !isFirstRow);
    m_moveDownButton->setEnabled(hasSelection && !isLastRow);
}

void RightPanelWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN
    );
    layout->setSpacing(Theme::Spacing::RIGHT_PANEL_SECTION_SPACING);

    setupActionsSection();
    setupBackupSection();

    layout->addStretch();

    setupLaunchSection();

    setLayout(layout);
}

void RightPanelWidget::setupActionsSection() {
    auto *layout = qobject_cast<QVBoxLayout*>(this->layout());

    GradientFrame *frame = new GradientFrame(this);

    layout->addWidget(frame);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLabel = new QLabel("ACTIONS", this);
    titleLabel->setObjectName("modActionsTitle");
    frameLayout->addWidget(titleLabel);

    m_addButton = new QPushButton("Add Mod", this);
    connect(m_addButton, &QPushButton::clicked, this, &RightPanelWidget::onAddModClicked);
    frameLayout->addWidget(m_addButton);

    m_removeButton = new QPushButton("Remove Mod", this);
    m_removeButton->setEnabled(false);
    connect(m_removeButton, &QPushButton::clicked, this, &RightPanelWidget::onRemoveModClicked);
    frameLayout->addWidget(m_removeButton);

    m_moveUpButton = new QPushButton("Move Up", this);
    m_moveUpButton->setEnabled(false);
    connect(m_moveUpButton, &QPushButton::clicked, this, &RightPanelWidget::onMoveUpClicked);
    frameLayout->addWidget(m_moveUpButton);

    m_moveDownButton = new QPushButton("Move Down", this);
    m_moveDownButton->setEnabled(false);
    connect(m_moveDownButton, &QPushButton::clicked, this, &RightPanelWidget::onMoveDownClicked);
    frameLayout->addWidget(m_moveDownButton);

    frameLayout->addWidget(createSeparator());

    m_exploreFolderButton = new QPushButton("Explore Mod Folder", this);
    connect(m_exploreFolderButton, &QPushButton::clicked, this, &RightPanelWidget::onExploreFolderClicked);
    frameLayout->addWidget(m_exploreFolderButton);
}

void RightPanelWidget::setupBackupSection() {
    auto *layout = qobject_cast<QVBoxLayout*>(this->layout());

    GradientFrame *frame = new GradientFrame(this);

    layout->addWidget(frame);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLabel = new QLabel("BACKUP", this);
    titleLabel->setObjectName("backupTitle");
    frameLayout->addWidget(titleLabel);

    m_createBackupButton = new QPushButton("Create Backup", this);
    connect(m_createBackupButton, &QPushButton::clicked, this, &RightPanelWidget::onCreateBackupClicked);
    frameLayout->addWidget(m_createBackupButton);

    m_restoreBackupButton = new QPushButton("Restore Backup", this);
    connect(m_restoreBackupButton, &QPushButton::clicked, this, &RightPanelWidget::onRestoreBackupClicked);
    frameLayout->addWidget(m_restoreBackupButton);
}

void RightPanelWidget::setupLaunchSection() {
    auto *layout = qobject_cast<QVBoxLayout*>(this->layout());

    m_launchButton = new QToolButton(this);
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

    connect(m_launchButton, &QToolButton::clicked, this, [this]() {
        if (m_launchButton->text() == "Play with mods") {
            onLaunchWithMods();
        } else {
            onLaunchWithoutMods();
        }
    });

    layout->addWidget(m_launchButton);
}

QFrame* RightPanelWidget::createSeparator() {
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedHeight(1);
    return line;
}

void RightPanelWidget::onAddModClicked() {
    emit addModRequested();
}

void RightPanelWidget::onRemoveModClicked() {
    emit removeModRequested();
}

void RightPanelWidget::onMoveUpClicked() {
    emit moveUpRequested();
}

void RightPanelWidget::onMoveDownClicked() {
    emit moveDownRequested();
}

void RightPanelWidget::onExploreFolderClicked() {
    if (m_foxholeInstallPath.isEmpty()) {
        emit errorOccurred("Foxhole installation path not set");
        return;
    }

    QString paksPath = m_foxholeInstallPath + "/War/Content/Paks";
    QDir dir(paksPath);

    if (!dir.exists()) {
        emit errorOccurred("Paks folder not found: " + paksPath);
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(paksPath));
}

void RightPanelWidget::onCreateBackupClicked() {
    if (!m_modManager) {
        emit errorOccurred("Mod manager not initialized");
        return;
    }

    QString backupDir = getBackupsPath() + "/backup_" +
                        QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");

    QDir().mkpath(backupDir);

    QString modsStoragePath = m_modManager->getModsStoragePath();
    QString sourcePath = modsStoragePath + "/mods.json";
    QString destPath = backupDir + "/mods.json";

    if (!QFile::exists(sourcePath)) {
        emit errorOccurred("No mods metadata file to backup");
        return;
    }

    if (!QFile::copy(sourcePath, destPath)) {
        emit errorOccurred("Failed to create backup");
        return;
    }

    QJsonObject backupInfo;
    backupInfo["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    backupInfo["modCount"] = m_modManager->getMods().count();

    QFile infoFile(backupDir + "/backup_info.json");
    if (infoFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(backupInfo);
        infoFile.write(doc.toJson());
        infoFile.close();
    }

    QMessageBox::information(this, "Success", "Backup created successfully at:\n" + backupDir);
}

void RightPanelWidget::onRestoreBackupClicked() {
    if (!m_modManager) {
        emit errorOccurred("Mod manager not initialized");
        return;
    }

    QString backupsPath = getBackupsPath();
    QDir backupsDir(backupsPath);

    if (!backupsDir.exists()) {
        emit errorOccurred("No backups found");
        return;
    }

    QStringList backups = backupsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);

    if (backups.isEmpty()) {
        emit errorOccurred("No backups available");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Restore Backup");
    dialog.setMinimumWidth(400);

    auto *layout = new QVBoxLayout(&dialog);

    auto *label = new QLabel("Select a backup to restore:", &dialog);
    layout->addWidget(label);

    auto *listWidget = new QListWidget(&dialog);
    for (const QString &backup : backups) {
        QString infoPath = backupsPath + "/" + backup + "/backup_info.json";
        QString displayText = backup;

        QFile infoFile(infoPath);
        if (infoFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(infoFile.readAll());
            if (doc.isObject()) {
                QJsonObject info = doc.object();
                int modCount = info["modCount"].toInt();
                displayText += QString(" (%1 mods)").arg(modCount);
            }
            infoFile.close();
        }

        listWidget->addItem(displayText);
    }
    layout->addWidget(listWidget);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    if (dialog.exec() != QDialog::Accepted || !listWidget->currentItem()) {
        return;
    }

    int selectedIndex = listWidget->currentRow();
    QString selectedBackup = backups[selectedIndex];

    auto reply = QMessageBox::question(
        this,
        "Confirm Restore",
        "This will replace your current mod configuration. Continue?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    QString backupPath = backupsPath + "/" + selectedBackup + "/mods.json";
    QString modsStoragePath = m_modManager->getModsStoragePath();
    QString destPath = modsStoragePath + "/mods.json";

    QFile::remove(destPath);

    if (!QFile::copy(backupPath, destPath)) {
        emit errorOccurred("Failed to restore backup");
        return;
    }

    m_modManager->loadMods();

    QMessageBox::information(this, "Success", "Backup restored successfully");
}

void RightPanelWidget::onLaunchWithMods() {
    QString exePath = getFoxholeExecutablePath();

    if (exePath.isEmpty()) {
        emit errorOccurred("Foxhole executable not found. Please check your installation path.");
        return;
    }

    if (!QProcess::startDetached(exePath, QStringList(), m_foxholeInstallPath)) {
        emit errorOccurred("Failed to launch Foxhole");
    }
}

void RightPanelWidget::onLaunchWithoutMods() {
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
            this, &RightPanelWidget::onGameProcessFinished);

    m_gameProcess->start();

    if (!m_gameProcess->waitForStarted(5000)) {
        emit errorOccurred("Failed to launch Foxhole");

        for (const QString &modId : m_modsToRestore) {
            m_modManager->enableMod(modId);
        }
        m_modsToRestore.clear();
    }
}

void RightPanelWidget::onGameProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    if (!m_modsToRestore.isEmpty() && m_modManager) {
        for (const QString &modId : m_modsToRestore) {
            m_modManager->enableMod(modId);
        }

        QMessageBox::information(this, "Mods Restored",
            QString("Restored %1 mod(s) that were disabled for vanilla gameplay.")
            .arg(m_modsToRestore.count()));

        m_modsToRestore.clear();
    }
}

QString RightPanelWidget::getFoxholeExecutablePath() const {
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

QString RightPanelWidget::getBackupsPath() const {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/backups";
}
