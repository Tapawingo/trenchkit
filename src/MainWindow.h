#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFutureWatcher>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class InstallPathWidget;
class ModListWidget;
class RightPanelWidget;
class ProfileManagerWidget;
class ModManager;
class ProfileManager;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onMinimizeClicked();
    void onCloseClicked();
    void onInstallPathChanged(const QString &path);
    void onModsLoadComplete();
    void onUnregisteredModsDetectionComplete();

private:
    void setupTitleBar();
    void setupInstallPath();
    void setupProfileManager();
    void setupModList();
    void setupRightPanel();
    void loadSettings();
    void saveSettings();

    Ui::MainWindow *ui;
    InstallPathWidget *m_installPathWidget;
    ProfileManagerWidget *m_profileManagerWidget;
    ModListWidget *m_modListWidget;
    RightPanelWidget *m_rightPanelWidget;
    ModManager *m_modManager;
    ProfileManager *m_profileManager;
    QFutureWatcher<bool> *m_modLoadWatcher;
    QFutureWatcher<void> *m_unregisteredModsWatcher;
    bool m_firstShow = true;
};

#endif // MAINWINDOW_H
