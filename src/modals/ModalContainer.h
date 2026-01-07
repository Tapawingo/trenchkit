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
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupLayout();
    void updateBackgroundCache();
    static const QPixmap& getTexture();

    BaseModalContent *m_content;
    QVBoxLayout *m_layout;

    QPixmap m_cachedBackground;
    QSize m_lastSize;

    static constexpr const char* GRADIENT_LIGHT = "#141413";
    static constexpr const char* GRADIENT_DARK = "#141413";
    static constexpr int BORDER_WIDTH = 3;
    static constexpr int BORDER_RADIUS = 0;
};

#endif // MODALCONTAINER_H
