#ifndef NEOZ_ADBCONNECTION_H
#define NEOZ_ADBCONNECTION_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QHash>
#include <QMutex>
#include <QQueue>
#include <functional>

namespace NeoZ {

/**
 * @brief High-performance ADB connection with command batching.
 * 
 * Features:
 * - Command batching: Combines multiple commands into single shell session
 * - Caching: Short-lived cache for frequently polled values
 * - Async execution: Non-blocking command execution
 * - Connection pooling: Reuses ADB connection when possible
 */
class AdbConnection : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString deviceId READ deviceId NOTIFY connectionChanged)
    Q_PROPERTY(int latencyMs READ latencyMs NOTIFY latencyChanged)
    
public:
    explicit AdbConnection(QObject* parent = nullptr);
    ~AdbConnection();
    
    // Connection management
    bool connect(const QString& deviceId);
    void disconnect();
    bool isConnected() const { return m_connected; }
    QString deviceId() const { return m_deviceId; }
    int latencyMs() const { return m_latencyMs; }
    
    // Set ADB path
    void setAdbPath(const QString& path) { m_adbPath = path; }
    QString adbPath() const { return m_adbPath; }
    
    // Synchronous execution (blocking)
    QString execute(const QString& command, int timeoutMs = 5000);
    
    // Asynchronous execution
    void executeAsync(const QString& command, 
                      std::function<void(const QString&)> callback = nullptr);
    
    // Batch execution - runs multiple commands in single shell session
    struct BatchResult {
        QStringList commands;
        QStringList results;
        bool success;
        int totalTimeMs;
    };
    BatchResult executeBatch(const QStringList& commands, int timeoutMs = 10000);
    
    // Cached getters - uses internal cache with TTL
    QString getCached(const QString& command, int ttlMs = 500);
    void invalidateCache(const QString& command = QString());
    
    // Free Fire specific shortcuts
    QString getScreenSize();
    QString getDensity();
    bool isFreeFireRunning();
    QString getCurrentFocus();
    
signals:
    void connectionChanged();
    void latencyChanged();
    void commandCompleted(const QString& command, const QString& result);
    void commandError(const QString& command, const QString& error);
    
private slots:
    void processAsyncQueue();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    
private:
    struct CacheEntry {
        QString value;
        qint64 timestamp;
        int ttlMs;
        
        bool isValid() const;
    };
    
    struct AsyncCommand {
        QString command;
        std::function<void(const QString&)> callback;
    };
    
    QString m_adbPath;
    QString m_deviceId;
    bool m_connected = false;
    int m_latencyMs = 0;
    
    // Command cache
    QHash<QString, CacheEntry> m_cache;
    QMutex m_cacheMutex;
    
    // Async queue
    QQueue<AsyncCommand> m_asyncQueue;
    QProcess* m_asyncProcess = nullptr;
    bool m_asyncBusy = false;
    
    // Batch separator for combining commands
    static constexpr const char* BATCH_SEPARATOR = "---NEOZ_BATCH_SEP---";
};

} // namespace NeoZ

#endif // NEOZ_ADBCONNECTION_H
