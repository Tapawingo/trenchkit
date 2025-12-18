#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class InstallPathWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMinimizeClicked();
    void onCloseClicked();
    void onInstallPathChanged(const QString &path);

private:
    void setupTitleBar();
    void setupInstallPath();
    void loadSettings();
    void saveSettings();

    Ui::MainWindow *ui;
    InstallPathWidget *m_installPathWidget;
};

#endif // MAINWINDOW_H
