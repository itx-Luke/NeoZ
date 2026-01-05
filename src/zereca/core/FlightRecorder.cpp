#include "FlightRecorder.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>
#include <QMutexLocker>

namespace Zereca {

FlightRecorder::FlightRecorder(QObject* parent)
    : QObject(parent)
{
    m_dumpDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/zereca_dumps";
    QDir().mkpath(m_dumpDir);
    
    m_buffer.reserve(1000);  // Pre-allocate for typical usage
}

FlightRecorder::~FlightRecorder()
{
    // Optionally dump on shutdown
}

void FlightRecorder::record(const StateChangeRecord& entry)
{
    QMutexLocker locker(&m_mutex);
    
    // Add new record
    m_buffer.push_back(entry);
    
    // Prune old records
    pruneOldRecords();
    
    emit recordCountChanged(static_cast<int>(m_buffer.size()));
}

void FlightRecorder::record(uint32_t component, uint64_t oldVal, uint64_t newVal,
                            float expectedGain, float actualDelta, uint8_t rollbackReason)
{
    StateChangeRecord entry;
    entry.timestamp = QDateTime::currentMSecsSinceEpoch();
    entry.component = component;
    entry.oldVal = oldVal;
    entry.newVal = newVal;
    entry.expectedGain = expectedGain;
    entry.actualDelta = actualDelta;
    entry.rollbackReason = rollbackReason;
    
    record(entry);
}

QString FlightRecorder::dumpToDisk(const QString& reason)
{
    QMutexLocker locker(&m_mutex);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("flight_recorder_%1.json").arg(timestamp);
    QString path = m_dumpDir + "/" + filename;
    
    QJsonObject root;
    root["dump_reason"] = reason;
    root["dump_timestamp"] = QDateTime::currentMSecsSinceEpoch();
    root["record_count"] = static_cast<int>(m_buffer.size());
    
    QJsonArray records;
    for (const auto& rec : m_buffer) {
        QJsonObject obj;
        obj["timestamp"] = static_cast<qint64>(rec.timestamp);
        obj["component"] = static_cast<int>(rec.component);
        obj["old_val"] = static_cast<qint64>(rec.oldVal);
        obj["new_val"] = static_cast<qint64>(rec.newVal);
        obj["expected_gain"] = rec.expectedGain;
        obj["actual_delta"] = rec.actualDelta;
        obj["rollback_reason"] = rec.rollbackReason;
        records.append(obj);
    }
    root["records"] = records;
    
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        
        qDebug() << "[Zereca] FlightRecorder dump created:" << path;
        emit dumpCreated(path, reason);
        return path;
    }
    
    qWarning() << "[Zereca] Failed to create flight recorder dump";
    return QString();
}

std::vector<StateChangeRecord> FlightRecorder::recentRecords(size_t count) const
{
    QMutexLocker locker(&m_mutex);
    
    if (count >= m_buffer.size()) {
        return m_buffer;
    }
    
    return std::vector<StateChangeRecord>(m_buffer.end() - count, m_buffer.end());
}

std::vector<StateChangeRecord> FlightRecorder::allRecords() const
{
    QMutexLocker locker(&m_mutex);
    return m_buffer;
}

void FlightRecorder::clear()
{
    QMutexLocker locker(&m_mutex);
    m_buffer.clear();
    emit recordCountChanged(0);
}

int FlightRecorder::recordCount() const
{
    QMutexLocker locker(&m_mutex);
    return static_cast<int>(m_buffer.size());
}

void FlightRecorder::pruneOldRecords()
{
    // Already under lock from caller
    
    uint64_t now = QDateTime::currentMSecsSinceEpoch();
    uint64_t cutoff = now - MAX_BUFFER_DURATION_MS;
    
    // Remove records older than 5 minutes
    auto it = m_buffer.begin();
    while (it != m_buffer.end() && it->timestamp < cutoff) {
        ++it;
    }
    
    if (it != m_buffer.begin()) {
        m_buffer.erase(m_buffer.begin(), it);
    }
    
    // Enforce hard cap
    while (m_buffer.size() > MAX_RECORDS) {
        m_buffer.erase(m_buffer.begin());
    }
}

} // namespace Zereca
