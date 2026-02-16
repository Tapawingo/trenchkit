#ifndef RIGHTPANELWIDGET_H
#define RIGHTPANELWIDGET_H

#include <QWidget>
#include <QString>
#include <QEvent>

class ModManager;
class ModalManager;
class ActionsWidget;
class BackupWidget;
class LaunchWidget;
class ActivityLogWidget;

class RightPanelWidget : public QWidget {
    Q_OBJECT

public:
    explicit RightPanelWidget(QWidget *parent = nullptr);
    ~RightPanelWidget() override = default;

    void setModManager(ModManager *modManager);
    void setModalManager(ModalManager *modalManager);
    void setFoxholeInstallPath(const QString &path);

    ActivityLogWidget* getActivityLog() const { return m_activityLogWidget; }

protected:
    void changeEvent(QEvent *event) override;

signals:
    void addModRequested();
    void removeModRequested();
    void moveUpRequested();
    void moveDownRequested();
    void errorOccurred(const QString &error);

public slots:
    void onModSelectionChanged(int selectedRow, int totalMods);

private:
    void setupUi();
    void retranslateUi();
    void setupConnections();

    ModManager *m_modManager = nullptr;
    QString m_foxholeInstallPath;

    ActionsWidget *m_actionsWidget;
    BackupWidget *m_backupWidget;
    ActivityLogWidget *m_activityLogWidget;
    LaunchWidget *m_launchWidget;
};

#endif // RIGHTPANELWIDGET_H
