#include "ActivityLogWidget.h"
#include "common/widgets/GradientFrame.h"
#include "LogEntryWidget.h"
#include "core/utils/Theme.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>

ActivityLogWidget::ActivityLogWidget(QWidget *parent)
    : QWidget(parent)
    , m_logList(new QListWidget(this))
{
    setupUi();
}

void ActivityLogWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN
    );
    layout->setSpacing(Theme::Spacing::CONTAINER_SPACING);

    GradientFrame *frame = new GradientFrame(this);
    layout->addWidget(frame);

    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("activityLogTitle");
    frameLayout->addWidget(m_titleLabel);

    m_logList->setSpacing(Theme::Spacing::LOG_LIST_ITEM_SPACING);
    m_logList->setSelectionMode(QAbstractItemView::NoSelection);
    m_logList->setFocusPolicy(Qt::NoFocus);

    frameLayout->addWidget(m_logList);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    setLayout(layout);

    retranslateUi();
}

void ActivityLogWidget::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void ActivityLogWidget::retranslateUi() {
    m_titleLabel->setText(tr("Logs"));
}

void ActivityLogWidget::addLogEntry(const QString &message, LogLevel level) {
    if (m_logList->count() >= MAX_LOG_ENTRIES) {
        delete m_logList->takeItem(0);
    }

    auto *entryWidget = new LogEntryWidget(message, level, this);

    auto *item = new QListWidgetItem(m_logList);
    item->setSizeHint(entryWidget->sizeHint());

    m_logList->addItem(item);
    m_logList->setItemWidget(item, entryWidget);

    m_logList->scrollToBottom();
}

void ActivityLogWidget::clearLog() {
    m_logList->clear();
}
