#include "Logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

Logger::Logger() {
    m_logDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs";
}

Logger::~Logger() {
    shutdown();
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::initialize() {
    QMutexLocker locker(&m_mutex);

    QDir dir;
    if (!dir.mkpath(m_logDir)) {
        qWarning() << "Failed to create log directory:" << m_logDir;
        return;
    }

    QString logFileName = QString("trenchkit_%1.log")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_HHmmss"));
    QString logPath = m_logDir + "/" + logFileName;

    m_logFile.setFileName(logPath);
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << logPath;
        return;
    }

    m_stream.setDevice(&m_logFile);

    cleanOldLogs();

    QString separator = QString(80, '=');
    m_stream << separator << "\n";
    m_stream << "TrenchKit started at " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    m_stream << separator << "\n";
    m_stream.flush();
}

void Logger::shutdown() {
    QMutexLocker locker(&m_mutex);

    if (m_logFile.isOpen()) {
        m_stream << "\n" << QString(80, '=') << "\n";
        m_stream << "TrenchKit shutdown at " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
        m_stream << QString(80, '=') << "\n";
        m_stream.flush();
        m_logFile.close();
    }
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Logger::instance().writeToFile(Logger::instance().formatMessage(type, msg, context));
}

void Logger::writeToFile(const QString &message) {
    QMutexLocker locker(&m_mutex);

    if (!m_logFile.isOpen()) {
        return;
    }

    rotateIfNeeded();

    m_stream << message << "\n";
    m_stream.flush();
}

void Logger::rotateIfNeeded() {
    if (m_logFile.size() >= m_maxFileSize) {
        m_logFile.close();

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HHmmss");
        QString rotatedName = QString("trenchkit_%1.log").arg(timestamp);
        QString rotatedPath = m_logDir + "/" + rotatedName;

        QFile::rename(m_logFile.fileName(), rotatedPath);

        QString logFileName = QString("trenchkit_%1.log")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
        m_logFile.setFileName(m_logDir + "/" + logFileName);
        if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            qWarning() << "Failed to open rotated log file:" << m_logFile.fileName();
            return;
        }
        m_stream.setDevice(&m_logFile);
    }
}

void Logger::cleanOldLogs() {
    QDir logDir(m_logDir);
    QFileInfoList logFiles = logDir.entryInfoList(QStringList() << "*.log", QDir::Files, QDir::Time);

    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-m_maxDays);

    for (const QFileInfo &fileInfo : logFiles) {
        if (fileInfo.lastModified() < cutoffDate) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

QString Logger::formatMessage(QtMsgType type, const QString &msg, const QMessageLogContext &context) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString level = logLevelToString(type);

    QString formattedMsg = QString("[%1] [%2] %3")
        .arg(timestamp)
        .arg(level)
        .arg(msg);

    if (context.file && type >= QtWarningMsg) {
        formattedMsg += QString(" (%1:%2)").arg(context.file).arg(context.line);
    }

    return formattedMsg;
}

QString Logger::logLevelToString(QtMsgType type) {
    switch (type) {
        case QtDebugMsg:    return "DEBUG";
        case QtInfoMsg:     return "INFO ";
        case QtWarningMsg:  return "WARN ";
        case QtCriticalMsg: return "ERROR";
        case QtFatalMsg:    return "FATAL";
        default:            return "UNKNOWN";
    }
}

QString Logger::logDirectory() const {
    return m_logDir;
}

QString Logger::currentLogFile() const {
    return m_logFile.fileName();
}
