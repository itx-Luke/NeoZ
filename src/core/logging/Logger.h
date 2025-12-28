#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QMutex>
#include <QDateTime>

/**
 * @brief Structured logging system for Neo-Z
 * 
 * Provides leveled logging with optional file output and context tagging.
 * Thread-safe singleton implementation.
 * 
 * Usage:
 *   Logger::info("Device connected", "ADB");
 *   Logger::warning("Connection timeout", "Emulator");
 *   Logger::error("Failed to set DPI", "Logitech");
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    enum Level {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
        Critical = 4
    };
    Q_ENUM(Level)

    /**
     * @brief Get the singleton instance
     */
    static Logger& instance();

    /**
     * @brief Set minimum log level (messages below this are ignored)
     */
    static void setLogLevel(Level level);

    /**
     * @brief Enable file logging
     * @param path Path to log file (created if doesn't exist)
     */
    static void setLogFile(const QString& path);

    /**
     * @brief Close log file and disable file logging
     */
    static void closeLogFile();

    // Convenience static methods
    static void debug(const QString& message, const QString& context = QString());
    static void info(const QString& message, const QString& context = QString());
    static void warning(const QString& message, const QString& context = QString());
    static void error(const QString& message, const QString& context = QString());
    static void critical(const QString& message, const QString& context = QString());

    /**
     * @brief Log a message with specified level
     */
    static void log(Level level, const QString& message, const QString& context = QString());

signals:
    /**
     * @brief Emitted when a new log entry is created
     * Useful for displaying logs in UI
     */
    void logEntry(int level, const QString& timestamp, const QString& context, const QString& message);

private:
    explicit Logger(QObject* parent = nullptr);
    ~Logger();

    void writeLog(Level level, const QString& message, const QString& context);
    QString levelToString(Level level) const;
    QString formatMessage(Level level, const QString& message, const QString& context) const;

    Level m_minLevel = Level::Debug;
    QFile* m_logFile = nullptr;
    QMutex m_mutex;

    // Singleton prevention
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

// Convenience macros for easy logging with automatic context
#define LOG_DEBUG(msg) Logger::debug(msg, __FUNCTION__)
#define LOG_INFO(msg) Logger::info(msg, __FUNCTION__)
#define LOG_WARNING(msg) Logger::warning(msg, __FUNCTION__)
#define LOG_ERROR(msg) Logger::error(msg, __FUNCTION__)
#define LOG_CRITICAL(msg) Logger::critical(msg, __FUNCTION__)

#endif // LOGGER_H
