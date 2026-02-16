#include "BackupWidget.h"
#include "common/widgets/GradientFrame.h"
#include "common/modals/ModalManager.h"
#include "common/modals/MessageModal.h"
#include "modals/mod_manager/BackupSelectionModalContent.h"
#include "core/managers/ModManager.h"
#include "core/utils/Theme.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

BackupWidget::BackupWidget(QWidget *parent)
    : QWidget(parent)
    , m_createBackupButton(new QPushButton(this))
    , m_restoreBackupButton(new QPushButton(this))
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

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("backupTitle");
    frameLayout->addWidget(m_titleLabel);

    m_createBackupButton->setCursor(Qt::PointingHandCursor);
    m_restoreBackupButton->setCursor(Qt::PointingHandCursor);
    frameLayout->addWidget(m_createBackupButton);
    frameLayout->addWidget(m_restoreBackupButton);

    setLayout(layout);

    retranslateUi();
}

void BackupWidget::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void BackupWidget::retranslateUi() {
    m_titleLabel->setText(tr("Backup"));
    m_createBackupButton->setText(tr("Create Backup"));
    m_restoreBackupButton->setText(tr("Restore Backup"));
}

void BackupWidget::setupConnections() {
    connect(m_createBackupButton, &QPushButton::clicked, this, &BackupWidget::onCreateBackupClicked);
    connect(m_restoreBackupButton, &QPushButton::clicked, this, &BackupWidget::onRestoreBackupClicked);
}

void BackupWidget::onCreateBackupClicked() {
    if (!m_modManager) {
        emit errorOccurred(tr("Mod manager not initialized"));
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
        emit errorOccurred(tr("No mods metadata file to backup"));
        return;
    }

    if (!QFile::copy(sourcePath, destPath)) {
        emit errorOccurred(tr("Failed to create backup"));
        return;
    }

    QDir modsDir(modsStoragePath);
    QStringList modFiles = modsDir.entryList(QStringList() << "*.pak", QDir::Files);

    int copiedFiles = 0;
    for (const QString &fileName : modFiles) {
        QString sourceFile = modsStoragePath + "/" + fileName;
        QString destFile = backupDir + "/" + fileName;

        if (QFile::copy(sourceFile, destFile)) {
            copiedFiles++;
        }
    }

    QJsonObject backupInfo;
    backupInfo["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    int modCount = m_modManager->getMods().count();
    backupInfo["modCount"] = modCount;
    backupInfo["fileCount"] = copiedFiles;

    QFile infoFile(backupDir + "/backup_info.json");
    if (infoFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(backupInfo);
        infoFile.write(doc.toJson());
        infoFile.close();
    }

    emit backupCreated(modCount);
    MessageModal::information(m_modalManager, tr("Success"),
        tr("Backup created successfully:\n%1\n\nBacked up %2 mod file(s)")
        .arg(backupDir).arg(copiedFiles));
}

void BackupWidget::onRestoreBackupClicked() {
    if (!m_modManager) {
        emit errorOccurred(tr("Mod manager not initialized"));
        return;
    }

    if (!m_modalManager) {
        return;
    }

    QString backupsPath = getBackupsPath();
    QDir backupsDir(backupsPath);

    if (!backupsDir.exists()) {
        emit errorOccurred(tr("No backups found"));
        return;
    }

    QStringList backups = backupsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);

    if (backups.isEmpty()) {
        emit errorOccurred(tr("No backups available"));
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
                displayText += tr(" (%1 mods)").arg(modCount);
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
            tr("Confirm Restore"),
            tr("This will replace your current mod configuration. Continue?"),
            MessageModal::Question,
            MessageModal::Yes | MessageModal::No
        );
        connect(confirmModal, &MessageModal::finished, this, [this, confirmModal, backupsPath, selectedBackup]() {
            if (confirmModal->clickedButton() != MessageModal::Yes) {
                return;
            }

            QString backupDir = backupsPath + "/" + selectedBackup;
            QString modsStoragePath = m_modManager->getModsStoragePath();

            QString backupMetadataPath = backupDir + "/mods.json";
            QString destMetadataPath = modsStoragePath + "/mods.json";

            QFile::remove(destMetadataPath);

            if (!QFile::copy(backupMetadataPath, destMetadataPath)) {
                emit errorOccurred(tr("Failed to restore backup metadata"));
                return;
            }

            QDir backupDirObj(backupDir);
            QStringList modFiles = backupDirObj.entryList(QStringList() << "*.pak", QDir::Files);

            int restoredFiles = 0;
            for (const QString &fileName : modFiles) {
                QString sourceFile = backupDir + "/" + fileName;
                QString destFile = modsStoragePath + "/" + fileName;

                QFile::remove(destFile);

                if (QFile::copy(sourceFile, destFile)) {
                    restoredFiles++;
                }
            }

            emit backupRestored(selectedBackup);
            MessageModal::information(m_modalManager, tr("Success"),
                tr("Backup restored successfully\n\nRestored %1 mod file(s)")
                .arg(restoredFiles));
        });
        m_modalManager->showModal(confirmModal);
    });
    m_modalManager->showModal(selectionModal);
}

QString BackupWidget::getBackupsPath() const {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/backups";
}
