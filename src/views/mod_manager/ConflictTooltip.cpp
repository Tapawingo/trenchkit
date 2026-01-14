#include "ConflictTooltip.h"
#include <QLabel>
#include <QVBoxLayout>

ConflictTooltip::ConflictTooltip(QWidget *parent)
    : GradientFrame(parent)
{
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setObjectName("conflictTooltip");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);

    m_label = new QLabel(this);
    m_label->setObjectName("conflictTooltipLabel");
    m_label->setTextFormat(Qt::RichText);
    m_label->setWordWrap(true);
    m_label->setTextInteractionFlags(Qt::NoTextInteraction);
    m_label->setMaximumWidth(360);

    layout->addWidget(m_label);
}

void ConflictTooltip::setText(const QString &text) {
    m_label->setText(text);
    adjustSize();
}

void ConflictTooltip::showAt(const QPoint &globalPos) {
    adjustSize();
    move(globalPos);
    show();
    raise();
}
