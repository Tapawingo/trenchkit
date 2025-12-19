#include "RightPanelWidget.h"
#include "../utils/ModManager.h"
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
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
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(12);

    setupActionsSection();
    setupBackupSection();

    layout->addStretch();

    setupLaunchSection();

    setLayout(layout);
}

void RightPanelWidget::setupActionsSection() {
    QString buttonStyle = R"(
        QPushButton {
            background-color: #0e639c;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1177bb;
        }
        QPushButton:pressed {
            background-color: #0d5689;
        }
        QPushButton:disabled {
            background-color: #3d3d3d;
            color: #888888;
        }
    )";

    auto *layout = qobject_cast<QVBoxLayout*>(this->layout());

    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);

    layout->addWidget(frame);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLabel = new QLabel("ACTIONS", this);
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            margin-bottom: 4px;
        }
    )");
    frameLayout->addWidget(titleLabel);

    m_addButton = new QPushButton("Add Mod", this);
    m_addButton->setStyleSheet(buttonStyle);
    connect(m_addButton, &QPushButton::clicked, this, &RightPanelWidget::onAddModClicked);
    frameLayout->addWidget(m_addButton);

    m_removeButton = new QPushButton("Remove Mod", this);
    m_removeButton->setStyleSheet(buttonStyle);
    m_removeButton->setEnabled(false);
    connect(m_removeButton, &QPushButton::clicked, this, &RightPanelWidget::onRemoveModClicked);
    frameLayout->addWidget(m_removeButton);

    m_moveUpButton = new QPushButton("Move Up", this);
    m_moveUpButton->setStyleSheet(buttonStyle);
    m_moveUpButton->setEnabled(false);
    connect(m_moveUpButton, &QPushButton::clicked, this, &RightPanelWidget::onMoveUpClicked);
    frameLayout->addWidget(m_moveUpButton);

    m_moveDownButton = new QPushButton("Move Down", this);
    m_moveDownButton->setStyleSheet(buttonStyle);
    m_moveDownButton->setEnabled(false);
    connect(m_moveDownButton, &QPushButton::clicked, this, &RightPanelWidget::onMoveDownClicked);
    frameLayout->addWidget(m_moveDownButton);

    frameLayout->addWidget(createSeparator());

    m_exploreFolderButton = new QPushButton("Explore Mod Folder", this);
    m_exploreFolderButton->setStyleSheet(buttonStyle);
    connect(m_exploreFolderButton, &QPushButton::clicked, this, &RightPanelWidget::onExploreFolderClicked);
    frameLayout->addWidget(m_exploreFolderButton);
}

void RightPanelWidget::setupBackupSection() {
    QString buttonStyle = R"(
        QPushButton {
            background-color: #0e639c;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1177bb;
        }
        QPushButton:pressed {
            background-color: #0d5689;
        }
        QPushButton:disabled {
            background-color: #3d3d3d;
            color: #888888;
        }
    )";

    auto *layout = qobject_cast<QVBoxLayout*>(this->layout());

    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);

    layout->addWidget(frame);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLabel = new QLabel("BACKUP", this);
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            margin-bottom: 4px;
        }
    )");
    frameLayout->addWidget(titleLabel);

    m_createBackupButton = new QPushButton("Create Backup", this);
    m_createBackupButton->setStyleSheet(buttonStyle);
    connect(m_createBackupButton, &QPushButton::clicked, this, &RightPanelWidget::onCreateBackupClicked);
    frameLayout->addWidget(m_createBackupButton);

    m_restoreBackupButton = new QPushButton("Restore Backup", this);
    m_restoreBackupButton->setStyleSheet(buttonStyle);
    connect(m_restoreBackupButton, &QPushButton::clicked, this, &RightPanelWidget::onRestoreBackupClicked);
    frameLayout->addWidget(m_restoreBackupButton);
}

void RightPanelWidget::setupLaunchSection() {
    QString toolButtonStyle = R"(
        QToolButton {
            background-color: #0e639c;
            color: white;
            border: none;
            padding: 12px 20px;
            border-radius: 4px;
            font-weight: bold;
            font-size: 13px;
        }
        QToolButton:hover {
            background-color: #1177bb;
        }
        QToolButton:pressed {
            background-color: #0d5689;
        }
        QToolButton::menu-button {
            background-color: #0e639c;
            border: none;
            border-left: 1px solid #0a4f7a;
            border-top-right-radius: 4px;
            border-bottom-right-radius: 4px;
            width: 18px;
        }
        QToolButton::menu-button:hover {
            background-color: #1177bb;
        }
        QToolButton::menu-button:pressed {
            background-color: #0d5689;
        }
        QToolButton::menu-indicator {
            subcontrol-position: right center;
            subcontrol-origin: padding;
            font-size: 10px;
            left: -6px;
            width: 10px;
            height: 10px;
        }
    )";

    auto *layout = qobject_cast<QVBoxLayout*>(this->layout());

    m_launchButton = new QToolButton(this);
    m_launchButton->setText("Play with mods");
    m_launchButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_launchButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_launchButton->setStyleSheet(toolButtonStyle);
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
    line->setStyleSheet("background-color: #404040; max-height: 1px;");
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
                    "You will need to manually re-enable them after playing.\n\n"
                    "Continue?").arg(enabledModIds.count()),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply != QMessageBox::Yes) {
            return;
        }

        for (const QString &modId : enabledModIds) {
            m_modManager->disableMod(modId);
        }
    }

    if (!QProcess::startDetached(exePath, QStringList(), m_foxholeInstallPath)) {
        emit errorOccurred("Failed to launch Foxhole");

        for (const QString &modId : enabledModIds) {
            m_modManager->enableMod(modId);
        }
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
