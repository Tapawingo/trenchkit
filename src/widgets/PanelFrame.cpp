#include "PanelFrame.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QBrush>

PanelFrame::PanelFrame(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
}

void PanelFrame::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Use contentsRect() which accounts for frame margins/borders
    QRectF contentRect = this->contentsRect();

    QPainterPath backgroundPath;
    backgroundPath.addRect(contentRect);

    QPixmap texture(":/tex_panel.png");
    QBrush textureBrush(texture);
    painter.fillPath(backgroundPath, textureBrush);
}
