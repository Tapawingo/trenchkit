#ifndef MESSAGEMODAL_H
#define MESSAGEMODAL_H

#include "BaseModalContent.h"
#include <QString>

class QLabel;
class QPushButton;
class ModalManager;

class MessageModal : public BaseModalContent {
    Q_OBJECT

public:
    enum Icon {
        NoIcon,
        Information,
        Warning,
        Critical,
        Question
    };

    enum StandardButton {
        NoButton = 0,
        Ok = 1,
        Cancel = 2,
        Yes = 4,
        No = 8
    };
    Q_DECLARE_FLAGS(StandardButtons, StandardButton)

    explicit MessageModal(const QString &title,
                         const QString &text,
                         Icon icon = Information,
                         StandardButtons buttons = Ok,
                         QWidget *parent = nullptr);

    StandardButton clickedButton() const { return m_clickedButton; }

    static void information(ModalManager *manager, const QString &title, const QString &text);
    static void warning(ModalManager *manager, const QString &title, const QString &text);
    static void critical(ModalManager *manager, const QString &title, const QString &text);
    static bool question(ModalManager *manager, const QString &title, const QString &text);

private:
    void setupUi(const QString &text, Icon icon, StandardButtons buttons);
    QString getIconText(Icon icon);

    QLabel *m_textLabel;
    QLabel *m_iconLabel;
    StandardButton m_clickedButton = NoButton;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MessageModal::StandardButtons)

#endif // MESSAGEMODAL_H
