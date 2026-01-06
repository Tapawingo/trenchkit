#include "ModalContainer.h"
#include "BaseModalContent.h"
#include "../utils/Theme.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

ModalContainer::ModalContainer(BaseModalContent *content, QWidget *parent)
    : QWidget(parent)
    , m_content(content)
{
    setupLayout();

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 100));
    setGraphicsEffect(shadow);

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

void ModalContainer::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);

    painter.fillPath(path, QColor(Theme::Colors::BACKGROUND_SECONDARY));

    QPen pen(QColor(Theme::Colors::BORDER_PRIMARY));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}
