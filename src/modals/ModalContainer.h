#ifndef MODALCONTAINER_H
#define MODALCONTAINER_H

#include <QWidget>

class BaseModalContent;
class QVBoxLayout;

class ModalContainer : public QWidget {
    Q_OBJECT

public:
    explicit ModalContainer(BaseModalContent *content, QWidget *parent = nullptr);
    ~ModalContainer() override = default;

    void setContent(BaseModalContent *content);
    BaseModalContent* content() const { return m_content; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupLayout();

    BaseModalContent *m_content;
    QVBoxLayout *m_layout;
};

#endif // MODALCONTAINER_H
