#ifndef PROGRESSMODAL_H
#define PROGRESSMODAL_H

#include "BaseModalContent.h"
#include <QString>

class QLabel;
class QProgressBar;
class QPushButton;

class ProgressModal : public BaseModalContent {
    Q_OBJECT

public:
    explicit ProgressModal(const QString &labelText,
                          const QString &cancelButtonText = QString(),
                          int minimum = 0,
                          int maximum = 100,
                          QWidget *parent = nullptr);

    void setRange(int minimum, int maximum);
    void setValue(int value);
    void setLabelText(const QString &text);
    void setCancelButtonEnabled(bool enabled);

signals:
    void canceled();

private:
    void setupUi(const QString &labelText, const QString &cancelButtonText);

    QLabel *m_label;
    QProgressBar *m_progressBar;
    QPushButton *m_cancelButton;
};

#endif // PROGRESSMODAL_H
