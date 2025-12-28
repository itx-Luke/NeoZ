#include "Logger.h"
#include <QDebug>
#include <QTextStream>
#include <QDir>

Logger::Logger(QObject* parent)
    : QObject(parent)
{
}

Logger::~Logger()
{
    closeLogFile();
}

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(Level level)
{
    instance().m_minLevel = level;
}

void Logger::setLogFile(const QString& path)
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);

    // Close existing file if open
    if (logger.m_logFile) {
        logger.m_logFile->close();
        delete logger.m_logFile;
    }

    // Ensure directory exists
    QFileInfo fileInfo(path);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    logger.m_logFile = new QFile(path);
    if (!logger.m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "[Logger] Failed to open log file:" << path;
        delete logger.m_logFile;
        logger.m_logFile = nullptr;
    } else {
        // Write header
        QTextStream stream(logger.m_logFile);
        stream << "\n=== Neo-Z Log Session Started: " 
               << QDateTime::currentDateTime().toString(Qt::ISODate) 
               << " ===\n";
    }
}

void Logger::closeLogFile()
{
    Logger& logger = instance();
    QMutexLocker locker(&logger.m_mutex);

    if (logger.m_logFile) {
        QTextStream stream(logger.m_logFile);
        stream << "=== Log Session Ended: " 
               << QDateTime::currentDateTime().toString(Qt::ISODate) 
               << " ===\n";
        logger.m_logFile->close();
        delete logger.m_logFile;
        logger.m_logFile = nullptr;
    }
}

void Logger::debug(const QString& message, const QString& context)
{
    log(Level::Debug, message, context);
}

void Logger::info(const QString& message, const QString& context)
{
    log(Level::Info, message, context);
}

void Logger::warning(const QString& message, const QString& context)
{
    log(Level::Warning, message, context);
}

void Logger::error(const QString& message, const QString& context)
{
    log(Level::Error, message, context);
}

void Logger::critical(const QString& message, const QString& context)
{
    log(Level::Critical, message, context);
}

void Logger::log(Level level, const QString& message, const QString& context)
{
    instance().writeLog(level, message, context);
}

void Logger::writeLog(Level level, const QString& message, const QString& context)
{
    if (level < m_minLevel) {
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString formatted = formatMessage(level, message, context);

    // Output to console via Qt logging
    switch (level) {
        case Debug:
            qDebug().noquote() << formatted;
            break;
        case Info:
            qInfo().noquote() << formatted;
            break;
        case Warning:
            qWarning().noquote() << formatted;
            break;
        case Error:
        case Critical:
            qCritical().noquote() << formatted;
            break;
    }

    // Write to file if enabled
    {
        QMutexLocker locker(&m_mutex);
        if (m_logFile && m_logFile->isOpen()) {
            QTextStream stream(m_logFile);
            stream << timestamp << " " << formatted << "\n";
            stream.flush();
        }
    }

    // Emit signal for UI listeners
    emit logEntry(static_cast<int>(level), timestamp, context, message);
}

QString Logger::levelToString(Level level) const
{
    switch (level) {
        case Debug:    return "DEBUG";
        case Info:     return "INFO ";
        case Warning:  return "WARN ";
        case Error:    return "ERROR";
        case Critical: return "CRIT ";
        default:       return "?????";
    }
}

QString Logger::formatMessage(Level level, const QString& message, const QString& context) const
{
    QString levelStr = levelToString(level);
    
    if (context.isEmpty()) {
        return QString("[%1] %2").arg(levelStr, message);
    }
    
    return QString("[%1] [%2] %3").arg(levelStr, context, message);
}
