#ifndef NEOZ_FASTCONFIG_H
#define NEOZ_FASTCONFIG_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <thread>
#include <vector>

namespace NeoZ {

// ========== HASH SUPPORT FOR QString ==========
struct QStringHash {
    std::size_t operator()(const QString& s) const noexcept {
        return qHash(s);
    }
};

// ========== STATISTICS (Copyable return type) ==========
struct FastConfigStats {
    uint64_t reads = 0;
    uint64_t writes = 0;
    uint64_t snapshots = 0;
    uint64_t flushes = 0;
    uint64_t pendingWrites = 0;
    uint64_t avgFlushTimeUs = 0;
    uint64_t totalFlushTimeUs = 0;
};

// ========== INTERNAL ATOMIC STATS (Not copyable) ==========
struct FastConfigStatsInternal {
    std::atomic<uint64_t> reads{0};
    std::atomic<uint64_t> writes{0};
    std::atomic<uint64_t> snapshots{0};
    std::atomic<uint64_t> flushes{0};
    std::atomic<uint64_t> pendingWrites{0};
    std::atomic<uint64_t> avgFlushTimeUs{0};
    std::atomic<uint64_t> totalFlushTimeUs{0};
    
    FastConfigStats toStats() const {
        FastConfigStats result;
        result.reads = reads.load(std::memory_order_relaxed);
        result.writes = writes.load(std::memory_order_relaxed);
        result.snapshots = snapshots.load(std::memory_order_relaxed);
        result.flushes = flushes.load(std::memory_order_relaxed);
        result.pendingWrites = pendingWrites.load(std::memory_order_relaxed);
        result.avgFlushTimeUs = avgFlushTimeUs.load(std::memory_order_relaxed);
        result.totalFlushTimeUs = totalFlushTimeUs.load(std::memory_order_relaxed);
        return result;
    }
    
    void reset() {
        reads.store(0, std::memory_order_relaxed);
        writes.store(0, std::memory_order_relaxed);
        snapshots.store(0, std::memory_order_relaxed);
        flushes.store(0, std::memory_order_relaxed);
        pendingWrites.store(0, std::memory_order_relaxed);
        avgFlushTimeUs.store(0, std::memory_order_relaxed);
        totalFlushTimeUs.store(0, std::memory_order_relaxed);
    }
};

// ========== SNAPSHOT (Immutable, Lock-Free Reads) ==========
struct ConfigSnapshot {
    std::unordered_map<QString, QVariant, QStringHash> data;
    std::atomic<int> refCount{0};
    
    ConfigSnapshot() = default;
    ConfigSnapshot(const std::unordered_map<QString, QVariant, QStringHash>& d) : data(d) {}
    
    void addRef() { refCount.fetch_add(1, std::memory_order_relaxed); }
    void release() { 
        if (refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // Could schedule for deletion, but we let parent manage
        }
    }
};

// ========== RAII SNAPSHOT GUARD (Hazard Pointer Pattern) ==========
class SnapshotGuard {
public:
    SnapshotGuard(std::atomic<ConfigSnapshot*>& snapshotPtr) 
        : m_snapshot(snapshotPtr.load(std::memory_order_acquire))
    {
        if (m_snapshot) {
            m_snapshot->addRef();
        }
    }
    
    ~SnapshotGuard() {
        if (m_snapshot) {
            m_snapshot->release();
        }
    }
    
    // Non-copyable, movable
    SnapshotGuard(const SnapshotGuard&) = delete;
    SnapshotGuard& operator=(const SnapshotGuard&) = delete;
    
    SnapshotGuard(SnapshotGuard&& other) noexcept : m_snapshot(other.m_snapshot) {
        other.m_snapshot = nullptr;
    }
    
    SnapshotGuard& operator=(SnapshotGuard&& other) noexcept {
        if (this != &other) {
            if (m_snapshot) m_snapshot->release();
            m_snapshot = other.m_snapshot;
            other.m_snapshot = nullptr;
        }
        return *this;
    }
    
    ConfigSnapshot* get() const { return m_snapshot; }
    ConfigSnapshot* operator->() const { return m_snapshot; }
    explicit operator bool() const { return m_snapshot != nullptr; }
    
private:
    ConfigSnapshot* m_snapshot;
};

// ========== BATCH SCOPE RAII ==========
class FastConfig;
class BatchScope {
public:
    explicit BatchScope(FastConfig* config);
    ~BatchScope();
    
    BatchScope(const BatchScope&) = delete;
    BatchScope& operator=(const BatchScope&) = delete;
    
private:
    FastConfig* m_config;
};

/**
 * @brief FastConfig V3 - High-Performance Lock-Free Configuration
 * 
 * Features:
 * - O(1) lock-free reads via atomic snapshot pointer
 * - Hazard pointer pattern for safe memory reclamation
 * - Crash-safe atomic writes (write temp + rename)
 * - Type-safe accessors (no split-brain)
 * - Batch write coalescing
 * - Statistics and monitoring
 */
class FastConfig : public QObject
{
    Q_OBJECT
    
public:
    explicit FastConfig(const QString& configPath, QObject* parent = nullptr);
    ~FastConfig();
    
    // ========== LOCK-FREE READS (O(1)) ==========
    
    // Generic read - returns QVariant
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;
    
    // Templated getter for backwards compatibility
    template<typename T>
    T get(const QString& key, const T& defaultValue = T()) const {
        return get(key, QVariant::fromValue(defaultValue)).template value<T>();
    }
    
    // Type-safe reads (returns default if type mismatch - no split-brain!)
    int getInt(const QString& key, int defaultValue = 0) const;
    bool getBool(const QString& key, bool defaultValue = false) const;
    double getDouble(const QString& key, double defaultValue = 0.0) const;
    QString getString(const QString& key, const QString& defaultValue = QString()) const;
    
    // Check if key exists
    bool contains(const QString& key) const;
    
    // Get all keys
    QStringList keys() const;
    
    // ========== WRITES (Mutex-Protected, Batched) ==========
    
    // Generic write
    void set(const QString& key, const QVariant& value);
    
    // Type-safe writes
    void setInt(const QString& key, int value);
    void setBool(const QString& key, bool value);
    void setDouble(const QString& key, double value);
    void setString(const QString& key, const QString& value);
    
    // Remove a key
    void remove(const QString& key);
    
    // ========== BATCH OPERATIONS ==========
    void beginBatch();
    void endBatch();
    
    // ========== PERSISTENCE ==========
    void flush();           // Force immediate flush to disk
    void reload();          // Reload from disk
    
    // ========== CONFIGURATION ==========
    void setFlushThreshold(int writeCount);     // Auto-snapshot after N writes
    void setFlushDelay(int delayMs);            // Delay before flushing to disk
    void setCrashSafeWrites(bool enabled);      // Atomic write (temp + rename)
    void setBackupEnabled(bool enabled);        // Create .bak file
    
    // ========== STATISTICS ==========
    FastConfigStats getStats() const;
    void resetStats();
    
signals:
    void configChanged(const QString& key, const QVariant& value);
    void flushed();
    
private slots:
    void performFlush();
    
private:
    void createSnapshot();
    void writeToFile(const QString& path, const std::unordered_map<QString, QVariant, QStringHash>& data);
    void cleanupOldSnapshots();
    
    QString m_configPath;
    
    // Lock-free snapshot for reads
    std::atomic<ConfigSnapshot*> m_currentSnapshot{nullptr};
    std::vector<ConfigSnapshot*> m_oldSnapshots;  // For cleanup
    std::mutex m_snapshotMutex;
    
    // Pending writes (mutex-protected)
    std::mutex m_writeMutex;
    std::unordered_map<QString, QVariant, QStringHash> m_pendingWrites;
    int m_writesSinceSnapshot = 0;
    
    // Timers and thresholds
    QTimer* m_flushTimer = nullptr;
    int m_flushDelayMs = 500;
    int m_flushThreshold = 100;
    
    // Options
    bool m_crashSafeWrites = false;
    bool m_backupEnabled = false;
    bool m_batchMode = false;
    bool m_dirty = false;
    
    // Statistics (internal atomic)
    mutable FastConfigStatsInternal m_stats;
};

// ========== GLOBAL ACCESSOR ==========
FastConfig* globalConfig();
void initGlobalConfig(const QString& path);
void destroyGlobalConfig();

// ========== CONVENIENCE MACROS ==========

// Type-safe getters
#define CONFIG_GET(key, def) NeoZ::globalConfig()->get(key, def)
#define CONFIG_GET_INT(key, def) NeoZ::globalConfig()->getInt(key, def)
#define CONFIG_GET_BOOL(key, def) NeoZ::globalConfig()->getBool(key, def)
#define CONFIG_GET_DOUBLE(key, def) NeoZ::globalConfig()->getDouble(key, def)
#define CONFIG_GET_STRING(key, def) NeoZ::globalConfig()->getString(key, def)

// Type-safe setters
#define CONFIG_SET(key, val) NeoZ::globalConfig()->set(key, val)
#define CONFIG_SET_INT(key, val) NeoZ::globalConfig()->setInt(key, val)
#define CONFIG_SET_BOOL(key, val) NeoZ::globalConfig()->setBool(key, val)
#define CONFIG_SET_DOUBLE(key, val) NeoZ::globalConfig()->setDouble(key, val)
#define CONFIG_SET_STRING(key, val) NeoZ::globalConfig()->setString(key, val)

// Batch scope
#define CONFIG_BATCH_SCOPE() NeoZ::BatchScope __batch_scope(NeoZ::globalConfig())

} // namespace NeoZ

#endif // NEOZ_FASTCONFIG_H
