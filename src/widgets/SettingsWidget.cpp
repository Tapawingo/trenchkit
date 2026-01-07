#include "SettingsWidget.h"

#include "../modals/ModalManager.h"
#include "../modals/MessageModal.h"
#include "../modals/InputModal.h"
#include "../utils/UpdaterService.h"
#include "../utils/NexusModsClient.h"
#include "../utils/NexusModsAuth.h"
#include "../utils/Theme.h"
#include "PanelFrame.h"
#include "GradientFrame.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QInputDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QUrl>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>

SettingsWidget::SettingsWidget(QWidget *parent, UpdaterService *updater)
    : QWidget(parent),
      m_updater(updater) {
    buildUi();
    loadSettings(false);
}

bool SettingsWidget::applyStoredSettings() {
    loadSettings(true);
    return m_autoCheckCheckbox ? m_autoCheckCheckbox->isChecked() : false;
}

void SettingsWidget::loadSettingsIntoUi() {
    loadSettings(false);
}

void SettingsWidget::setCurrentVersion(const QString &version) {
    if (m_versionLabel) {
        m_versionLabel->setText(version.isEmpty() ? QStringLiteral("—") : version);
    }
}

void SettingsWidget::setCheckStatus(const QString &status) {
    if (m_checkStatusLabel) {
        m_checkStatusLabel->setText(status.isEmpty() ? QStringLiteral("Idle") : status);
    }
}

QString SettingsWidget::resolvedDownloadDir() const {
    QString dir;
    if (m_downloadDirEdit) {
        dir = m_downloadDirEdit->text().trimmed();
    }
    if (dir.isEmpty()) {
        const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        return QDir(base).filePath("updates");
    }
    return dir;
}

void SettingsWidget::buildUi() {
    auto *overlayLayout = new QVBoxLayout(this);
    overlayLayout->setContentsMargins(
        Theme::Spacing::SETTINGS_OUTER_MARGIN,
        Theme::Spacing::SETTINGS_OUTER_MARGIN,
        Theme::Spacing::SETTINGS_OUTER_MARGIN,
        Theme::Spacing::SETTINGS_OUTER_MARGIN);

    m_panel = new PanelFrame(this);
    m_panel->setObjectName("settingsPanel");
    m_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *panelLayout = new QVBoxLayout(m_panel);
    panelLayout->setContentsMargins(
        Theme::Spacing::SETTINGS_PANEL_MARGIN,
        Theme::Spacing::SETTINGS_PANEL_MARGIN,
        Theme::Spacing::SETTINGS_PANEL_MARGIN,
        Theme::Spacing::SETTINGS_PANEL_MARGIN);
    panelLayout->setSpacing(Theme::Spacing::SETTINGS_SECTION_SPACING);

    auto *scrollArea = new QScrollArea(m_panel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_container = new GradientFrame(scrollArea);
    m_container->setObjectName("settingsContainer");
    m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *containerLayout = new QGridLayout(m_container);
    containerLayout->setContentsMargins(
        Theme::Spacing::SETTINGS_CONTAINER_MARGIN,
        Theme::Spacing::SETTINGS_CONTAINER_MARGIN,
        Theme::Spacing::SETTINGS_CONTAINER_MARGIN,
        Theme::Spacing::SETTINGS_CONTAINER_MARGIN);
    containerLayout->setHorizontalSpacing(Theme::Spacing::SETTINGS_ROW_SPACING);
    containerLayout->setVerticalSpacing(Theme::Spacing::SETTINGS_ROW_SPACING);
    containerLayout->setColumnStretch(0, 1);
    containerLayout->setColumnStretch(1, 2);
    containerLayout->setRowStretch(12, 1);

    auto *title = new QLabel("Settings", m_panel);
    title->setObjectName("settingsTitle");
    title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(title, 0, 0, 1, 2);

    auto *updaterHeader = new QLabel("Updater", m_panel);
    updaterHeader->setObjectName("settingsSectionHeader");
    updaterHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(updaterHeader, 1, 0, 1, 2);

    auto *versionLabel = new QLabel("Current version", m_panel);
    versionLabel->setObjectName("settingsLabel");
    versionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(versionLabel, 2, 0);

    m_versionLabel = new QLabel("—", m_panel);
    m_versionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(m_versionLabel, 2, 1);

    auto *sourceLabel = new QLabel("Source", m_panel);
    sourceLabel->setObjectName("settingsLabel");
    sourceLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(sourceLabel, 3, 0);

    m_updateSourceEdit = new QLineEdit(m_panel);
    m_updateSourceEdit->setPlaceholderText("https://github.com/Tapawingo/TrenchKit");
    m_updateSourceEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    containerLayout->addWidget(m_updateSourceEdit, 3, 1);

    auto *channelLabel = new QLabel("Update channel", m_panel);
    channelLabel->setObjectName("settingsLabel");
    channelLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(channelLabel, 4, 0);

    m_updateChannelCombo = new QComboBox(m_panel);
    m_updateChannelCombo->addItem("Stable", "stable");
    m_updateChannelCombo->addItem("Pre-release", "prerelease");
    m_updateChannelCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    containerLayout->addWidget(m_updateChannelCombo, 4, 1);

    auto *autoCheckLabel = new QLabel("Check for updates on startup", m_panel);
    autoCheckLabel->setObjectName("settingsLabel");
    autoCheckLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(autoCheckLabel, 5, 0);

    m_autoCheckCheckbox = new QCheckBox(m_panel);
    m_autoCheckCheckbox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(m_autoCheckCheckbox, 5, 1);

    auto *downloadLabel = new QLabel("Download location (optional)", m_panel);
    downloadLabel->setObjectName("settingsLabel");
    downloadLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(downloadLabel, 6, 0);

    auto *downloadRow = new QHBoxLayout();
    m_downloadDirEdit = new QLineEdit(m_panel);
    m_downloadDirEdit->setPlaceholderText("Default: AppData/Local/TrenchKit/updates");
    m_downloadDirEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_downloadBrowseButton = new QPushButton("Browse...", m_panel);
    m_downloadBrowseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_downloadBrowseButton->setCursor(Qt::PointingHandCursor);
    downloadRow->addWidget(m_downloadDirEdit);
    downloadRow->addWidget(m_downloadBrowseButton);
    containerLayout->addLayout(downloadRow, 6, 1);

    auto *checkLabel = new QLabel("Check for updates now", m_panel);
    checkLabel->setObjectName("settingsLabel");
    checkLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(checkLabel, 7, 0);

    m_checkNowButton = new QPushButton("Check now", m_panel);
    m_checkNowButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_checkNowButton->setCursor(Qt::PointingHandCursor);
    containerLayout->addWidget(m_checkNowButton, 7, 1);

    m_checkStatusLabel = new QLabel("Idle", m_panel);
    m_checkStatusLabel->setObjectName("settingsStatus");
    m_checkStatusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(m_checkStatusLabel, 8, 1);

    auto *nexusHeader = new QLabel("Nexus Mods Integration", m_panel);
    nexusHeader->setObjectName("settingsSectionHeader");
    nexusHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(nexusHeader, 9, 0, 1, 2);

    auto *nexusLabel = new QLabel("Connection status", m_panel);
    nexusLabel->setObjectName("settingsLabel");
    nexusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(nexusLabel, 10, 0);

    m_nexusStatusLabel = new QLabel("Not connected", m_panel);
    m_nexusStatusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(m_nexusStatusLabel, 10, 1);

    auto *nexusActionsLabel = new QLabel("Authentication", m_panel);
    nexusActionsLabel->setObjectName("settingsLabel");
    nexusActionsLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(nexusActionsLabel, 11, 0);

    auto *nexusButtonRow = new QHBoxLayout();
    m_nexusAuthButton = new QPushButton("Add API Key", m_panel);
    m_nexusAuthButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nexusAuthButton->setCursor(Qt::PointingHandCursor);
    m_nexusClearButton = new QPushButton("Clear API Key", m_panel);
    m_nexusClearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nexusClearButton->setCursor(Qt::PointingHandCursor);
    nexusButtonRow->addWidget(m_nexusAuthButton);
    nexusButtonRow->addWidget(m_nexusClearButton);
    nexusButtonRow->addStretch();
    containerLayout->addLayout(nexusButtonRow, 11, 1);

    scrollArea->setWidget(m_container);
    panelLayout->addWidget(scrollArea);

    m_footer = new GradientFrame(m_panel);
    m_footer->setObjectName("settingsFooter");
    m_footer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *footerLayout = new QHBoxLayout(m_footer);
    footerLayout->setContentsMargins(
        Theme::Spacing::SETTINGS_CONTAINER_MARGIN,
        Theme::Spacing::SETTINGS_CONTAINER_MARGIN,
        Theme::Spacing::SETTINGS_CONTAINER_MARGIN,
        Theme::Spacing::SETTINGS_CONTAINER_MARGIN);
    footerLayout->setSpacing(0);

    m_settingsCancelButton = new QPushButton("Cancel", m_footer);
    m_settingsCancelButton->setCursor(Qt::PointingHandCursor);
    m_settingsSaveButton = new QPushButton("Save", m_footer);
    m_settingsSaveButton->setCursor(Qt::PointingHandCursor);
    footerLayout->addWidget(m_settingsCancelButton, 0, Qt::AlignLeft);
    footerLayout->addStretch();
    footerLayout->addWidget(m_settingsSaveButton, 0, Qt::AlignRight);

    panelLayout->addWidget(m_footer);
    overlayLayout->addWidget(m_panel);
    setCurrentVersion(m_updater ? m_updater->currentVersion().toString() : QString());

    connect(m_settingsCancelButton, &QPushButton::clicked,
            this, &SettingsWidget::cancelRequested);
    connect(m_settingsSaveButton, &QPushButton::clicked,
            this, &SettingsWidget::applySettings);
    connect(m_checkNowButton, &QPushButton::clicked, this, [this]() {
        QString owner;
        QString repo;
        if (!parseGithubRepo(m_updateSourceEdit ? m_updateSourceEdit->text().trimmed() : QString(),
                             &owner, &repo)) {
            if (m_modalManager) {
                MessageModal::warning(m_modalManager, "Updater Settings",
                                     "Please enter a valid GitHub repository URL (e.g. https://github.com/owner/repo).");
            }
            return;
        }
        if (m_updater) {
            const QString channel = m_updateChannelCombo
                ? m_updateChannelCombo->currentData().toString()
                : QStringLiteral("stable");
            m_updater->setRepository(owner, repo);
            m_updater->setIncludePrereleases(channel == "prerelease");
        }
        if (m_checkStatusLabel) {
            m_checkStatusLabel->setText("Checking...");
        }
        emit manualCheckRequested();
    });
    connect(m_downloadBrowseButton, &QPushButton::clicked, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(
            this,
            "Select Download Folder",
            m_downloadDirEdit ? m_downloadDirEdit->text() : QString());
        if (!dir.isEmpty() && m_downloadDirEdit) {
            m_downloadDirEdit->setText(dir);
        }
    });
}

void SettingsWidget::applySettings() {
    if (!m_updateSourceEdit || !m_updateChannelCombo || !m_autoCheckCheckbox) {
        emit cancelRequested();
        return;
    }

    const QString sourceText = m_updateSourceEdit->text().trimmed();
    QString owner;
    QString repo;
    if (!parseGithubRepo(sourceText, &owner, &repo)) {
        if (m_modalManager) {
            MessageModal::warning(m_modalManager, "Updater Settings",
                                 "Please enter a valid GitHub repository URL (e.g. https://github.com/owner/repo).");
        }
        return;
    }

    const QString channel = m_updateChannelCombo->currentData().toString();
    const bool includePrereleases = (channel == "prerelease");
    if (m_updater) {
        m_updater->setRepository(owner, repo);
        m_updater->setIncludePrereleases(includePrereleases);
    }

    QSettings settings("TrenchKit", "FoxholeModManager");
    settings.setValue("updater/source", sourceText);
    settings.setValue("updater/channel", channel);
    settings.setValue("updater/autoCheck", m_autoCheckCheckbox->isChecked());
    settings.setValue("updater/downloadDir",
                      m_downloadDirEdit ? m_downloadDirEdit->text() : QString());

    emit settingsApplied(m_autoCheckCheckbox->isChecked());
}

void SettingsWidget::loadSettings(bool applyToUpdater) {
    QSettings settings("TrenchKit", "FoxholeModManager");
    const QString defaultSource = QStringLiteral("https://github.com/Tapawingo/TrenchKit");
    const QString source = settings.value("updater/source", defaultSource).toString();
    QString channel = settings.value("updater/channel", QString()).toString();
    if (channel.isEmpty()) {
        const bool legacyPrerelease = settings.value("updater/includePrereleases", false).toBool();
        channel = legacyPrerelease ? QStringLiteral("prerelease") : QStringLiteral("stable");
    }
    const bool includePrereleases = (channel == "prerelease");
    const bool autoCheck = settings.value("updater/autoCheck", true).toBool();
    const QString downloadDir = settings.value("updater/downloadDir", QString()).toString();

    if (m_updateSourceEdit) {
        m_updateSourceEdit->setText(source);
    }
    if (m_updateChannelCombo) {
        const int channelIndex = m_updateChannelCombo->findData(channel);
        if (channelIndex >= 0) {
            m_updateChannelCombo->setCurrentIndex(channelIndex);
        } else {
            m_updateChannelCombo->setCurrentIndex(0);
        }
    }
    if (m_autoCheckCheckbox) {
        m_autoCheckCheckbox->setChecked(autoCheck);
    }
    if (m_downloadDirEdit) {
        m_downloadDirEdit->setText(downloadDir);
    }

    if (applyToUpdater && m_updater) {
        QString owner;
        QString repo;
        if (parseGithubRepo(source, &owner, &repo)) {
            m_updater->setRepository(owner, repo);
        }
        m_updater->setIncludePrereleases(includePrereleases);
        setCurrentVersion(m_updater->currentVersion().toString());
    }
}

bool SettingsWidget::parseGithubRepo(const QString &text, QString *owner, QString *repo) {
    if (!owner || !repo) return false;
    QUrl url(text);
    if (!url.isValid() || url.host().toLower() != QStringLiteral("github.com")) {
        return false;
    }
    const QStringList parts = url.path().split('/', Qt::SkipEmptyParts);
    if (parts.size() < 2) {
        return false;
    }
    QString repoPart = parts.at(1);
    if (repoPart.endsWith(".git")) {
        repoPart.chop(4);
    }
    *owner = parts.at(0);
    *repo = repoPart;
    return !owner->isEmpty() && !repo->isEmpty();
}

void SettingsWidget::setNexusServices(NexusModsClient *client, NexusModsAuth *auth) {
    m_nexusClient = client;
    m_nexusAuth = auth;

    if (!m_nexusStatusLabel || !m_nexusAuthButton || !m_nexusClearButton) {
        return;
    }

    auto updateStatus = [this]() {
        if (m_nexusClient && m_nexusClient->hasApiKey()) {
            m_nexusStatusLabel->setText(QStringLiteral("Connected"));
            m_nexusAuthButton->setVisible(false);
            m_nexusClearButton->setVisible(true);
        } else {
            m_nexusStatusLabel->setText(QStringLiteral("Not connected"));
            m_nexusAuthButton->setVisible(true);
            m_nexusClearButton->setVisible(false);
        }
    };

    updateStatus();

    if (m_nexusAuth) {
        connect(m_nexusAuth, &NexusModsAuth::authenticationStarted, this, [this](const QString &url) {
            if (m_nexusStatusLabel) {
                m_nexusStatusLabel->setText(QStringLiteral("Authenticating..."));
            }
            QDesktopServices::openUrl(QUrl(url));
        });

        connect(m_nexusAuth, &NexusModsAuth::authenticationComplete, this, [this, updateStatus](const QString &apiKey) {
            if (m_nexusClient) {
                m_nexusClient->setApiKey(apiKey);
            }
            if (m_nexusStatusLabel) {
                m_nexusStatusLabel->setText(QStringLiteral("Connected"));
            }
            updateStatus();
            if (m_modalManager) {
                MessageModal::information(m_modalManager, QStringLiteral("Success"),
                                       QStringLiteral("Successfully authenticated with Nexus Mods!"));
            }
        });

        connect(m_nexusAuth, &NexusModsAuth::authenticationFailed, this, [this, updateStatus](const QString &error) {
            updateStatus();
            if (m_modalManager) {
                MessageModal::warning(m_modalManager, QStringLiteral("Authentication Failed"), error);
            }
        });
    }

    if (m_nexusAuthButton) {
        connect(m_nexusAuthButton, &QPushButton::clicked, this, [this, updateStatus]() {
            if (!m_modalManager) {
                return;
            }

            auto *modal = new MessageModal(
                QStringLiteral("Authentication Method"),
                QStringLiteral("SSO authentication requires 'trenchkit' to be registered with Nexus Mods.\n\n"
                             "Would you like to use SSO (if registered) or enter an API key manually?\n\n"
                             "You can get a personal API key from: https://www.nexusmods.com/users/myaccount?tab=api+access"),
                MessageModal::Question,
                MessageModal::Yes | MessageModal::No
            );
            connect(modal, &MessageModal::finished, this, [this, modal, updateStatus]() {
                if (modal->clickedButton() == MessageModal::Yes) {
                    if (m_nexusAuth) {
                        m_nexusAuth->startAuthentication();
                    }
                } else {
                    auto *inputModal = new InputModal(
                        QStringLiteral("Enter API Key"),
                        QStringLiteral("Enter your Nexus Mods API key:\n"
                                     "(Get it from: https://www.nexusmods.com/users/myaccount?tab=api+access)"),
                        QString()
                    );
                    connect(inputModal, &InputModal::accepted, this, [this, inputModal, updateStatus]() {
                        QString apiKey = inputModal->textValue();
                        if (!apiKey.isEmpty()) {
                            if (m_nexusClient) {
                                m_nexusClient->setApiKey(apiKey);
                                updateStatus();
                                if (m_modalManager) {
                                    MessageModal::information(m_modalManager, QStringLiteral("Success"),
                                                           QStringLiteral("API key saved successfully!"));
                                }
                            }
                        }
                    });
                    m_modalManager->showModal(inputModal);
                }
            });
            m_modalManager->showModal(modal);
        });
    }

    if (m_nexusClearButton) {
        connect(m_nexusClearButton, &QPushButton::clicked, this, [this, updateStatus]() {
            if (!m_modalManager) {
                return;
            }

            auto *modal = new MessageModal(
                QStringLiteral("Clear API Key"),
                QStringLiteral("Are you sure you want to clear your Nexus Mods API key?"),
                MessageModal::Question,
                MessageModal::Yes | MessageModal::No
            );
            connect(modal, &MessageModal::finished, this, [this, modal, updateStatus]() {
                if (modal->clickedButton() == MessageModal::Yes && m_nexusClient) {
                    m_nexusClient->clearApiKey();
                    updateStatus();
                }
            });
            m_modalManager->showModal(modal);
        });
    }
}
