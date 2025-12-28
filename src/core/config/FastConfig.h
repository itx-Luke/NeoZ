#ifndef NEOZ_FASTCONFIG_H
#define NEOZ_FASTCONFIG_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QMutex>
#include <QTimer>
#include <QSettings>

namespace NeoZ {

/**
 * @brief High-performance configuration with memory caching.
 * 
 * Features:
 * - All reads from memory (instant)
 * - Writes are cached and flushed asynchronously
 * - Automatic dirty tracking
 * - Thread-safe access
 */
class FastConfig : public QObject
{
    Q_OBJECT
    
public:
    explicit FastConfig(const QString& configPath, QObject* parent = nullptr);
    ~FastConfig();
    
    // Instant read from cache
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;
    
    // Templated getter for convenience
    template<typename T>
    T get(const QString& key, const T& defaultValue = T()) const {
        return get(key, QVariant::fromValue(defaultValue)).template value<T>();
    }
    
    // Write to cache (async flush to disk)
    void set(const QString& key, const QVariant& value);
    
    // Check if key exists
    bool contains(const QString& key) const;
    
    // Remove a key
    void remove(const QString& key);
    
    // Force immediate flush to disk
    void flush();
    
    // Load from disk (refreshes cache)
    void reload();
    
    // Get all keys
    QStringList keys() const;
    
    // Batch operations
    void beginBatch();
    void endBatch();
    
signals:
    void configChanged(const QString& key, const QVariant& value);
    void flushed();
    
private slots:
    void performFlush();
    
private:
    QString m_configPath;
    QHash<QString, QVariant> m_cache;
    mutable QMutex m_mutex;
    QTimer* m_flushTimer;
    bool m_dirty = false;
    bool m_batchMode = false;
    int m_flushDelayMs = 100; // 100ms debounce
};

// ========== GLOBAL ACCESSOR ==========
FastConfig* globalConfig();
void initGlobalConfig(const QString& path);

} // namespace NeoZ

#endif // NEOZ_FASTCONFIG_H
