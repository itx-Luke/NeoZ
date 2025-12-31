#include "AdbConnection.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QDateTime>

namespace NeoZ {

AdbConnection::AdbConnection(QObject* parent)
    : QObject(parent)
{
    // Try to find ADB in common locations
    m_adbPath = "adb"; // Default to PATH
}

AdbConnection::~AdbConnection()
{
    disconnect();
    if (m_asyncProcess) {
        m_asyncProcess->kill();
        // unique_ptr handles deletion automatically
    }
}

bool AdbConnection::connect(const QString& deviceId)
{
    if (deviceId.isEmpty()) {
        qWarning() << "[AdbConnection] Cannot connect: empty device ID";
        return false;
    }
    
    // Test connection with a simple command
    QProcess proc;
    QStringList args;
    args << "-s" << deviceId << "shell" << "echo" << "connected";
    
    QElapsedTimer timer;
    timer.start();
    
    proc.start(m_adbPath, args);
    if (!proc.waitForFinished(3000)) {
        qWarning() << "[AdbConnection] Connection timeout for device:" << deviceId;
        return false;
    }
    
    m_latencyMs = timer.elapsed();
    
    if (proc.exitCode() == 0) {
        m_deviceId = deviceId;
        m_connected = true;
        qDebug() << "[AdbConnection] Connected to" << deviceId << "| Latency:" << m_latencyMs << "ms";
        emit connectionChanged();
        emit latencyChanged();
        return true;
    }
    
    qWarning() << "[AdbConnection] Failed to connect:" << proc.readAllStandardError();
    return false;
}

void AdbConnection::disconnect()
{
    if (m_connected) {
        m_connected = false;
        m_deviceId.clear();
        invalidateCache();
        emit connectionChanged();
        qDebug() << "[AdbConnection] Disconnected";
    }
}

QString AdbConnection::execute(const QString& command, int timeoutMs)
{
    if (!m_connected) {
        qWarning() << "[AdbConnection] Not connected";
        return QString();
    }
    
    QProcess proc;
    QStringList args;
    args << "-s" << m_deviceId << "shell" << command;
    
    QElapsedTimer timer;
    timer.start();
    
    proc.start(m_adbPath, args);
    if (!proc.waitForFinished(timeoutMs)) {
        qWarning() << "[AdbConnection] Command timeout:" << command;
        proc.kill();
        return QString();
    }
    
    m_latencyMs = timer.elapsed();
    emit latencyChanged();
    
    QString result = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    emit commandCompleted(command, result);
    return result;
}

void AdbConnection::executeAsync(const QString& command, 
                                  std::function<void(const QString&)> callback)
{
    AsyncCommand cmd;
    cmd.command = command;
    cmd.callback = callback;
    m_asyncQueue.enqueue(cmd);
    
    if (!m_asyncBusy) {
        processAsyncQueue();
    }
}

void AdbConnection::processAsyncQueue()
{
    if (m_asyncQueue.isEmpty() || !m_connected) {
        m_asyncBusy = false;
        return;
    }
    
    m_asyncBusy = true;
    AsyncCommand cmd = m_asyncQueue.dequeue();
    
    if (!m_asyncProcess) {
        m_asyncProcess = std::make_unique<QProcess>(this);
        QObject::connect(m_asyncProcess.get(), 
                        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        this, &AdbConnection::onProcessFinished);
    }
    
    QStringList args;
    args << "-s" << m_deviceId << "shell" << cmd.command;
    
    m_asyncProcess->setProperty("command", cmd.command);
    m_asyncProcess->setProperty("callback", QVariant::fromValue(cmd.callback));
    m_asyncProcess->start(m_adbPath, args);
}

void AdbConnection::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(status)
    
    QString command = m_asyncProcess->property("command").toString();
    auto callback = m_asyncProcess->property("callback").value<std::function<void(const QString&)>>();
    
    QString result;
    if (exitCode == 0) {
        result = QString::fromUtf8(m_asyncProcess->readAllStandardOutput()).trimmed();
        emit commandCompleted(command, result);
    } else {
        QString error = QString::fromUtf8(m_asyncProcess->readAllStandardError()).trimmed();
        emit commandError(command, error);
    }
    
    if (callback) {
        callback(result);
    }
    
    // Process next command in queue
    processAsyncQueue();
}

// ========== BATCH EXECUTION ==========
// This is the KEY optimization - runs multiple commands in a single shell session
AdbConnection::BatchResult AdbConnection::executeBatch(const QStringList& commands, int timeoutMs)
{
    BatchResult result;
    result.commands = commands;
    result.success = false;
    
    if (!m_connected || commands.isEmpty()) {
        return result;
    }
    
    // Build a single shell command that runs all commands with separators
    // Format: cmd1; echo SEP; cmd2; echo SEP; cmd3
    QString batchCommand;
    for (int i = 0; i < commands.size(); ++i) {
        if (i > 0) {
            batchCommand += "; echo '" + QString(BATCH_SEPARATOR) + "'; ";
        }
        batchCommand += commands[i];
    }
    
    QProcess proc;
    QStringList args;
    args << "-s" << m_deviceId << "shell" << batchCommand;
    
    QElapsedTimer timer;
    timer.start();
    
    proc.start(m_adbPath, args);
    if (!proc.waitForFinished(timeoutMs)) {
        qWarning() << "[AdbConnection] Batch timeout";
        proc.kill();
        return result;
    }
    
    result.totalTimeMs = timer.elapsed();
    m_latencyMs = result.totalTimeMs / commands.size(); // Average latency
    emit latencyChanged();
    
    // Parse results by separator
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    result.results = output.split(BATCH_SEPARATOR, Qt::KeepEmptyParts);
    
    // Trim each result
    for (QString& r : result.results) {
        r = r.trimmed();
    }
    
    result.success = (proc.exitCode() == 0);
    
    qDebug() << "[AdbConnection] Batch executed:" << commands.size() << "commands in" 
             << result.totalTimeMs << "ms (avg:" << m_latencyMs << "ms/cmd)";
    
    return result;
}

// ========== CACHING ==========
bool AdbConnection::CacheEntry::isValid() const
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    return (now - timestamp) < ttlMs;
}

QString AdbConnection::getCached(const QString& command, int ttlMs)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (m_cache.contains(command)) {
        CacheEntry& entry = m_cache[command];
        if (entry.isValid()) {
            return entry.value;
        }
    }
    
    // Cache miss - execute and store
    locker.unlock();
    QString result = execute(command);
    
    locker.relock();
    CacheEntry entry;
    entry.value = result;
    entry.timestamp = QDateTime::currentMSecsSinceEpoch();
    entry.ttlMs = ttlMs;
    m_cache[command] = entry;
    
    return result;
}

void AdbConnection::invalidateCache(const QString& command)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (command.isEmpty()) {
        m_cache.clear();
        qDebug() << "[AdbConnection] Cache cleared";
    } else {
        m_cache.remove(command);
    }
}

// ========== FREE FIRE SHORTCUTS ==========
QString AdbConnection::getScreenSize()
{
    return getCached("wm size", 5000); // Cache for 5 seconds (rarely changes)
}

QString AdbConnection::getDensity()
{
    return getCached("wm density", 5000);
}

bool AdbConnection::isFreeFireRunning()
{
    QString pid = getCached("pidof com.dts.freefireth", 500); // Cache for 500ms
    return !pid.isEmpty();
}

QString AdbConnection::getCurrentFocus()
{
    return getCached("dumpsys window displays | grep mCurrentFocus", 200);
}

} // namespace NeoZ
