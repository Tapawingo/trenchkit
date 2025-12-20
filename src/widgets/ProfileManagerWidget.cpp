#include "ProfileManagerWidget.h"
#include "ProfileRowWidget.h"
#include "GradientFrame.h"
#include "../utils/ProfileManager.h"
#include "../utils/Theme.h"
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

ProfileManagerWidget::ProfileManagerWidget(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(new QLabel(this))
    , m_profileList(new QListWidget(this))
    , m_createButton(new QPushButton(this))
    , m_loadButton(new QPushButton(this))
    , m_updateButton(new QPushButton(this))
    , m_importIconButton(new QToolButton(this))
    , m_layout(new QVBoxLayout(this))
{
    setupUi();
}

void ProfileManagerWidget::setupUi() {
    GradientFrame *frame = new GradientFrame(this);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLayout = new QHBoxLayout();
    m_titleLabel->setText("Profiles");
    m_titleLabel->setObjectName("profileTitle");

    m_importIconButton->setIcon(QIcon(":/icon_import.png"));
    m_importIconButton->setIconSize(QSize(20, 20));
    m_importIconButton->setFixedSize(28, 28);
    m_importIconButton->setToolTip("Import Profile");
    m_importIconButton->setObjectName("importIconButton");

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_importIconButton);

    m_profileList->setObjectName("profileList");
    m_profileList->setMinimumHeight(150);
    m_profileList->setSpacing(Theme::Spacing::PROFILE_LIST_ITEM_SPACING);

    m_createButton->setText("New Profile");
    m_createButton->setObjectName("profileButton");

    m_loadButton->setText("Select Profile");
    m_loadButton->setObjectName("profileButton");
    m_loadButton->setEnabled(false);

    m_updateButton->setText("Update Selected");
    m_updateButton->setObjectName("profileButton");
    m_updateButton->setEnabled(false);


    frameLayout->addLayout(titleLayout);
    frameLayout->addWidget(m_profileList, 1);
    frameLayout->addWidget(m_createButton);
    frameLayout->addWidget(m_loadButton);
    frameLayout->addWidget(m_updateButton);

    m_layout->setContentsMargins(
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN
    );
    m_layout->setSpacing(Theme::Spacing::CONTAINER_SPACING);
    m_layout->addWidget(frame);

    setLayout(m_layout);

    frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_profileList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    setupConnections();
}

void ProfileManagerWidget::setupConnections() {
    connect(m_createButton, &QPushButton::clicked, this, &ProfileManagerWidget::onCreateClicked);
    connect(m_loadButton, &QPushButton::clicked, this, &ProfileManagerWidget::onLoadClicked);
    connect(m_updateButton, &QPushButton::clicked, this, &ProfileManagerWidget::onUpdateClicked);
    connect(m_importIconButton, &QToolButton::clicked, this, &ProfileManagerWidget::onImportClicked);

    connect(m_profileList, &QListWidget::itemSelectionChanged,
            this, &ProfileManagerWidget::onItemSelectionChanged);
    connect(m_profileList, &QListWidget::itemDoubleClicked,
            this, &ProfileManagerWidget::onItemDoubleClicked);
}

void ProfileManagerWidget::setProfileManager(ProfileManager *profileManager) {
    m_profileManager = profileManager;

    if (m_profileManager) {
        connect(m_profileManager, &ProfileManager::profilesChanged,
                this, &ProfileManagerWidget::onProfilesChanged);
        connect(m_profileManager, &ProfileManager::activeProfileChanged,
                this, &ProfileManagerWidget::onActiveProfileChanged);

        refreshProfileList();
    }
}

void ProfileManagerWidget::refreshProfileList() {
    if (!m_profileManager) {
        return;
    }

    m_profileList->clear();

    QList<ProfileInfo> profiles = m_profileManager->getProfiles();
    QString activeProfileId = m_profileManager->getActiveProfileId();

    for (const ProfileInfo &profile : profiles) {
        auto *rowWidget = new ProfileRowWidget(profile.id, profile.name, this);

        if (profile.id == activeProfileId) {
            rowWidget->setActive(true);
        }

        connect(rowWidget, &ProfileRowWidget::exportRequested,
                this, &ProfileManagerWidget::onExportClicked);
        connect(rowWidget, &ProfileRowWidget::renameRequested,
                this, &ProfileManagerWidget::onRenameClicked);
        connect(rowWidget, &ProfileRowWidget::deleteRequested,
                this, &ProfileManagerWidget::onDeleteClicked);
        connect(rowWidget, &ProfileRowWidget::clicked,
                this, &ProfileManagerWidget::onProfileRowClicked);

        auto *item = new QListWidgetItem(m_profileList);
        item->setSizeHint(rowWidget->sizeHint());
        item->setData(Qt::UserRole, profile.id);

        m_profileList->addItem(item);
        m_profileList->setItemWidget(item, rowWidget);
    }

    updateButtonStates();
}

void ProfileManagerWidget::onCreateClicked() {
    bool ok;
    QString name = QInputDialog::getText(this, "Create Profile",
                                        "Profile name:", QLineEdit::Normal,
                                        "", &ok);

    if (ok && !name.isEmpty()) {
        if (m_profileManager->createProfile(name)) {
            QMessageBox::information(this, "Success",
                "Profile '" + name + "' created successfully!");
        }
    }
}

void ProfileManagerWidget::onLoadClicked() {
    QString profileId = getSelectedProfileId();
    if (profileId.isEmpty()) {
        return;
    }

    showValidationDialog(profileId);
}

void ProfileManagerWidget::onUpdateClicked() {
    QString profileId = getSelectedProfileId();
    if (profileId.isEmpty()) {
        return;
    }

    ProfileInfo profile = m_profileManager->getProfile(profileId);

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Update Profile",
        "Update profile '" + profile.name + "' with current mod configuration?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_profileManager->updateProfile(profileId)) {
            QMessageBox::information(this, "Success", "Profile updated successfully!");
        }
    }
}

void ProfileManagerWidget::onRenameClicked(const QString &profileId) {
    QString targetProfileId = profileId.isEmpty() ? getSelectedProfileId() : profileId;
    if (targetProfileId.isEmpty()) {
        return;
    }

    ProfileInfo profile = m_profileManager->getProfile(targetProfileId);

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Profile",
                                           "Enter new name:", QLineEdit::Normal,
                                           profile.name, &ok);

    if (ok && !newName.isEmpty() && newName != profile.name) {
        if (m_profileManager->renameProfile(targetProfileId, newName)) {
            QMessageBox::information(this, "Success",
                "Profile renamed successfully!");
        }
    }
}

void ProfileManagerWidget::onExportClicked(const QString &profileId) {
    QString targetProfileId = profileId.isEmpty() ? getSelectedProfileId() : profileId;
    if (targetProfileId.isEmpty()) {
        return;
    }

    ProfileInfo profile = m_profileManager->getProfile(targetProfileId);
    QString defaultFileName = profile.name + ".json";

    QString filePath = QFileDialog::getSaveFileName(this, "Export Profile",
        defaultFileName, "JSON Files (*.json)");

    if (!filePath.isEmpty()) {
        if (m_profileManager->exportProfile(targetProfileId, filePath)) {
            QMessageBox::information(this, "Success",
                "Profile exported to: " + filePath);
        }
    }
}

void ProfileManagerWidget::onImportClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Import Profile",
        "", "JSON Files (*.json)");

    if (!filePath.isEmpty()) {
        QString importedProfileId;
        if (m_profileManager->importProfile(filePath, importedProfileId)) {
            QMessageBox::information(this, "Success", "Profile imported successfully!");

            QMessageBox::StandardButton reply = QMessageBox::question(this,
                "Load Profile",
                "Would you like to load the imported profile now?",
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                showValidationDialog(importedProfileId);
            }
        }
    }
}

void ProfileManagerWidget::onDeleteClicked(const QString &profileId) {
    QString targetProfileId = profileId.isEmpty() ? getSelectedProfileId() : profileId;
    if (targetProfileId.isEmpty()) {
        return;
    }

    ProfileInfo profile = m_profileManager->getProfile(targetProfileId);

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Delete Profile",
        "Are you sure you want to delete profile '" + profile.name + "'?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_profileManager->deleteProfile(targetProfileId);
    }
}

void ProfileManagerWidget::onProfilesChanged() {
    refreshProfileList();
}

void ProfileManagerWidget::onActiveProfileChanged(const QString &profileId) {
    Q_UNUSED(profileId);
    refreshProfileList();
}

void ProfileManagerWidget::onItemSelectionChanged() {
    updateButtonStates();

    QString profileId = getSelectedProfileId();
    if (!profileId.isEmpty()) {
        emit profileSelected(profileId);
    }
}

void ProfileManagerWidget::onItemDoubleClicked(QListWidgetItem *item) {
    Q_UNUSED(item);
    onLoadClicked();
}

QString ProfileManagerWidget::getSelectedProfileId() const {
    QListWidgetItem *item = m_profileList->currentItem();
    if (!item) {
        return QString();
    }
    return item->data(Qt::UserRole).toString();
}

void ProfileManagerWidget::updateButtonStates() {
    bool hasSelection = !getSelectedProfileId().isEmpty();

    m_loadButton->setEnabled(hasSelection);
    m_updateButton->setEnabled(hasSelection);
}

void ProfileManagerWidget::onProfileRowClicked(const QString &profileId) {
    for (int i = 0; i < m_profileList->count(); ++i) {
        QListWidgetItem *item = m_profileList->item(i);
        if (item && item->data(Qt::UserRole).toString() == profileId) {
            m_profileList->setCurrentItem(item);

            auto *rowWidget = qobject_cast<ProfileRowWidget*>(m_profileList->itemWidget(item));
            if (rowWidget) {
                rowWidget->setSelected(true);
            }
        } else {
            auto *rowWidget = qobject_cast<ProfileRowWidget*>(m_profileList->itemWidget(item));
            if (rowWidget) {
                rowWidget->setSelected(false);
            }
        }
    }
}

void ProfileManagerWidget::showValidationDialog(const QString &profileId) {
    if (!m_profileManager) {
        return;
    }

    ProfileValidationResult validation = m_profileManager->validateProfile(profileId);
    ProfileInfo profile = m_profileManager->getProfile(profileId);

    if (validation.hasMissingMods()) {
        QString message = validation.getMessage();
        message += "\n\nMissing mods:\n";

        for (const MissingModInfo &missing : validation.missingMods) {
            message += "- Mod ID: " + missing.modId;
            if (missing.hasNexusId()) {
                message += " (NexusMods: " + missing.nexusModId + ")";
            }
            message += "\n";
        }

        message += "\nDo you want to load the profile anyway with available mods only?";

        QMessageBox::StandardButton reply = QMessageBox::warning(this,
            "Missing Mods", message,
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            if (m_profileManager->applyProfile(profileId, true)) {
                emit profileLoadRequested(profileId);
            }
        }
    } else {
        if (m_profileManager->applyProfile(profileId, false)) {
            emit profileLoadRequested(profileId);
            QMessageBox::information(this, "Success",
                "Profile '" + profile.name + "' loaded successfully!");
        }
    }
}
