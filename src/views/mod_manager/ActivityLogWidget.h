#ifndef ACTIVITYLOGWIDGET_H
#define ACTIVITYLOGWIDGET_H

#include <QWidget>
#include <QString>
#include "LogEntryWidget.h"

class QListWidget;

class ActivityLogWidget : public QWidget {
    Q_OBJECT

public:
    using LogLevel = LogEntryWidget::LogLevel;

    explicit ActivityLogWidget(QWidget *parent = nullptr);
    ~ActivityLogWidget() override = default;

    void addLogEntry(const QString &message, LogLevel level = LogLevel::Info);
    void clearLog();

private:
    void setupUi();

    static constexpr int MAX_LOG_ENTRIES = 1000;

    QListWidget *m_logList;
};

#endif // ACTIVITYLOGWIDGET_H
