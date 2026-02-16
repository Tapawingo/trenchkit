#ifndef ACTIVITYLOGWIDGET_H
#define ACTIVITYLOGWIDGET_H

#include <QWidget>
#include <QString>
#include <QEvent>
#include "LogEntryWidget.h"

class QLabel;
class QListWidget;

class ActivityLogWidget : public QWidget {
    Q_OBJECT

public:
    using LogLevel = LogEntryWidget::LogLevel;

    explicit ActivityLogWidget(QWidget *parent = nullptr);
    ~ActivityLogWidget() override = default;

    void addLogEntry(const QString &message, LogLevel level = LogLevel::Info);
    void clearLog();

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void retranslateUi();

    static constexpr int MAX_LOG_ENTRIES = 1000;

    QLabel *m_titleLabel = nullptr;
    QListWidget *m_logList;
};

#endif // ACTIVITYLOGWIDGET_H
