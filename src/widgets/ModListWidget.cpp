#include "ModListWidget.h"
#include "ModRowWidget.h"
#include "GradientFrame.h"
#include "../modals/ModalManager.h"
#include "../modals/MessageModal.h"
#include "../modals/InputModal.h"
#include "../modals/content/AddModModalContent.h"
#include "../modals/content/ModMetadataModalContent.h"
#include "../modals/content/ModUpdateModalContent.h"
#include "../modals/content/ItchModUpdateModalContent.h"
#include "../modals/content/FileSelectionModalContent.h"
#include "../utils/Theme.h"
#include "../utils/ArchiveExtractor.h"
#include "../utils/ModUpdateService.h"
#include "../utils/ModUpdateInfo.h"
#include "../utils/ItchModUpdateService.h"
#include "../utils/ItchUpdateInfo.h"
#include "../utils/ModConflictDetector.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QLabel>
#include <QTimer>
#include <QFileInfo>
#include <QPushButton>

ModListWidget::ModListWidget(QWidget *parent)
    : QWidget(parent)
    , m_loadingLabel(new QLabel(this))
    , m_modCountLabel(new QLabel(this))
    , m_loadingTimer(new QTimer(this))
    , m_conflictDetector(nullptr)
{
    setupUi();

    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setObjectName("modLoadingLabel");
    m_loadingLabel->hide();

    connect(m_loadingTimer, &QTimer::timeout, this, &ModListWidget::updateLoadingAnimation);
}

void ModListWidget::setModManager(ModManager *modManager) {
    if (m_modManager) {
        disconnect(m_modManager, nullptr, this, nullptr);
    }

    m_modManager = modManager;

    if (m_modManager) {
        connect(m_modManager, &ModManager::modsChanged,
                this, &ModListWidget::onModsChanged);
        connect(m_modManager, &ModManager::errorOccurred,
                this, [this](const QString &error) {
            if (m_modalManager) {
                MessageModal::warning(m_modalManager, "Error", error);
            }
        });

        if (!m_conflictDetector) {
            m_conflictDetector = new ModConflictDetector(m_modManager, this);
            connect(m_conflictDetector, &ModConflictDetector::scanComplete,
                    this, &ModListWidget::onConflictScanComplete);
        }

        connect(m_modManager, &ModManager::modsChanged,
                m_conflictDetector, &ModConflictDetector::scanForConflicts);

        refreshModList();
        m_conflictDetector->scanForConflicts();
    }
}

void ModListWidget::setupUi() {
    GradientFrame *frame = new GradientFrame(this);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(Theme::Spacing::MOD_LIST_TITLE_SPACING);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    auto *titleLabel = new QLabel("Installed Mods:", this);
    titleLabel->setObjectName("modListTitle");

    m_modCountLabel->setObjectName("modCountLabel");
    m_modCountLabel->setText("0");

    m_checkUpdatesButton = new QPushButton("Check for Updates", this);
    m_checkUpdatesButton->setObjectName("checkUpdatesButton");
    m_checkUpdatesButton->setCursor(Qt::PointingHandCursor);

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(m_modCountLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_checkUpdatesButton);

    connect(m_checkUpdatesButton, &QPushButton::clicked,
            this, &ModListWidget::onCheckUpdatesClicked);

    m_modList = new DraggableModList(this);
    m_modList->setSpacing(Theme::Spacing::MOD_LIST_ITEM_SPACING);
    m_modList->setUniformItemSizes(false);

    frameLayout->addLayout(titleLayout);
    frameLayout->addWidget(m_modList, 1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(frame);

    connect(m_modList, &DraggableModList::itemsReordered,
            this, &ModListWidget::onItemsReordered);

    connect(m_modList, &DraggableModList::filesDropped,
            this, &ModListWidget::onFilesDropped);

    connect(m_modList, &QListWidget::itemSelectionChanged,
            this, [this]() {
        int selectedRow = getSelectedRow();
        int totalMods = m_modList->count();

        for (int i = 0; i < totalMods; ++i) {
            QListWidgetItem *item = m_modList->item(i);
            if (item) {
                auto *rowWidget = qobject_cast<ModRowWidget*>(m_modList->itemWidget(item));
                if (rowWidget) {
                    rowWidget->setSelected(i == selectedRow);
                }
            }
        }

        emit modSelectionChanged(selectedRow, totalMods);
    });
}

void ModListWidget::refreshModList() {
    if (!m_modManager) {
        return;
    }

    m_updating = true;

    int currentRow = getSelectedRow();
    m_modList->clear();

    QList<ModInfo> mods = m_modManager->getMods();
    m_modCountLabel->setText(QString::number(mods.size()) + " Total");
    for (int i = 0; i < mods.size(); ++i) {
        const ModInfo &mod = mods[i];
        auto *modRow = new ModRowWidget(mod, this);
        connect(modRow, &ModRowWidget::enabledChanged,
                this, &ModListWidget::onModEnabledChanged);
        connect(modRow, &ModRowWidget::renameRequested,
                this, &ModListWidget::onRenameRequested);
        connect(modRow, &ModRowWidget::editMetaRequested,
                this, &ModListWidget::onEditMetaRequested);
        connect(modRow, &ModRowWidget::removeRequested,
                this, &ModListWidget::onRemoveRequested);
        connect(modRow, &ModRowWidget::updateRequested,
                this, &ModListWidget::onUpdateRequested);

        auto *item = new QListWidgetItem(m_modList);
        item->setSizeHint(modRow->sizeHint());
        item->setData(Qt::UserRole, mod.id);

        m_modList->addItem(item);
        m_modList->setItemWidget(item, modRow);

        if (m_updateService && m_updateService->hasUpdate(mod.id)) {
            ModUpdateInfo updateInfo = m_updateService->getUpdateInfo(mod.id);
            modRow->setUpdateAvailable(true, updateInfo.availableVersion);
        } else if (m_itchUpdateService && m_itchUpdateService->hasUpdate(mod.id)) {
            ItchUpdateInfo updateInfo = m_itchUpdateService->getUpdateInfo(mod.id);
            modRow->setUpdateAvailable(true, updateInfo.availableVersion);
        }

        if (m_conflictDetector) {
            ConflictInfo conflictInfo = m_conflictDetector->getConflictInfo(mod.id);
            modRow->setConflictInfo(conflictInfo);
        }

        modRow->setSelected(i == currentRow);
    }

    if (currentRow >= 0 && currentRow < m_modList->count()) {
        m_modList->setCurrentRow(currentRow);
    }

    m_updating = false;
}

void ModListWidget::onModsChanged() {
    refreshModList();
}

void ModListWidget::onModEnabledChanged(const QString &modId, bool enabled) {
    if (m_updating || !m_modManager) {
        return;
    }

    if (enabled) {
        m_modManager->enableMod(modId);
    } else {
        m_modManager->disableMod(modId);
    }
}

void ModListWidget::onAddModClicked() {
    if (!m_modManager || !m_modalManager) {
        return;
    }

    auto *modal = new AddModModalContent(m_modManager, m_nexusClient, m_nexusAuth, m_itchClient, m_itchAuth, m_modalManager);
    connect(modal, &AddModModalContent::modAdded, this, &ModListWidget::modAdded);
    m_modalManager->showModal(modal);
}

void ModListWidget::onRemoveModClicked() {
    if (!m_modManager || !m_modalManager) {
        return;
    }

    QString modId = getSelectedModId();
    if (modId.isEmpty()) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    auto *modal = new MessageModal(
        "Remove Mod",
        QString("Are you sure you want to remove '%1'?").arg(mod.name),
        MessageModal::Question,
        MessageModal::Yes | MessageModal::No
    );
    connect(modal, &MessageModal::finished, this, [this, modal, modId, mod]() {
        if (modal->clickedButton() == MessageModal::Yes) {
            QString modName = mod.name;
            if (!m_modManager->removeMod(modId)) {
                MessageModal::warning(m_modalManager, "Error", "Failed to remove mod");
            } else {
                emit modRemoved(modName);
            }
        }
    });
    m_modalManager->showModal(modal);
}

void ModListWidget::onMoveUpClicked() {
    if (!m_modManager) {
        return;
    }

    int selectedRow = getSelectedRow();
    if (selectedRow <= 0) {
        return;
    }

    QString modId = getSelectedModId();
    if (modId.isEmpty()) {
        return;
    }

    // Get mod ID from row above
    QListWidgetItem *aboveItem = m_modList->item(selectedRow - 1);
    if (!aboveItem) {
        return;
    }
    QString aboveModId = aboveItem->data(Qt::UserRole).toString();

    // Swap priorities
    ModInfo currentMod = m_modManager->getMod(modId);
    ModInfo aboveMod = m_modManager->getMod(aboveModId);

    m_modManager->setModPriority(modId, aboveMod.priority);
    m_modManager->setModPriority(aboveModId, currentMod.priority);

    // Select the moved row
    m_modList->setCurrentRow(selectedRow - 1);

    emit modReordered();
}

void ModListWidget::onMoveDownClicked() {
    if (!m_modManager) {
        return;
    }

    int selectedRow = getSelectedRow();
    if (selectedRow < 0 || selectedRow >= m_modList->count() - 1) {
        return;
    }

    QString modId = getSelectedModId();
    if (modId.isEmpty()) {
        return;
    }

    // Get mod ID from row below
    QListWidgetItem *belowItem = m_modList->item(selectedRow + 1);
    if (!belowItem) {
        return;
    }
    QString belowModId = belowItem->data(Qt::UserRole).toString();

    // Swap priorities
    ModInfo currentMod = m_modManager->getMod(modId);
    ModInfo belowMod = m_modManager->getMod(belowModId);

    m_modManager->setModPriority(modId, belowMod.priority);
    m_modManager->setModPriority(belowModId, currentMod.priority);

    // Select the moved row
    m_modList->setCurrentRow(selectedRow + 1);

    emit modReordered();
}

QString ModListWidget::getSelectedModId() const {
    QListWidgetItem *item = m_modList->currentItem();
    if (!item) {
        return QString();
    }

    return item->data(Qt::UserRole).toString();
}

int ModListWidget::getSelectedRow() const {
    return m_modList->currentRow();
}

void ModListWidget::setLoadingState(bool loading, const QString &message) {
    if (loading) {
        m_loadingDots = 0;
        m_loadingLabel->setText(message + ".");

        QRect listGeometry = m_modList->geometry();
        int labelWidth = 200;
        int labelHeight = 30;
        int x = listGeometry.x() + (listGeometry.width() - labelWidth) / 2;
        int y = listGeometry.y() + (listGeometry.height() - labelHeight) / 2;
        m_loadingLabel->setGeometry(x, y, labelWidth, labelHeight);

        m_loadingLabel->show();
        m_loadingLabel->raise();

        m_loadingTimer->start(500);
    } else {
        m_loadingTimer->stop();
        m_loadingLabel->hide();
    }
}

void ModListWidget::updateLoadingAnimation() {
    m_loadingDots = (m_loadingDots + 1) % 4;
    QString baseText = m_loadingLabel->text();
    int dotIndex = baseText.indexOf('.');
    if (dotIndex != -1) {
        baseText = baseText.left(dotIndex);
    }

    QString dots(m_loadingDots, '.');
    m_loadingLabel->setText(baseText + dots);
}

void ModListWidget::onItemsReordered() {
    if (!m_modManager) {
        return;
    }

    m_updating = true;

    QMap<QString, int> newPriorities;
    for (int row = 0; row < m_modList->count(); ++row) {
        QListWidgetItem *item = m_modList->item(row);
        if (item) {
            QString modId = item->data(Qt::UserRole).toString();
            newPriorities[modId] = row;
        }
    }

    m_modManager->batchSetModPriorities(newPriorities);

    m_updating = false;
}

void ModListWidget::onRenameRequested(const QString &modId) {
    if (!m_modManager) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    if (mod.id.isEmpty()) {
        return;
    }

    auto *modal = new InputModal("Rename Mod", "Enter new name:", mod.name);
    connect(modal, &InputModal::accepted, this, [this, modal, modId, mod]() {
        QString newName = modal->textValue();
        if (!newName.isEmpty() && newName != mod.name) {
            ModInfo updatedMod = mod;
            updatedMod.name = newName;
            m_modManager->updateModMetadata(updatedMod);
        }
    });
    m_modalManager->showModal(modal);
}

void ModListWidget::onEditMetaRequested(const QString &modId) {
    if (!m_modManager) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    if (mod.id.isEmpty()) {
        return;
    }

    auto *modal = new ModMetadataModalContent(mod);
    connect(modal, &ModMetadataModalContent::accepted, this, [this, modal]() {
        ModInfo updatedMod = modal->getModInfo();
        m_modManager->updateModMetadata(updatedMod);
    });
    m_modalManager->showModal(modal);
}

void ModListWidget::onRemoveRequested(const QString &modId) {
    if (!m_modManager || !m_modalManager) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    auto *modal = new MessageModal(
        "Remove Mod",
        QString("Are you sure you want to remove '%1'?").arg(mod.name),
        MessageModal::Question,
        MessageModal::Yes | MessageModal::No
    );
    connect(modal, &MessageModal::finished, this, [this, modal, modId, mod]() {
        if (modal->clickedButton() == MessageModal::Yes) {
            QString modName = mod.name;
            if (!m_modManager->removeMod(modId)) {
                MessageModal::warning(m_modalManager, "Error", "Failed to remove mod");
            } else {
                emit modRemoved(modName);
            }
        }
    });
    m_modalManager->showModal(modal);
}

void ModListWidget::setNexusServices(NexusModsClient *client, NexusModsAuth *auth) {
    m_nexusClient = client;
    m_nexusAuth = auth;
}

void ModListWidget::setItchServices(ItchClient *client, ItchAuth *auth) {
    m_itchClient = client;
    m_itchAuth = auth;
}

void ModListWidget::setUpdateService(ModUpdateService *service) {
    if (m_updateService) {
        disconnect(m_updateService, nullptr, this, nullptr);
    }

    m_updateService = service;

    if (m_updateService) {
        connect(m_updateService, &ModUpdateService::updateFound,
                this, &ModListWidget::onUpdateFound);
        connect(m_updateService, &ModUpdateService::checkComplete,
                this, &ModListWidget::onUpdateCheckComplete);
    }
}

void ModListWidget::setItchUpdateService(ItchModUpdateService *service) {
    if (m_itchUpdateService) {
        disconnect(m_itchUpdateService, nullptr, this, nullptr);
    }

    m_itchUpdateService = service;

    if (m_itchUpdateService) {
        connect(m_itchUpdateService, &ItchModUpdateService::updateFound,
                this, &ModListWidget::onItchUpdateFound);
        connect(m_itchUpdateService, &ItchModUpdateService::checkComplete,
                this, &ModListWidget::onItchUpdateCheckComplete);
    }
}

void ModListWidget::onCheckUpdatesClicked() {
    m_checkUpdatesButton->setEnabled(false);
    m_checkUpdatesButton->setText("Checking...");

    if (m_updateService) {
        m_updateService->checkAllModsForUpdates();
    }

    if (m_itchUpdateService) {
        m_itchUpdateService->checkAllModsForUpdates();
    }
}

void ModListWidget::onUpdateFound(const QString &modId, const ModUpdateInfo &updateInfo) {
    for (int i = 0; i < m_modList->count(); ++i) {
        QListWidgetItem *item = m_modList->item(i);
        if (item && item->data(Qt::UserRole).toString() == modId) {
            auto *rowWidget = qobject_cast<ModRowWidget*>(m_modList->itemWidget(item));
            if (rowWidget) {
                rowWidget->setUpdateAvailable(true, updateInfo.availableVersion);
            }
            break;
        }
    }
}

void ModListWidget::onUpdateCheckComplete(int updatesFound) {
    m_checkUpdatesButton->setEnabled(true);
    m_checkUpdatesButton->setText("Check for Updates");
}

void ModListWidget::onItchUpdateFound(const QString &modId, const ItchUpdateInfo &updateInfo) {
    for (int i = 0; i < m_modList->count(); ++i) {
        QListWidgetItem *item = m_modList->item(i);
        if (item && item->data(Qt::UserRole).toString() == modId) {
            auto *rowWidget = qobject_cast<ModRowWidget*>(m_modList->itemWidget(item));
            if (rowWidget) {
                rowWidget->setUpdateAvailable(true, updateInfo.availableVersion);
            }
            break;
        }
    }
}

void ModListWidget::onItchUpdateCheckComplete(int updatesFound) {
    m_checkUpdatesButton->setEnabled(true);
    m_checkUpdatesButton->setText("Check for Updates");
}

void ModListWidget::onUpdateRequested(const QString &modId) {
    if (!m_modManager) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    if (mod.id.isEmpty()) {
        return;
    }

    auto hideUpdateButton = [this, modId]() {
        for (int i = 0; i < m_modList->count(); ++i) {
            QListWidgetItem *item = m_modList->item(i);
            if (item && item->data(Qt::UserRole).toString() == modId) {
                auto *rowWidget = qobject_cast<ModRowWidget*>(m_modList->itemWidget(item));
                if (rowWidget) {
                    rowWidget->hideUpdateButton();
                }
                break;
            }
        }
    };

    if (!mod.nexusModId.isEmpty() && m_nexusClient && m_updateService && m_updateService->hasUpdate(modId)) {
        ModUpdateInfo updateInfo = m_updateService->getUpdateInfo(modId);
        auto *modal = new ModUpdateModalContent(mod, updateInfo, m_modManager, m_nexusClient, m_modalManager);
        connect(modal, &ModUpdateModalContent::accepted, this, hideUpdateButton);
        m_modalManager->showModal(modal);
    } else if (!mod.itchGameId.isEmpty() && m_itchClient && m_itchUpdateService && m_itchUpdateService->hasUpdate(modId)) {
        ItchUpdateInfo updateInfo = m_itchUpdateService->getUpdateInfo(modId);

        // Check if any upload matches the current filename
        ItchUploadInfo matchingUpload;
        bool hasMatchingFilename = false;

        if (!mod.fileName.isEmpty()) {
            for (const ItchUploadInfo &upload : updateInfo.candidateUploads) {
                if (upload.filename == mod.fileName) {
                    matchingUpload = upload;
                    hasMatchingFilename = true;
                    break;
                }
            }
        }

        // If we found a matching filename, auto-select it and skip the file selection modal
        if (hasMatchingFilename) {
            ItchUpdateInfo selectedUpdateInfo = updateInfo;
            selectedUpdateInfo.availableUploadId = matchingUpload.id;
            selectedUpdateInfo.availableUploadDate =
                matchingUpload.updatedAt.isValid() ? matchingUpload.updatedAt : matchingUpload.createdAt;
            selectedUpdateInfo.availableVersion = extractVersionFromFilename(matchingUpload.filename);
            if (selectedUpdateInfo.availableVersion.isEmpty()) {
                selectedUpdateInfo.availableVersion = QString("Updated: %1")
                    .arg(selectedUpdateInfo.availableUploadDate.toString("yyyy-MM-dd"));
            }

            auto *modal = new ItchModUpdateModalContent(
                mod, selectedUpdateInfo, m_modManager, m_itchClient, m_modalManager);
            connect(modal, &ItchModUpdateModalContent::accepted, this, hideUpdateButton);
            m_modalManager->showModal(modal);
        }
        // If multiple uploads and no matching filename, show file selection modal
        else if (updateInfo.hasMultipleUploads()) {
            QList<FileItem> fileItems;
            for (const ItchUploadInfo &upload : updateInfo.candidateUploads) {
                fileItems.append({upload.id, upload.filename});
            }

            auto *fileModal = new FileSelectionModalContent(
                fileItems,
                "Select Update File",
                QString("Multiple update files are available for '%1'. Select one:").arg(mod.name),
                false,
                true  // Show "Ignore These Updates" button
            );

            // If user clicks "Ignore These Updates", mark all these uploads as ignored and hide the update button
            connect(fileModal, &FileSelectionModalContent::allIgnored, this,
                    [this, modId, updateInfo, hideUpdateButton]() {
                if (m_itchUpdateService) {
                    QStringList uploadIds;
                    for (const ItchUploadInfo &upload : updateInfo.candidateUploads) {
                        uploadIds.append(upload.id);
                    }
                    m_itchUpdateService->ignoreUpdatesForMod(modId, uploadIds);
                }
                hideUpdateButton();
            });

            connect(fileModal, &FileSelectionModalContent::accepted, this,
                    [this, fileModal, mod, updateInfo, hideUpdateButton]() {
                QStringList selectedIds = fileModal->getSelectedIds();
                if (!selectedIds.isEmpty()) {
                    QString selectedUploadId = selectedIds.first();

                    ItchUploadInfo selectedUpload;
                    for (const ItchUploadInfo &upload : updateInfo.candidateUploads) {
                        if (upload.id == selectedUploadId) {
                            selectedUpload = upload;
                            break;
                        }
                    }

                    ItchUpdateInfo selectedUpdateInfo = updateInfo;
                    selectedUpdateInfo.availableUploadId = selectedUpload.id;
                    selectedUpdateInfo.availableUploadDate =
                        selectedUpload.updatedAt.isValid() ? selectedUpload.updatedAt : selectedUpload.createdAt;
                    selectedUpdateInfo.availableVersion = extractVersionFromFilename(selectedUpload.filename);
                    if (selectedUpdateInfo.availableVersion.isEmpty()) {
                        selectedUpdateInfo.availableVersion = QString("Updated: %1")
                            .arg(selectedUpdateInfo.availableUploadDate.toString("yyyy-MM-dd"));
                    }

                    auto *downloadModal = new ItchModUpdateModalContent(
                        mod, selectedUpdateInfo, m_modManager, m_itchClient, m_modalManager);
                    connect(downloadModal, &ItchModUpdateModalContent::accepted,
                            this, hideUpdateButton);
                    m_modalManager->showModal(downloadModal);
                }
            });

            m_modalManager->showModal(fileModal);
        }
        // Single upload, proceed directly
        else {
            auto *modal = new ItchModUpdateModalContent(mod, updateInfo, m_modManager, m_itchClient, m_modalManager);
            connect(modal, &ItchModUpdateModalContent::accepted, this, hideUpdateButton);
            m_modalManager->showModal(modal);
        }
    }
}

QString ModListWidget::extractVersionFromFilename(const QString &filename) const {
    static const QRegularExpression versionRegex(R"(v?(\d+\.\d+(?:\.\d+)?))");
    QRegularExpressionMatch match = versionRegex.match(filename);

    if (match.hasMatch()) {
        return match.captured(1);
    }

    return QString();
}

void ModListWidget::onFilesDropped(const QStringList &filePaths) {
    if (!m_modManager || !m_modalManager) {
        return;
    }

    for (const QString &filePath : filePaths) {
        if (filePath.endsWith(".pak", Qt::CaseInsensitive)) {
            handlePakFile(filePath);
        } else if (isArchiveFile(filePath)) {
            handleArchiveFile(filePath);
        }
    }
}

void ModListWidget::handlePakFile(const QString &pakPath) {
    QFileInfo fileInfo(pakPath);
    QString modName = fileInfo.baseName();

    if (!m_modManager->addMod(pakPath, modName)) {
        MessageModal::warning(m_modalManager, "Error",
                            "Failed to add mod: " + modName);
    } else {
        emit modAdded(modName);
    }
}

bool ModListWidget::isArchiveFile(const QString &filePath) const {
    QString lower = filePath.toLower();
    return lower.endsWith(".zip") ||
           lower.endsWith(".rar") ||
           lower.endsWith(".7z") ||
           lower.endsWith(".tar.gz") ||
           lower.endsWith(".tar.bz2") ||
           lower.endsWith(".tar.xz");
}

void ModListWidget::handleArchiveFile(const QString &archivePath) {
    ArchiveExtractor extractor;
    auto result = extractor.extractPakFiles(archivePath);

    if (!result.success) {
        MessageModal::warning(m_modalManager, "Error", result.error);
        return;
    }

    if (result.pakFiles.isEmpty()) {
        MessageModal::warning(m_modalManager, "Error",
                            "No .pak files found in archive");
        ArchiveExtractor::cleanupTempDir(result.tempDir);
        return;
    }

    if (result.pakFiles.size() == 1) {
        handlePakFile(result.pakFiles.first());
        ArchiveExtractor::cleanupTempDir(result.tempDir);
    } else {
        QStringList fileNames;
        for (const QString &path : result.pakFiles) {
            fileNames.append(QFileInfo(path).fileName());
        }

        auto *fileModal = new FileSelectionModalContent(
            fileNames, QFileInfo(archivePath).fileName(), true);

        connect(fileModal, &FileSelectionModalContent::accepted,
                this, [this, result, fileModal]() {
            QStringList selectedFileNames = fileModal->getSelectedFiles();

            for (const QString &fileName : selectedFileNames) {
                for (const QString &path : result.pakFiles) {
                    if (QFileInfo(path).fileName() == fileName) {
                        handlePakFile(path);
                        break;
                    }
                }
            }

            ArchiveExtractor::cleanupTempDir(result.tempDir);
        });

        connect(fileModal, &FileSelectionModalContent::rejected,
                this, [result]() {
            ArchiveExtractor::cleanupTempDir(result.tempDir);
        });

        m_modalManager->showModal(fileModal);
    }
}

void ModListWidget::onConflictScanComplete(QMap<QString, ConflictInfo> conflicts) {
    for (int i = 0; i < m_modList->count(); ++i) {
        QListWidgetItem *item = m_modList->item(i);
        if (!item) {
            continue;
        }

        auto *rowWidget = qobject_cast<ModRowWidget*>(m_modList->itemWidget(item));
        if (rowWidget) {
            QString modId = rowWidget->modId();
            ConflictInfo info = conflicts.value(modId);
            rowWidget->setConflictInfo(info);
        }
    }
}
