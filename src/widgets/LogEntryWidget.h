#ifndef LOGENTRYWIDGET_H
#define LOGENTRYWIDGET_H

#include <QWidget>
#include <QString>

class QLabel;

class LogEntryWidget : public QWidget {
    Q_OBJECT

public:
    enum LogLevel {
        Info,
        Success,
        Warning,
        Error
    };

    explicit LogEntryWidget(const QString &message, LogLevel level, QWidget *parent = nullptr);
    ~LogEntryWidget() override = default;

private:
    void setupUi(const QString &message, LogLevel level);
    QString getLevelString(LogLevel level) const;

    QLabel *m_messageLabel;
};

#endif // LOGENTRYWIDGET_H
