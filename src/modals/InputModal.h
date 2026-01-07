#ifndef INPUTMODAL_H
#define INPUTMODAL_H

#include "BaseModalContent.h"
#include <QString>

class QLabel;
class QLineEdit;

class InputModal : public BaseModalContent {
    Q_OBJECT

public:
    explicit InputModal(const QString &title,
                       const QString &label,
                       const QString &defaultValue = QString(),
                       QWidget *parent = nullptr);

    QString textValue() const;

private:
    void setupUi(const QString &label, const QString &defaultValue);

    QLineEdit *m_lineEdit;
};

#endif // INPUTMODAL_H
