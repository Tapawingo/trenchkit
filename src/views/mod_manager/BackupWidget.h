#ifndef BACKUPWIDGET_H
#define BACKUPWIDGET_H

#include <QWidget>
#include <QString>

class QPushButton;
class ModManager;
class ModalManager;

class BackupWidget : public QWidget {
    Q_OBJECT

public:
    explicit BackupWidget(QWidget *parent = nullptr);
    ~BackupWidget() override = default;

    void setModManager(ModManager *modManager);
    void setModalManager(ModalManager *modalManager) { m_modalManager = modalManager; }

signals:
    void errorOccurred(const QString &error);
    void backupCreated(int fileCount);
    void backupRestored(const QString &backupName);

private slots:
    void onCreateBackupClicked();
    void onRestoreBackupClicked();

private:
    void setupUi();
    void setupConnections();
    QString getBackupsPath() const;

    ModManager *m_modManager = nullptr;
    ModalManager *m_modalManager = nullptr;

    QPushButton *m_createBackupButton;
    QPushButton *m_restoreBackupButton;
};

#endif // BACKUPWIDGET_H
