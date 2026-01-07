#ifndef LAUNCHWIDGET_H
#define LAUNCHWIDGET_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QProcess>

class QToolButton;
class ModManager;
class ModalManager;

class LaunchWidget : public QWidget {
    Q_OBJECT

public:
    explicit LaunchWidget(QWidget *parent = nullptr);
    ~LaunchWidget() override = default;

    void setModManager(ModManager *modManager);
    void setModalManager(ModalManager *modalManager) { m_modalManager = modalManager; }
    void setFoxholeInstallPath(const QString &path);

signals:
    void errorOccurred(const QString &error);
    void gameLaunched(bool withMods);
    void modsRestored(int modCount);

private slots:
    void onLaunchWithMods();
    void onLaunchWithoutMods();
    void onGameProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void setupUi();
    void setupConnections();
    QString getFoxholeExecutablePath() const;

    ModManager *m_modManager = nullptr;
    ModalManager *m_modalManager = nullptr;
    QString m_foxholeInstallPath;
    QProcess *m_gameProcess = nullptr;
    QStringList m_modsToRestore;

    QToolButton *m_launchButton;
};

#endif // LAUNCHWIDGET_H
