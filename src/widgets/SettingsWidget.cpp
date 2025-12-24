#include "SettingsWidget.h"

#include "../utils/UpdaterService.h"
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
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QUrl>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QDir>

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
        return QDir(QCoreApplication::applicationDirPath()).filePath("updates");
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
    containerLayout->setRowStretch(9, 1);

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
    m_downloadDirEdit->setPlaceholderText("Default: <appdir>/updates");
    m_downloadDirEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_downloadBrowseButton = new QPushButton("Browse...", m_panel);
    m_downloadBrowseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    downloadRow->addWidget(m_downloadDirEdit);
    downloadRow->addWidget(m_downloadBrowseButton);
    containerLayout->addLayout(downloadRow, 6, 1);

    auto *checkLabel = new QLabel("Check for updates now", m_panel);
    checkLabel->setObjectName("settingsLabel");
    checkLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(checkLabel, 7, 0);

    m_checkNowButton = new QPushButton("Check now", m_panel);
    m_checkNowButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(m_checkNowButton, 7, 1);

    m_checkStatusLabel = new QLabel("Idle", m_panel);
    m_checkStatusLabel->setObjectName("settingsStatus");
    m_checkStatusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    containerLayout->addWidget(m_checkStatusLabel, 8, 1);

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
    m_settingsSaveButton = new QPushButton("Save", m_footer);
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
            QMessageBox::warning(this, "Updater Settings",
                                 "Please enter a valid GitHub repository URL (e.g. https://github.com/owner/repo).");
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
        QMessageBox::warning(this, "Updater Settings",
                             "Please enter a valid GitHub repository URL (e.g. https://github.com/owner/repo).");
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
