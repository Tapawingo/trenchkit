#include "BackupWidget.h"
#include "GradientFrame.h"
#include "../modals/ModalManager.h"
#include "../modals/MessageModal.h"
#include "../modals/content/BackupSelectionModalContent.h"
#include "../utils/ModManager.h"
#include "../utils/Theme.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

BackupWidget::BackupWidget(QWidget *parent)
    : QWidget(parent)
    , m_createBackupButton(new QPushButton("Create Backup", this))
    , m_restoreBackupButton(new QPushButton("Restore Backup", this))
{
    setupUi();
    setupConnections();
}

void BackupWidget::setModManager(ModManager *modManager) {
    m_modManager = modManager;
}

void BackupWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN
    );
    layout->setSpacing(Theme::Spacing::CONTAINER_SPACING);

    GradientFrame *frame = new GradientFrame(this);
    layout->addWidget(frame);

    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLabel = new QLabel("Backup", this);
    titleLabel->setObjectName("backupTitle");
    frameLayout->addWidget(titleLabel);

    frameLayout->addWidget(m_createBackupButton);
    frameLayout->addWidget(m_restoreBackupButton);

    setLayout(layout);
}

void BackupWidget::setupConnections() {
    connect(m_createBackupButton, &QPushButton::clicked, this, &BackupWidget::onCreateBackupClicked);
    connect(m_restoreBackupButton, &QPushButton::clicked, this, &BackupWidget::onRestoreBackupClicked);
}

void BackupWidget::onCreateBackupClicked() {
    if (!m_modManager) {
        emit errorOccurred("Mod manager not initialized");
        return;
    }

    if (!m_modalManager) {
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
    int modCount = m_modManager->getMods().count();
    backupInfo["modCount"] = modCount;

    QFile infoFile(backupDir + "/backup_info.json");
    if (infoFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(backupInfo);
        infoFile.write(doc.toJson());
        infoFile.close();
    }

    emit backupCreated(modCount);
    MessageModal::information(m_modalManager, "Success", "Backup created successfully at:\n" + backupDir);
}

void BackupWidget::onRestoreBackupClicked() {
    if (!m_modManager) {
        emit errorOccurred("Mod manager not initialized");
        return;
    }

    if (!m_modalManager) {
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

    QStringList displayNames;
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

        displayNames.append(displayText);
    }

    auto *selectionModal = new BackupSelectionModalContent(backups, displayNames);
    connect(selectionModal, &BackupSelectionModalContent::accepted, this, [this, selectionModal, backupsPath]() {
        QString selectedBackup = selectionModal->getSelectedBackup();
        if (selectedBackup.isEmpty()) {
            return;
        }

        auto *confirmModal = new MessageModal(
            "Confirm Restore",
            "This will replace your current mod configuration. Continue?",
            MessageModal::Question,
            MessageModal::Yes | MessageModal::No
        );
        connect(confirmModal, &MessageModal::finished, this, [this, confirmModal, backupsPath, selectedBackup]() {
            if (confirmModal->clickedButton() != MessageModal::Yes) {
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

            emit backupRestored(selectedBackup);
            MessageModal::information(m_modalManager, "Success", "Backup restored successfully");
        });
        m_modalManager->showModal(confirmModal);
    });
    m_modalManager->showModal(selectionModal);
}

QString BackupWidget::getBackupsPath() const {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/backups";
}
