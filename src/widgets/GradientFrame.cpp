#include "GradientFrame.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QPixmap>
#include <QBrush>

GradientFrame::GradientFrame(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
}

const QPixmap& GradientFrame::getTexture() {
    static QPixmap texture(":/tex_container.png");
    return texture;
}

void GradientFrame::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = this->rect().adjusted(0, 0, 0, 0);

    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(rect, BORDER_RADIUS, BORDER_RADIUS);

    QBrush textureBrush(getTexture());
    painter.fillPath(backgroundPath, textureBrush);

    QRectF borderRect = rect.adjusted(
        BORDER_WIDTH / 2.0,
        BORDER_WIDTH / 2.0,
        -BORDER_WIDTH / 2.0,
        -BORDER_WIDTH / 2.0
    );

    QLinearGradient gradient(
        borderRect.topRight(),
        borderRect.bottomLeft()
    );
    gradient.setColorAt(0.0, QColor(GRADIENT_LIGHT));
    gradient.setColorAt(1.0, QColor(GRADIENT_DARK));

    QPen pen(QBrush(gradient), BORDER_WIDTH);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    QPainterPath borderPath;
    borderPath.addRoundedRect(borderRect, BORDER_RADIUS, BORDER_RADIUS);
    painter.drawPath(borderPath);
}
