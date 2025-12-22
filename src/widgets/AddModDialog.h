#ifndef ADDMODDIALOG_H
#define ADDMODDIALOG_H

#include <QDialog>
#include <QString>

class QPushButton;
class QDialogButtonBox;
class ModManager;

class AddModDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddModDialog(ModManager *modManager, QWidget *parent = nullptr);

signals:
    void modAdded(const QString &modName);

private slots:
    void onFromFileClicked();
    void onFromNexusClicked();
    void onFromItchClicked();

private:
    void setupUi();
    void handleZipFile(const QString &zipPath);
    void handlePakFile(const QString &pakPath);

    ModManager *m_modManager;
    QPushButton *m_fromFileButton;
    QPushButton *m_fromNexusButton;
    QPushButton *m_fromItchButton;
    QDialogButtonBox *m_buttonBox;
};

#endif // ADDMODDIALOG_H
