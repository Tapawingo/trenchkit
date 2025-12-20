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

const QPixmap& PanelFrame::getTexture() {
    static QPixmap texture(":/tex_panel.png");
    return texture;
}

void PanelFrame::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Use contentsRect() which accounts for frame margins/borders
    QRectF contentRect = this->contentsRect();

    QPainterPath backgroundPath;
    backgroundPath.addRect(contentRect);

    QBrush textureBrush(getTexture());
    painter.fillPath(backgroundPath, textureBrush);
}
