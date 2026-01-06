#ifndef INPUTMODAL_H
#define INPUTMODAL_H

#include "BaseModalContent.h"
#include <QString>

class QLabel;
class QLineEdit;
class ModalManager;

class InputModal : public BaseModalContent {
    Q_OBJECT

public:
    explicit InputModal(const QString &title,
                       const QString &label,
                       const QString &defaultValue = QString(),
                       QWidget *parent = nullptr);

    QString textValue() const;

    static QString getText(ModalManager *manager,
                          const QString &title,
                          const QString &label,
                          const QString &defaultValue = QString(),
                          bool *ok = nullptr);

private:
    void setupUi(const QString &label, const QString &defaultValue);

    QLineEdit *m_lineEdit;
};

#endif // INPUTMODAL_H
