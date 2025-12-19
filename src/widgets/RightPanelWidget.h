#ifndef RIGHTPANELWIDGET_H
#define RIGHTPANELWIDGET_H

#include <QWidget>
#include <QString>
#include <QProcess>

class QPushButton;
class QToolButton;
class QFrame;
class QLabel;
class ModManager;

class RightPanelWidget : public QWidget {
    Q_OBJECT

public:
    explicit RightPanelWidget(QWidget *parent = nullptr);
    ~RightPanelWidget() override = default;

    void setModManager(ModManager *modManager);
    void setFoxholeInstallPath(const QString &path);

signals:
    void addModRequested();
    void removeModRequested();
    void moveUpRequested();
    void moveDownRequested();
    void errorOccurred(const QString &error);

public slots:
    void onModSelectionChanged(int selectedRow, int totalMods);

private slots:
    void onAddModClicked();
    void onRemoveModClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onExploreFolderClicked();
    void onCreateBackupClicked();
    void onRestoreBackupClicked();
    void onLaunchWithMods();
    void onLaunchWithoutMods();
    void onGameProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void setupUi();
    void setupActionsSection();
    void setupBackupSection();
    void setupLaunchSection();
    QFrame* createSeparator();

    QString getFoxholeExecutablePath() const;
    QString getBackupsPath() const;

    ModManager *m_modManager = nullptr;
    QString m_foxholeInstallPath;
    int m_selectedRow = -1;
    int m_totalMods = 0;

    QProcess *m_gameProcess = nullptr;
    QStringList m_modsToRestore;

    QPushButton *m_addButton;
    QPushButton *m_removeButton;
    QPushButton *m_moveUpButton;
    QPushButton *m_moveDownButton;
    QPushButton *m_exploreFolderButton;
    QPushButton *m_createBackupButton;
    QPushButton *m_restoreBackupButton;
    QToolButton *m_launchButton;
};

#endif // RIGHTPANELWIDGET_H
