#include "ProfileManagerWidget.h"
#include "../utils/ProfileManager.h"
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QFrame>

ProfileManagerWidget::ProfileManagerWidget(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(new QLabel(this))
    , m_profileList(new QListWidget(this))
    , m_createButton(new QPushButton(this))
    , m_loadButton(new QPushButton(this))
    , m_updateButton(new QPushButton(this))
    , m_exportButton(new QPushButton(this))
    , m_importButton(new QPushButton(this))
    , m_deleteButton(new QPushButton(this))
    , m_layout(new QVBoxLayout(this))
{
    setupUi();
}

void ProfileManagerWidget::setupUi() {
    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    m_titleLabel->setText("Profiles");
    m_titleLabel->setObjectName("profileTitle");

    m_profileList->setObjectName("profileList");
    m_profileList->setMinimumHeight(150);

    m_createButton->setText("Create");
    m_createButton->setObjectName("profileButton");

    m_loadButton->setText("Load");
    m_loadButton->setObjectName("profileButton");
    m_loadButton->setEnabled(false);

    m_updateButton->setText("Update");
    m_updateButton->setObjectName("profileButton");
    m_updateButton->setEnabled(false);

    m_exportButton->setText("Export");
    m_exportButton->setObjectName("profileButton");
    m_exportButton->setEnabled(false);

    m_importButton->setText("Import");
    m_importButton->setObjectName("profileButton");

    m_deleteButton->setText("Delete");
    m_deleteButton->setObjectName("profileButton");
    m_deleteButton->setEnabled(false);

    auto *buttonGrid1 = new QHBoxLayout();
    buttonGrid1->addWidget(m_createButton);
    buttonGrid1->addWidget(m_loadButton);
    buttonGrid1->addWidget(m_updateButton);

    auto *buttonGrid2 = new QHBoxLayout();
    buttonGrid2->addWidget(m_exportButton);
    buttonGrid2->addWidget(m_importButton);
    buttonGrid2->addWidget(m_deleteButton);

    frameLayout->addWidget(m_titleLabel);
    frameLayout->addWidget(m_profileList, 1);
    frameLayout->addLayout(buttonGrid1);
    frameLayout->addLayout(buttonGrid2);

    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(8);
    m_layout->addWidget(frame);

    setLayout(m_layout);

    frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_profileList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    setStyleSheet(R"(
        #profileTitle {
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            margin-bottom: 4px;
        }
        #profileList {
            background-color: #2c2c2c;
            color: #ffffff;
            border: 1px solid #404040;
            border-radius: 4px;
            padding: 4px;
        }
        #profileList::item {
            padding: 6px;
            border-radius: 4px;
        }
        #profileList::item:selected {
            background-color: #0078d4;
        }
        #profileList::item:hover {
            background-color: #3a3a3a;
        }
        #profileButton {
            background-color: #0078d4;
            color: #ffffff;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 12px;
        }
        #profileButton:hover {
            background-color: #106ebe;
        }
        #profileButton:pressed {
            background-color: #005a9e;
        }
        #profileButton:disabled {
            background-color: #404040;
            color: #808080;
        }
    )");

    setupConnections();
}

void ProfileManagerWidget::setupConnections() {
    connect(m_createButton, &QPushButton::clicked, this, &ProfileManagerWidget::onCreateClicked);
    connect(m_loadButton, &QPushButton::clicked, this, &ProfileManagerWidget::onLoadClicked);
    connect(m_updateButton, &QPushButton::clicked, this, &ProfileManagerWidget::onUpdateClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &ProfileManagerWidget::onExportClicked);
    connect(m_importButton, &QPushButton::clicked, this, &ProfileManagerWidget::onImportClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &ProfileManagerWidget::onDeleteClicked);

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
        QListWidgetItem *item = new QListWidgetItem(profile.name);
        item->setData(Qt::UserRole, profile.id);

        if (profile.id == activeProfileId) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setForeground(QColor("#4ec9b0"));
            item->setText(profile.name + " (Active)");
        }

        m_profileList->addItem(item);
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

void ProfileManagerWidget::onExportClicked() {
    QString profileId = getSelectedProfileId();
    if (profileId.isEmpty()) {
        return;
    }

    ProfileInfo profile = m_profileManager->getProfile(profileId);
    QString defaultFileName = profile.name + ".json";

    QString filePath = QFileDialog::getSaveFileName(this, "Export Profile",
        defaultFileName, "JSON Files (*.json)");

    if (!filePath.isEmpty()) {
        if (m_profileManager->exportProfile(profileId, filePath)) {
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

void ProfileManagerWidget::onDeleteClicked() {
    QString profileId = getSelectedProfileId();
    if (profileId.isEmpty()) {
        return;
    }

    ProfileInfo profile = m_profileManager->getProfile(profileId);

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Delete Profile",
        "Are you sure you want to delete profile '" + profile.name + "'?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_profileManager->deleteProfile(profileId);
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
    m_exportButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
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
