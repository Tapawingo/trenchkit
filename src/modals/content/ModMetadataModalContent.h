#ifndef MODMETADATAMODALCONTENT_H
#define MODMETADATAMODALCONTENT_H

#include "../BaseModalContent.h"
#include "../../utils/ModInfo.h"

class QLineEdit;
class QTextEdit;
class QDateTimeEdit;

class ModMetadataModalContent : public BaseModalContent {
    Q_OBJECT

public:
    explicit ModMetadataModalContent(const ModInfo &mod, QWidget *parent = nullptr);

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
    QDateTimeEdit *m_uploadDateEdit;

    int m_priority;
    bool m_enabled;
    QString m_numberedFileName;
};

#endif // MODMETADATAMODALCONTENT_H
