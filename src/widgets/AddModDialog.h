#ifndef ADDMODDIALOG_H
#define ADDMODDIALOG_H

#include <QDialog>
#include <QString>

class QPushButton;
class QDialogButtonBox;
class ModManager;
class NexusModsClient;
class NexusModsAuth;

class AddModDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddModDialog(ModManager *modManager,
                         NexusModsClient *nexusClient,
                         NexusModsAuth *nexusAuth,
                         QWidget *parent = nullptr);

signals:
    void modAdded(const QString &modName);

private slots:
    void onFromFileClicked();
    void onFromNexusClicked();
    void onFromItchClicked();

private:
    void setupUi();
    void handleZipFile(const QString &zipPath, const QString &nexusModId = QString(), const QString &nexusFileId = QString(),
                       const QString &author = QString(), const QString &description = QString(), const QString &version = QString());
    void handlePakFile(const QString &pakPath, const QString &nexusModId = QString(), const QString &nexusFileId = QString(),
                       const QString &author = QString(), const QString &description = QString(), const QString &version = QString());

    ModManager *m_modManager;
    NexusModsClient *m_nexusClient;
    NexusModsAuth *m_nexusAuth;
    QPushButton *m_fromFileButton;
    QPushButton *m_fromNexusButton;
    QPushButton *m_fromItchButton;
    QDialogButtonBox *m_buttonBox;
};

#endif // ADDMODDIALOG_H
