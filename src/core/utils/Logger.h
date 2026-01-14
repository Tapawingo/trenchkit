#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QString>
#include <QDateTime>

class Logger : public QObject {
    Q_OBJECT

public:
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Critical
    };

    static Logger& instance();

    void initialize();
    void shutdown();

    QString logDirectory() const;
    QString currentLogFile() const;

    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    Logger();
    ~Logger() override;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void writeToFile(const QString &message);
    void rotateIfNeeded();
    void cleanOldLogs();
    QString formatMessage(QtMsgType type, const QString &msg, const QMessageLogContext &context);
    QString logLevelToString(QtMsgType type);

    QMutex m_mutex;
    QFile m_logFile;
    QTextStream m_stream;
    QString m_logDir;
    qint64 m_maxFileSize = 10 * 1024 * 1024;
    int m_maxDays = 7;
};

#endif // LOGGER_H
