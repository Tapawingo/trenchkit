#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QPointer>

class UpdaterService;
class PanelFrame;
class GradientFrame;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLabel;

class SettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent, UpdaterService *updater);
    ~SettingsWidget() override = default;

    bool applyStoredSettings();
    void loadSettingsIntoUi();
    QString resolvedDownloadDir() const;
    void setCurrentVersion(const QString &version);

signals:
    void cancelRequested();
    void settingsApplied(bool autoCheck);
    void manualCheckRequested();

private:
    void buildUi();
    void applySettings();
    void loadSettings(bool applyToUpdater);
    static bool parseGithubRepo(const QString &text, QString *owner, QString *repo);

    QPointer<UpdaterService> m_updater;
    PanelFrame *m_panel = nullptr;
    GradientFrame *m_container = nullptr;
    GradientFrame *m_footer = nullptr;
    QLineEdit *m_updateSourceEdit = nullptr;
    QComboBox *m_updateChannelCombo = nullptr;
    QCheckBox *m_autoCheckCheckbox = nullptr;
    QLineEdit *m_downloadDirEdit = nullptr;
    QPushButton *m_downloadBrowseButton = nullptr;
    QPushButton *m_checkNowButton = nullptr;
    QPushButton *m_settingsSaveButton = nullptr;
    QPushButton *m_settingsCancelButton = nullptr;
    QLabel *m_versionLabel = nullptr;
};

#endif // SETTINGSWIDGET_H
