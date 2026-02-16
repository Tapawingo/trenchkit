#ifndef INPUTMODAL_H
#define INPUTMODAL_H

#include "BaseModalContent.h"
#include <QString>

class QEvent;
class QLabel;
class QLineEdit;
class QPushButton;

class InputModal : public BaseModalContent {
    Q_OBJECT

public:
    explicit InputModal(const QString &title,
                       const QString &label,
                       const QString &defaultValue = QString(),
                       QWidget *parent = nullptr);

    QString textValue() const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupUi(const QString &label, const QString &defaultValue);
    void retranslateUi();

    QLineEdit *m_lineEdit;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // INPUTMODAL_H
