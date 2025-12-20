#include "ModListWidget.h"
#include "ModRowWidget.h"
#include "ModMetadataDialog.h"
#include "GradientFrame.h"
#include "../utils/Theme.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QLabel>
#include <QTimer>

ModListWidget::ModListWidget(QWidget *parent)
    : QWidget(parent)
    , m_loadingLabel(new QLabel(this))
    , m_loadingTimer(new QTimer(this))
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
                this, [](const QString &error) {
            QMessageBox::warning(nullptr, "Error", error);
        });

        refreshModList();
    }
}

void ModListWidget::setupUi() {
    GradientFrame *frame = new GradientFrame(this);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLabel = new QLabel("Installed Mods:", this);
    titleLabel->setObjectName("modListTitle");

    m_modList = new DraggableModList(this);
    m_modList->setSpacing(Theme::Spacing::MOD_LIST_ITEM_SPACING);
    m_modList->setUniformItemSizes(false);

    frameLayout->addWidget(titleLabel);
    frameLayout->addWidget(m_modList, 1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(frame);

    connect(m_modList, &DraggableModList::itemsReordered,
            this, &ModListWidget::onItemsReordered);

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

        auto *item = new QListWidgetItem(m_modList);
        item->setSizeHint(modRow->sizeHint());
        item->setData(Qt::UserRole, mod.id);

        m_modList->addItem(item);
        m_modList->setItemWidget(item, modRow);

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
    if (!m_modManager) {
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Mod File",
        QString(),
        "Pak Files (*.pak);;All Files (*.*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(filePath);
    QString modName = fileInfo.baseName();

    if (!m_modManager->addMod(filePath, modName)) {
        QMessageBox::warning(this, "Error", "Failed to add mod");
    }
}

void ModListWidget::onRemoveModClicked() {
    if (!m_modManager) {
        return;
    }

    QString modId = getSelectedModId();
    if (modId.isEmpty()) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    auto reply = QMessageBox::question(
        this,
        "Remove Mod",
        QString("Are you sure you want to remove '%1'?").arg(mod.name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (!m_modManager->removeMod(modId)) {
            QMessageBox::warning(this, "Error", "Failed to remove mod");
        }
    }
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

    bool ok;
    QString newName = QInputDialog::getText(
        this,
        "Rename Mod",
        "Enter new name:",
        QLineEdit::Normal,
        mod.name,
        &ok
    );

    if (ok && !newName.isEmpty() && newName != mod.name) {
        mod.name = newName;
        m_modManager->updateModMetadata(mod);
    }
}

void ModListWidget::onEditMetaRequested(const QString &modId) {
    if (!m_modManager) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    if (mod.id.isEmpty()) {
        return;
    }

    ModMetadataDialog dialog(mod, this);
    if (dialog.exec() == QDialog::Accepted) {
        ModInfo updatedMod = dialog.getModInfo();
        m_modManager->updateModMetadata(updatedMod);
    }
}

void ModListWidget::onRemoveRequested(const QString &modId) {
    if (!m_modManager) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    auto reply = QMessageBox::question(
        this,
        "Remove Mod",
        QString("Are you sure you want to remove '%1'?").arg(mod.name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (!m_modManager->removeMod(modId)) {
            QMessageBox::warning(this, "Error", "Failed to remove mod");
        }
    }
}
