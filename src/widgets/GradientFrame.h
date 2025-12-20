#ifndef GRADIENTFRAME_H
#define GRADIENTFRAME_H

#include <QFrame>

class GradientFrame : public QFrame {
    Q_OBJECT

public:
    explicit GradientFrame(QWidget *parent = nullptr);
    ~GradientFrame() override = default;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    static constexpr const char* GRADIENT_LIGHT = "#141413";
    static constexpr const char* GRADIENT_DARK = "#141413";
    static constexpr const char* BACKGROUND_COLOR = "#16130e";
    static constexpr int BORDER_WIDTH = 3;
    static constexpr int BORDER_RADIUS = 0;
};

#endif // GRADIENTFRAME_H
