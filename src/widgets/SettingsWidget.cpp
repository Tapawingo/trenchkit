#include "SettingsWidget.h"

#include "../modals/ModalManager.h"
#include "../modals/MessageModal.h"
#include "../modals/InputModal.h"
#include "../utils/UpdaterService.h"
#include "../utils/NexusModsClient.h"
#include "../utils/NexusModsAuth.h"
#include "../utils/ItchClient.h"
#include "../utils/ItchAuth.h"
#include "../utils/Logger.h"
#include "../utils/Theme.h"
#include "PanelFrame.h"
#include "GradientFrame.h"

#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QInputDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <shobjidl.h>
#endif

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
    containerLayout->setRowStretch(21, 1);

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
    m_updateSourceEdit->setFocusPolicy(Qt::ClickFocus);
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
    m_downloadDirEdit->setFocusPolicy(Qt::ClickFocus);
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

    auto *shortcutsHeader = new QLabel("Shortcuts", m_panel);
    shortcutsHeader->setObjectName("settingsSectionHeader");
    shortcutsHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(shortcutsHeader, 9, 0, 1, 2);

    auto *desktopShortcutLabel = new QLabel("Desktop shortcut", m_panel);
    desktopShortcutLabel->setObjectName("settingsLabel");
    desktopShortcutLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(desktopShortcutLabel, 10, 0);

    m_addDesktopShortcutButton = new QPushButton("Add Desktop Shortcut", m_panel);
    m_addDesktopShortcutButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_addDesktopShortcutButton->setCursor(Qt::PointingHandCursor);
    containerLayout->addWidget(m_addDesktopShortcutButton, 10, 1);

    auto *startMenuLabel = new QLabel("Start menu", m_panel);
    startMenuLabel->setObjectName("settingsLabel");
    startMenuLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(startMenuLabel, 11, 0);

    m_addStartMenuShortcutButton = new QPushButton("Add to Start Menu", m_panel);
    m_addStartMenuShortcutButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_addStartMenuShortcutButton->setCursor(Qt::PointingHandCursor);
    containerLayout->addWidget(m_addStartMenuShortcutButton, 11, 1);

    auto *nexusHeader = new QLabel("Nexus Mods Integration", m_panel);
    nexusHeader->setObjectName("settingsSectionHeader");
    nexusHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(nexusHeader, 12, 0, 1, 2);

    auto *nexusLabel = new QLabel("Connection status", m_panel);
    nexusLabel->setObjectName("settingsLabel");
    nexusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(nexusLabel, 13, 0);

    m_nexusStatusLabel = new QLabel("Not connected", m_panel);
    m_nexusStatusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(m_nexusStatusLabel, 13, 1);

    auto *nexusActionsLabel = new QLabel("Authentication", m_panel);
    nexusActionsLabel->setObjectName("settingsLabel");
    nexusActionsLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(nexusActionsLabel, 14, 0);

    auto *nexusButtonRow = new QHBoxLayout();
    m_nexusAuthButton = new QPushButton("Authenticate (SSO)", m_panel);
    m_nexusAuthButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nexusAuthButton->setCursor(Qt::PointingHandCursor);
    m_nexusClearButton = new QPushButton("Clear API Key", m_panel);
    m_nexusClearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nexusClearButton->setCursor(Qt::PointingHandCursor);
    nexusButtonRow->addWidget(m_nexusAuthButton);
    nexusButtonRow->addWidget(m_nexusClearButton);
    nexusButtonRow->addStretch();
    containerLayout->addLayout(nexusButtonRow, 14, 1);

    auto *itchHeader = new QLabel("Itch.io Integration", m_panel);
    itchHeader->setObjectName("settingsSectionHeader");
    itchHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(itchHeader, 15, 0, 1, 2);

    auto *itchLabel = new QLabel("Connection status", m_panel);
    itchLabel->setObjectName("settingsLabel");
    itchLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(itchLabel, 16, 0);

    m_itchStatusLabel = new QLabel("Not connected", m_panel);
    m_itchStatusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(m_itchStatusLabel, 16, 1);

    auto *itchActionsLabel = new QLabel("Authentication", m_panel);
    itchActionsLabel->setObjectName("settingsLabel");
    itchActionsLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(itchActionsLabel, 17, 0);

    auto *itchButtonRow = new QHBoxLayout();
    m_itchAuthButton = new QPushButton("Add API Key", m_panel);
    m_itchAuthButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_itchAuthButton->setCursor(Qt::PointingHandCursor);
    m_itchClearButton = new QPushButton("Clear API Key", m_panel);
    m_itchClearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_itchClearButton->setCursor(Qt::PointingHandCursor);
    itchButtonRow->addWidget(m_itchAuthButton);
    itchButtonRow->addWidget(m_itchClearButton);
    itchButtonRow->addStretch();
    containerLayout->addLayout(itchButtonRow, 17, 1);

    auto *loggingHeader = new QLabel("Logging", m_panel);
    loggingHeader->setObjectName("settingsSectionHeader");
    loggingHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(loggingHeader, 18, 0, 1, 2);

    auto *logLocationLabel = new QLabel("Log files location", m_panel);
    logLocationLabel->setObjectName("settingsLabel");
    logLocationLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(logLocationLabel, 19, 0);

    m_logPathLabel = new QLabel(Logger::instance().logDirectory(), m_panel);
    m_logPathLabel->setWordWrap(true);
    m_logPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_logPathLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    containerLayout->addWidget(m_logPathLabel, 19, 1);

    auto *logActionsLabel = new QLabel("Access logs", m_panel);
    logActionsLabel->setObjectName("settingsLabel");
    logActionsLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(logActionsLabel, 20, 0);

    auto *logButtonRow = new QHBoxLayout();
    m_openLogsButton = new QPushButton("Open Log Folder", m_panel);
    m_openLogsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_openLogsButton->setCursor(Qt::PointingHandCursor);
    m_copyLogPathButton = new QPushButton("Copy Log Path", m_panel);
    m_copyLogPathButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_copyLogPathButton->setCursor(Qt::PointingHandCursor);
    logButtonRow->addWidget(m_openLogsButton);
    logButtonRow->addWidget(m_copyLogPathButton);
    logButtonRow->addStretch();
    containerLayout->addLayout(logButtonRow, 20, 1);

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

    connect(m_openLogsButton, &QPushButton::clicked, this, &SettingsWidget::onOpenLogsClicked);
    connect(m_copyLogPathButton, &QPushButton::clicked, this, &SettingsWidget::onCopyLogPathClicked);
    connect(m_addDesktopShortcutButton, &QPushButton::clicked, this, &SettingsWidget::onAddDesktopShortcutClicked);
    connect(m_addStartMenuShortcutButton, &QPushButton::clicked, this, &SettingsWidget::onAddStartMenuShortcutClicked);
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
        disconnect(m_nexusAuth, &NexusModsAuth::authenticationStarted, this, nullptr);
        disconnect(m_nexusAuth, &NexusModsAuth::authenticationComplete, this, nullptr);
        disconnect(m_nexusAuth, &NexusModsAuth::authenticationFailed, this, nullptr);

        connect(m_nexusAuth, &NexusModsAuth::authenticationStarted, this, [this](const QString &) {
            if (m_nexusStatusLabel) {
                m_nexusStatusLabel->setText(QStringLiteral("Authenticating... (check your browser)"));
            }
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
        connect(m_nexusAuthButton, &QPushButton::clicked, this, [this]() {
            if (!m_nexusAuth) {
                return;
            }

            // Create a one-time connection to open browser for Settings-initiated auth
            QMetaObject::Connection *conn = new QMetaObject::Connection();
            *conn = connect(m_nexusAuth, &NexusModsAuth::authenticationStarted, this, [conn](const QString &url) {
                QDesktopServices::openUrl(QUrl(url));
                QObject::disconnect(*conn);
                delete conn;
            });
            m_nexusAuth->startAuthentication();
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

void SettingsWidget::setItchServices(ItchClient *client, ItchAuth *auth) {
    m_itchClient = client;
    m_itchAuth = auth;

    auto updateStatus = [this]() {
        if (m_itchClient && m_itchClient->hasApiKey()) {
            m_itchStatusLabel->setText(QStringLiteral("Connected"));
            m_itchAuthButton->setVisible(false);
            m_itchClearButton->setVisible(true);
        } else {
            m_itchStatusLabel->setText(QStringLiteral("Not connected"));
            m_itchAuthButton->setVisible(true);
            m_itchClearButton->setVisible(false);
        }
    };

    updateStatus();

    if (m_itchAuthButton) {
        connect(m_itchAuthButton, &QPushButton::clicked, this, [this, updateStatus]() {
            if (!m_modalManager) {
                return;
            }

            auto *inputModal = new InputModal(
                QStringLiteral("Enter API Key"),
                QStringLiteral("Enter your itch.io API key:\n\n"
                             "1. Visit https://itch.io/user/settings/api-keys\n"
                             "2. Click \"Generate new API key\"\n"
                             "3. Copy the key and paste it below"),
                QString()
            );

            connect(inputModal, &InputModal::accepted, this, [this, inputModal, updateStatus]() {
                QString apiKey = inputModal->textValue().trimmed();
                if (!apiKey.isEmpty()) {
                    if (m_itchClient) {
                        m_itchClient->setApiKey(apiKey);
                        updateStatus();
                        if (m_modalManager) {
                            MessageModal::information(m_modalManager,
                                                   QStringLiteral("Success"),
                                                   QStringLiteral("API key saved successfully!"));
                        }
                    }
                }
            });

            m_modalManager->showModal(inputModal);
        });
    }

    if (m_itchClearButton) {
        connect(m_itchClearButton, &QPushButton::clicked, this, [this, updateStatus]() {
            if (!m_modalManager) {
                return;
            }

            auto *modal = new MessageModal(
                QStringLiteral("Clear API Key"),
                QStringLiteral("Are you sure you want to clear your itch.io API key?"),
                MessageModal::Question,
                MessageModal::Yes | MessageModal::No
            );

            connect(modal, &MessageModal::finished, this, [this, modal, updateStatus]() {
                if (modal->clickedButton() == MessageModal::Yes && m_itchClient) {
                    m_itchClient->clearApiKey();
                    updateStatus();
                }
            });

            m_modalManager->showModal(modal);
        });
    }
}

void SettingsWidget::onOpenLogsClicked() {
    QString logDir = Logger::instance().logDirectory();
    QDesktopServices::openUrl(QUrl::fromLocalFile(logDir));
}

void SettingsWidget::onCopyLogPathClicked() {
    QString logPath = Logger::instance().currentLogFile();
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(logPath);

    if (m_copyLogPathButton) {
        m_copyLogPathButton->setText("Copied!");
        QTimer::singleShot(2000, this, [this]() {
            if (m_copyLogPathButton) {
                m_copyLogPathButton->setText("Copy Log Path");
            }
        });
    }
}

bool SettingsWidget::createShortcut(const QString &shortcutPath, const QString &targetPath, const QString &description) {
#ifdef Q_OS_WIN
    CoInitialize(nullptr);

    IShellLinkW *shellLink = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&shellLink);

    if (FAILED(hr) || !shellLink) {
        CoUninitialize();
        return false;
    }

    shellLink->SetPath(reinterpret_cast<const wchar_t*>(targetPath.utf16()));
    shellLink->SetDescription(reinterpret_cast<const wchar_t*>(description.utf16()));

    QString workingDir = QFileInfo(targetPath).absolutePath();
    shellLink->SetWorkingDirectory(reinterpret_cast<const wchar_t*>(workingDir.utf16()));

    IPersistFile *persistFile = nullptr;
    hr = shellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&persistFile);

    bool success = false;
    if (SUCCEEDED(hr) && persistFile) {
        hr = persistFile->Save(reinterpret_cast<const wchar_t*>(shortcutPath.utf16()), TRUE);
        success = SUCCEEDED(hr);
        persistFile->Release();
    }

    shellLink->Release();
    CoUninitialize();

    return success;
#else
    Q_UNUSED(shortcutPath);
    Q_UNUSED(targetPath);
    Q_UNUSED(description);
    return false;
#endif
}

void SettingsWidget::onAddDesktopShortcutClicked() {
#ifdef Q_OS_WIN
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString shortcutPath = desktopPath + "/TrenchKit.lnk";

    QString exePath = QCoreApplication::applicationFilePath();

    if (QFile::exists(shortcutPath)) {
        if (m_modalManager) {
            MessageModal::information(m_modalManager, "Shortcut Already Exists",
                "A desktop shortcut for TrenchKit already exists.");
        }
        return;
    }

    bool success = createShortcut(shortcutPath, exePath, "TrenchKit - Foxhole Mod Manager");

    if (m_modalManager) {
        if (success) {
            MessageModal::information(m_modalManager, "Success",
                "Desktop shortcut created successfully.");
        } else {
            MessageModal::warning(m_modalManager, "Error",
                "Failed to create desktop shortcut.");
        }
    }
#else
    if (m_modalManager) {
        MessageModal::information(m_modalManager, "Not Supported",
            "Shortcut creation is only supported on Windows.");
    }
#endif
}

void SettingsWidget::onAddStartMenuShortcutClicked() {
#ifdef Q_OS_WIN
    QString startMenuPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QDir().mkpath(startMenuPath + "/TrenchKit");
    QString shortcutPath = startMenuPath + "/TrenchKit/TrenchKit.lnk";

    QString exePath = QCoreApplication::applicationFilePath();

    if (QFile::exists(shortcutPath)) {
        if (m_modalManager) {
            MessageModal::information(m_modalManager, "Shortcut Already Exists",
                "A Start Menu shortcut for TrenchKit already exists.");
        }
        return;
    }

    bool success = createShortcut(shortcutPath, exePath, "TrenchKit - Foxhole Mod Manager");

    if (m_modalManager) {
        if (success) {
            MessageModal::information(m_modalManager, "Success",
                "Start Menu shortcut created successfully.");
        } else {
            MessageModal::warning(m_modalManager, "Error",
                "Failed to create Start Menu shortcut.");
        }
    }
#else
    if (m_modalManager) {
        MessageModal::information(m_modalManager, "Not Supported",
            "Shortcut creation is only supported on Windows.");
    }
#endif
}
