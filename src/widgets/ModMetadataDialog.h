#ifndef MODMETADATADIALOG_H
#define MODMETADATADIALOG_H

#include "../utils/ModInfo.h"
#include <QDialog>

class QLineEdit;
class QTextEdit;
class QDateTimeEdit;
class QDialogButtonBox;

class ModMetadataDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModMetadataDialog(const ModInfo &mod, QWidget *parent = nullptr);
    ~ModMetadataDialog() override = default;

    ModInfo getModInfo() const;

private:
    void setupUi(const ModInfo &mod);

    QString m_modId;
    QLineEdit *m_nameEdit;
    QLineEdit *m_fileNameEdit;
    QTextEdit *m_descriptionEdit;
    QLineEdit *m_nexusModIdEdit;
    QLineEdit *m_nexusFileIdEdit;
    QLineEdit *m_itchGameIdEdit;
    QLineEdit *m_versionEdit;
    QLineEdit *m_authorEdit;
    QDateTimeEdit *m_installDateEdit;
    QDialogButtonBox *m_buttonBox;

    int m_priority;
    bool m_enabled;
    QString m_numberedFileName;
};

#endif // MODMETADATADIALOG_H
