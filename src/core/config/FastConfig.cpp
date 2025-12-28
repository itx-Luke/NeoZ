#include "FastConfig.h"
#include <QDebug>
#include <QFile>
#include <QDir>

namespace NeoZ {

// Global instance
static FastConfig* g_globalConfig = nullptr;

FastConfig* globalConfig()
{
    return g_globalConfig;
}

void initGlobalConfig(const QString& path)
{
    if (!g_globalConfig) {
        g_globalConfig = new FastConfig(path);
    }
}

FastConfig::FastConfig(const QString& configPath, QObject* parent)
    : QObject(parent)
    , m_configPath(configPath)
{
    m_flushTimer = new QTimer(this);
    m_flushTimer->setSingleShot(true);
    connect(m_flushTimer, &QTimer::timeout, this, &FastConfig::performFlush);
    
    // Load existing config into cache
    reload();
    
    qDebug() << "[FastConfig] Initialized with" << m_cache.size() << "entries";
}

FastConfig::~FastConfig()
{
    // Final flush on destruction
    if (m_dirty) {
        performFlush();
    }
}

QVariant FastConfig::get(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.value(key, defaultValue);
}

void FastConfig::set(const QString& key, const QVariant& value)
{
    {
        QMutexLocker locker(&m_mutex);
        
        // Skip if unchanged
        if (m_cache.value(key) == value) {
            return;
        }
        
        m_cache[key] = value;
        m_dirty = true;
    }
    
    emit configChanged(key, value);
    
    // Schedule async flush (unless in batch mode)
    if (!m_batchMode && !m_flushTimer->isActive()) {
        m_flushTimer->start(m_flushDelayMs);
    }
}

bool FastConfig::contains(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.contains(key);
}

void FastConfig::remove(const QString& key)
{
    {
        QMutexLocker locker(&m_mutex);
        if (!m_cache.contains(key)) {
            return;
        }
        m_cache.remove(key);
        m_dirty = true;
    }
    
    if (!m_batchMode && !m_flushTimer->isActive()) {
        m_flushTimer->start(m_flushDelayMs);
    }
}

void FastConfig::flush()
{
    m_flushTimer->stop();
    performFlush();
}

void FastConfig::performFlush()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_dirty) {
        return;
    }
    
    // Ensure directory exists
    QDir dir = QFileInfo(m_configPath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QSettings settings(m_configPath, QSettings::IniFormat);
    
    // Write all cached values
    for (auto it = m_cache.constBegin(); it != m_cache.constEnd(); ++it) {
        settings.setValue(it.key(), it.value());
    }
    
    settings.sync();
    m_dirty = false;
    
    qDebug() << "[FastConfig] Flushed" << m_cache.size() << "entries to disk";
    
    emit flushed();
}

void FastConfig::reload()
{
    QMutexLocker locker(&m_mutex);
    
    m_cache.clear();
    
    if (!QFile::exists(m_configPath)) {
        return;
    }
    
    QSettings settings(m_configPath, QSettings::IniFormat);
    
    for (const QString& key : settings.allKeys()) {
        m_cache[key] = settings.value(key);
    }
    
    m_dirty = false;
    qDebug() << "[FastConfig] Loaded" << m_cache.size() << "entries from disk";
}

QStringList FastConfig::keys() const
{
    QMutexLocker locker(&m_mutex);
    return m_cache.keys();
}

void FastConfig::beginBatch()
{
    m_batchMode = true;
    m_flushTimer->stop();
}

void FastConfig::endBatch()
{
    m_batchMode = false;
    if (m_dirty) {
        m_flushTimer->start(m_flushDelayMs);
    }
}

} // namespace NeoZ
