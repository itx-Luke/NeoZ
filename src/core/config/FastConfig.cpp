#include "FastConfig.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QSaveFile>
#include <QStandardPaths>

namespace NeoZ {

// ========== GLOBAL INSTANCE ==========
static FastConfig* g_globalConfig = nullptr;
static std::mutex g_globalMutex;

FastConfig* globalConfig()
{
    return g_globalConfig;
}

void initGlobalConfig(const QString& path)
{
    std::lock_guard<std::mutex> lock(g_globalMutex);
    if (!g_globalConfig) {
        g_globalConfig = new FastConfig(path);
        qDebug() << "[FastConfig] V3 initialized at:" << path;
    }
}

void destroyGlobalConfig()
{
    std::lock_guard<std::mutex> lock(g_globalMutex);
    if (g_globalConfig) {
        g_globalConfig->flush();
        delete g_globalConfig;
        g_globalConfig = nullptr;
        qDebug() << "[FastConfig] Destroyed";
    }
}

// ========== BATCH SCOPE ==========
BatchScope::BatchScope(FastConfig* config) : m_config(config)
{
    if (m_config) {
        m_config->beginBatch();
    }
}

BatchScope::~BatchScope()
{
    if (m_config) {
        m_config->endBatch();
    }
}

// ========== FASTCONFIG IMPLEMENTATION ==========

FastConfig::FastConfig(const QString& configPath, QObject* parent)
    : QObject(parent)
    , m_configPath(configPath)
{
    // Initialize flush timer
    m_flushTimer = new QTimer(this);
    m_flushTimer->setSingleShot(true);
    connect(m_flushTimer, &QTimer::timeout, this, &FastConfig::performFlush);
    
    // Create initial empty snapshot
    auto* initial = new ConfigSnapshot();
    m_currentSnapshot.store(initial, std::memory_order_release);
    
    // Load existing config into snapshot
    reload();
    
    qDebug() << "[FastConfig] V3 initialized with" 
             << (m_currentSnapshot.load() ? m_currentSnapshot.load()->data.size() : 0) 
             << "entries";
}

FastConfig::~FastConfig()
{
    // Final flush
    if (m_dirty) {
        performFlush();
    }
    
    // Clean up snapshots
    ConfigSnapshot* current = m_currentSnapshot.exchange(nullptr);
    if (current) {
        delete current;
    }
    
    std::lock_guard<std::mutex> lock(m_snapshotMutex);
    for (auto* snap : m_oldSnapshots) {
        delete snap;
    }
    m_oldSnapshots.clear();
}

// ========== LOCK-FREE READS ==========

QVariant FastConfig::get(const QString& key, const QVariant& defaultValue) const
{
    m_stats.reads.fetch_add(1, std::memory_order_relaxed);
    
    // Lock-free snapshot read
    SnapshotGuard guard(const_cast<std::atomic<ConfigSnapshot*>&>(m_currentSnapshot));
    if (!guard) {
        return defaultValue;
    }
    
    auto it = guard->data.find(key);
    if (it != guard->data.end()) {
        return it->second;
    }
    return defaultValue;
}

int FastConfig::getInt(const QString& key, int defaultValue) const
{
    QVariant val = get(key);
    if (val.isValid() && val.canConvert<int>()) {
        bool ok;
        int result = val.toInt(&ok);
        return ok ? result : defaultValue;
    }
    return defaultValue;
}

bool FastConfig::getBool(const QString& key, bool defaultValue) const
{
    QVariant val = get(key);
    if (val.isValid() && val.canConvert<bool>()) {
        return val.toBool();
    }
    return defaultValue;
}

double FastConfig::getDouble(const QString& key, double defaultValue) const
{
    QVariant val = get(key);
    if (val.isValid() && val.canConvert<double>()) {
        bool ok;
        double result = val.toDouble(&ok);
        return ok ? result : defaultValue;
    }
    return defaultValue;
}

QString FastConfig::getString(const QString& key, const QString& defaultValue) const
{
    QVariant val = get(key);
    if (val.isValid() && val.canConvert<QString>()) {
        return val.toString();
    }
    return defaultValue;
}

bool FastConfig::contains(const QString& key) const
{
    SnapshotGuard guard(const_cast<std::atomic<ConfigSnapshot*>&>(m_currentSnapshot));
    if (!guard) {
        return false;
    }
    return guard->data.find(key) != guard->data.end();
}

QStringList FastConfig::keys() const
{
    SnapshotGuard guard(const_cast<std::atomic<ConfigSnapshot*>&>(m_currentSnapshot));
    QStringList result;
    if (guard) {
        for (const auto& pair : guard->data) {
            result.append(pair.first);
        }
    }
    return result;
}

// ========== WRITES ==========

void FastConfig::set(const QString& key, const QVariant& value)
{
    m_stats.writes.fetch_add(1, std::memory_order_relaxed);
    
    {
        std::lock_guard<std::mutex> lock(m_writeMutex);
        m_pendingWrites[key] = value;
        m_writesSinceSnapshot++;
        m_dirty = true;
        m_stats.pendingWrites.store(m_pendingWrites.size(), std::memory_order_relaxed);
    }
    
    emit configChanged(key, value);
    
    // Create snapshot if threshold reached (not in batch mode)
    if (!m_batchMode && m_writesSinceSnapshot >= m_flushThreshold) {
        createSnapshot();
    }
    
    // Schedule async flush (not in batch mode)
    if (!m_batchMode && !m_flushTimer->isActive()) {
        m_flushTimer->start(m_flushDelayMs);
    }
}

void FastConfig::setInt(const QString& key, int value)
{
    set(key, QVariant(value));
}

void FastConfig::setBool(const QString& key, bool value)
{
    set(key, QVariant(value));
}

void FastConfig::setDouble(const QString& key, double value)
{
    set(key, QVariant(value));
}

void FastConfig::setString(const QString& key, const QString& value)
{
    set(key, QVariant(value));
}

void FastConfig::remove(const QString& key)
{
    {
        std::lock_guard<std::mutex> lock(m_writeMutex);
        // Mark as "removed" by setting null variant
        m_pendingWrites[key] = QVariant();
        m_writesSinceSnapshot++;
        m_dirty = true;
    }
    
    if (!m_batchMode && m_writesSinceSnapshot >= m_flushThreshold) {
        createSnapshot();
    }
    
    if (!m_batchMode && !m_flushTimer->isActive()) {
        m_flushTimer->start(m_flushDelayMs);
    }
}

// ========== BATCH OPERATIONS ==========

void FastConfig::beginBatch()
{
    m_batchMode = true;
    m_flushTimer->stop();
}

void FastConfig::endBatch()
{
    m_batchMode = false;
    
    // Create snapshot with all batched writes
    if (m_writesSinceSnapshot > 0) {
        createSnapshot();
    }
    
    if (m_dirty) {
        m_flushTimer->start(m_flushDelayMs);
    }
}

// ========== SNAPSHOT CREATION ==========

void FastConfig::createSnapshot()
{
    std::lock_guard<std::mutex> writeLock(m_writeMutex);
    
    if (m_pendingWrites.empty()) {
        return;
    }
    
    // Get current snapshot data
    ConfigSnapshot* oldSnapshot = m_currentSnapshot.load(std::memory_order_acquire);
    std::unordered_map<QString, QVariant, QStringHash> newData;
    
    if (oldSnapshot) {
        newData = oldSnapshot->data;
    }
    
    // Apply pending writes
    for (const auto& pair : m_pendingWrites) {
        if (pair.second.isValid()) {
            newData[pair.first] = pair.second;
        } else {
            // Null variant = removal
            newData.erase(pair.first);
        }
    }
    
    // Create new snapshot
    auto* newSnapshot = new ConfigSnapshot(newData);
    
    // Atomically swap
    ConfigSnapshot* prev = m_currentSnapshot.exchange(newSnapshot, std::memory_order_acq_rel);
    
    // Track old snapshot for cleanup
    if (prev) {
        std::lock_guard<std::mutex> snapLock(m_snapshotMutex);
        m_oldSnapshots.push_back(prev);
    }
    
    // Reset counters
    m_pendingWrites.clear();
    m_writesSinceSnapshot = 0;
    m_stats.pendingWrites.store(0, std::memory_order_relaxed);
    m_stats.snapshots.fetch_add(1, std::memory_order_relaxed);
    
    // Cleanup old snapshots periodically
    cleanupOldSnapshots();
}

void FastConfig::cleanupOldSnapshots()
{
    std::lock_guard<std::mutex> lock(m_snapshotMutex);
    
    // Remove snapshots with refCount == 0
    auto it = m_oldSnapshots.begin();
    while (it != m_oldSnapshots.end()) {
        if ((*it)->refCount.load(std::memory_order_acquire) == 0) {
            delete *it;
            it = m_oldSnapshots.erase(it);
        } else {
            ++it;
        }
    }
}

// ========== PERSISTENCE ==========

void FastConfig::flush()
{
    m_flushTimer->stop();
    performFlush();
}

void FastConfig::performFlush()
{
    QElapsedTimer timer;
    timer.start();
    
    // Ensure snapshot is current
    if (m_writesSinceSnapshot > 0) {
        createSnapshot();
    }
    
    if (!m_dirty) {
        return;
    }
    
    // Get snapshot data
    SnapshotGuard guard(m_currentSnapshot);
    if (!guard) {
        return;
    }
    
    // Ensure directory exists
    QDir dir = QFileInfo(m_configPath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Create backup if enabled
    if (m_backupEnabled && QFile::exists(m_configPath)) {
        QString backupPath = m_configPath + ".bak";
        QFile::remove(backupPath);
        QFile::copy(m_configPath, backupPath);
    }
    
    // Write data
    if (m_crashSafeWrites) {
        // Atomic write: write to temp file, then rename
        QString tempPath = m_configPath + ".tmp";
        writeToFile(tempPath, guard->data);
        
        // Atomic rename
        QFile::remove(m_configPath);
        QFile::rename(tempPath, m_configPath);
    } else {
        // Direct write
        writeToFile(m_configPath, guard->data);
    }
    
    m_dirty = false;
    
    // Update stats
    qint64 elapsedUs = timer.nsecsElapsed() / 1000;
    m_stats.flushes.fetch_add(1, std::memory_order_relaxed);
    uint64_t totalFlushes = m_stats.flushes.load();
    uint64_t totalTime = m_stats.totalFlushTimeUs.fetch_add(elapsedUs, std::memory_order_relaxed) + elapsedUs;
    m_stats.avgFlushTimeUs.store(totalTime / totalFlushes, std::memory_order_relaxed);
    
    qDebug() << "[FastConfig] Flushed" << guard->data.size() << "entries in" << elapsedUs << "us";
    
    emit flushed();
}

void FastConfig::writeToFile(const QString& path, const std::unordered_map<QString, QVariant, QStringHash>& data)
{
    QSettings settings(path, QSettings::IniFormat);
    
    // Clear existing
    settings.clear();
    
    // Write all values
    for (const auto& pair : data) {
        settings.setValue(pair.first, pair.second);
    }
    
    settings.sync();
}

void FastConfig::reload()
{
    std::lock_guard<std::mutex> writeLock(m_writeMutex);
    
    std::unordered_map<QString, QVariant, QStringHash> data;
    
    if (QFile::exists(m_configPath)) {
        QSettings settings(m_configPath, QSettings::IniFormat);
        
        for (const QString& key : settings.allKeys()) {
            data[key] = settings.value(key);
        }
    }
    
    // Create new snapshot
    auto* newSnapshot = new ConfigSnapshot(data);
    ConfigSnapshot* prev = m_currentSnapshot.exchange(newSnapshot, std::memory_order_acq_rel);
    
    if (prev) {
        std::lock_guard<std::mutex> snapLock(m_snapshotMutex);
        m_oldSnapshots.push_back(prev);
    }
    
    m_pendingWrites.clear();
    m_writesSinceSnapshot = 0;
    m_dirty = false;
    
    m_stats.snapshots.fetch_add(1, std::memory_order_relaxed);
    
    qDebug() << "[FastConfig] Reloaded" << data.size() << "entries from disk";
}

// ========== CONFIGURATION ==========

void FastConfig::setFlushThreshold(int writeCount)
{
    m_flushThreshold = writeCount;
}

void FastConfig::setFlushDelay(int delayMs)
{
    m_flushDelayMs = delayMs;
}

void FastConfig::setCrashSafeWrites(bool enabled)
{
    m_crashSafeWrites = enabled;
}

void FastConfig::setBackupEnabled(bool enabled)
{
    m_backupEnabled = enabled;
}

// ========== STATISTICS ==========

FastConfigStats FastConfig::getStats() const
{
    return m_stats.toStats();
}

void FastConfig::resetStats()
{
    m_stats.reset();
}

} // namespace NeoZ
