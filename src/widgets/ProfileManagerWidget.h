#ifndef PROFILEMANAGERWIDGET_H
#define PROFILEMANAGERWIDGET_H

#include <QWidget>
#include <QString>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QVBoxLayout;
class ProfileManager;

class ProfileManagerWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProfileManagerWidget(QWidget *parent = nullptr);
    ~ProfileManagerWidget() override = default;

    void setProfileManager(ProfileManager *profileManager);
    void refreshProfileList();

signals:
    void profileSelected(const QString &profileId);
    void profileLoadRequested(const QString &profileId);

private slots:
    void onCreateClicked();
    void onLoadClicked();
    void onUpdateClicked();
    void onExportClicked();
    void onImportClicked();
    void onDeleteClicked();
    void onProfilesChanged();
    void onActiveProfileChanged(const QString &profileId);
    void onItemSelectionChanged();
    void onItemDoubleClicked(QListWidgetItem *item);

private:
    void setupUi();
    void setupConnections();
    QString getSelectedProfileId() const;
    void updateButtonStates();
    void showValidationDialog(const QString &profileId);

    QLabel *m_titleLabel;
    QListWidget *m_profileList;
    QPushButton *m_createButton;
    QPushButton *m_loadButton;
    QPushButton *m_updateButton;
    QPushButton *m_exportButton;
    QPushButton *m_importButton;
    QPushButton *m_deleteButton;
    QVBoxLayout *m_layout;

    ProfileManager *m_profileManager = nullptr;
};

#endif // PROFILEMANAGERWIDGET_H
