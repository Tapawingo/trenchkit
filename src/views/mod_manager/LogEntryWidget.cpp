#include "LogEntryWidget.h"
#include "core/utils/Theme.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDateTime>
#include <QStyle>

LogEntryWidget::LogEntryWidget(const QString &message, LogLevel level, QWidget *parent)
    : QWidget(parent)
    , m_messageLabel(new QLabel(this))
{
    setupUi(message, level);
}

void LogEntryWidget::setupUi(const QString &message, LogLevel level) {
    setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(
        Theme::Spacing::LOG_ENTRY_PADDING_HORIZONTAL,
        Theme::Spacing::LOG_ENTRY_PADDING_VERTICAL,
        Theme::Spacing::LOG_ENTRY_PADDING_HORIZONTAL,
        Theme::Spacing::LOG_ENTRY_PADDING_VERTICAL
    );
    layout->setSpacing(0);

    m_messageLabel->setText(message);
    layout->addWidget(m_messageLabel);

    setProperty("level", getLevelString(level));

    style()->unpolish(this);
    style()->polish(this);

    setLayout(layout);
}

QString LogEntryWidget::getLevelString(LogLevel level) const {
    switch (level) {
        case Info:    return "info";
        case Success: return "success";
        case Warning: return "warning";
        case Error:   return "error";
        default:      return "info";
    }
}
