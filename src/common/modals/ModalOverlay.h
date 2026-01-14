#ifndef MODALOVERLAY_H
#define MODALOVERLAY_H

#include <QWidget>

class BaseModalContent;
class ModalContainer;
class QPropertyAnimation;

class ModalOverlay : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ getOpacity WRITE setOpacity)

public:
    explicit ModalOverlay(BaseModalContent *content, QWidget *parent = nullptr);
    ~ModalOverlay() override = default;

    BaseModalContent* content() const { return m_content; }

    void show();
    void close();

signals:
    void closed(int result);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void fadeIn();
    void fadeOut();
    void centerContainer();

    qreal getOpacity() const { return m_opacity; }
    void setOpacity(qreal opacity);

    BaseModalContent *m_content;
    ModalContainer *m_container;
    QPropertyAnimation *m_fadeAnimation;
    qreal m_opacity = 0.0;
};

#endif // MODALOVERLAY_H
