#include "ModalContainer.h"
#include "BaseModalContent.h"
#include "../utils/Theme.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QBrush>
#include <QPixmap>
#include <QResizeEvent>

ModalContainer::ModalContainer(BaseModalContent *content, QWidget *parent)
    : QWidget(parent)
    , m_content(content)
{
    setAttribute(Qt::WA_OpaquePaintEvent);

    setupLayout();

    setMinimumWidth(400);
    setMaximumWidth(800);

    if (content) {
        resize(content->preferredSize());
    }
}

void ModalContainer::setupLayout() {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    if (m_content) {
        m_layout->addWidget(m_content);
    }
}

void ModalContainer::setContent(BaseModalContent *content) {
    if (m_content) {
        m_layout->removeWidget(m_content);
    }

    m_content = content;

    if (m_content) {
        m_layout->addWidget(m_content);
        resize(m_content->preferredSize());
    }
}

const QPixmap& ModalContainer::getTexture() {
    static QPixmap texture(":/tex_container.png");
    return texture;
}

void ModalContainer::updateBackgroundCache() {
    if (size() == m_lastSize && !m_cachedBackground.isNull()) {
        return;
    }

    m_lastSize = size();
    m_cachedBackground = QPixmap(size());
    m_cachedBackground.fill(Qt::transparent);

    QPainter painter(&m_cachedBackground);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = QRectF(0, 0, width(), height());

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

void ModalContainer::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    updateBackgroundCache();

    QPainter painter(this);
    painter.drawPixmap(0, 0, m_cachedBackground);
}

void ModalContainer::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    m_lastSize = QSize(-1, -1);
}
