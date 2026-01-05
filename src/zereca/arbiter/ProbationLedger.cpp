#include "ProbationLedger.h"
#include "../types/ContextHash.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDebug>
#include <QMutexLocker>

namespace Zereca {

ProbationLedger::ProbationLedger(QObject* parent)
    : QObject(parent)
{
    m_storagePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) 
                    + "/zereca_probation.json";
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    
    load();
}

ProbationLedger::~ProbationLedger()
{
    save();
}

bool ProbationLedger::isOnProbation(uint64_t configHash, const SystemContext& currentContext) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_entries.find(configHash);
    if (it == m_entries.end()) {
        return false;  // Not on probation
    }
    
    // Check if resurrection is allowed
    return !canResurrect(*it, currentContext);
}

void ProbationLedger::addToProbation(uint64_t configHash, Severity severity, const SystemContext& context)
{
    QMutexLocker locker(&m_mutex);
    
    ProbationEntry entry;
    entry.configHash = configHash;
    entry.lastFailureTs = QDateTime::currentMSecsSinceEpoch();
    entry.severity = severity;
    entry.driverVersion = context.gpuDriverVersion;
    entry.osBuild = context.osBuild;
    
    // Exponential backoff for repeat failures
    auto existing = m_entries.find(configHash);
    if (existing != m_entries.end()) {
        entry.backoff = existing->backoff * 2.0f;  // Double backoff
    } else {
        entry.backoff = 1.0f;
    }
    
    m_entries[configHash] = entry;
    
    locker.unlock();
    
    save();
    emit entryAdded(configHash, severity);
    emit entriesChanged(m_entries.size());
    
    qDebug() << "[Zereca] Added to probation:" << configHash 
             << "severity:" << static_cast<int>(severity);
}

std::optional<ProbationEntry> ProbationLedger::getEntry(uint64_t configHash) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_entries.find(configHash);
    if (it != m_entries.end()) {
        return *it;
    }
    return std::nullopt;
}

void ProbationLedger::clearEntry(uint64_t configHash)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_entries.remove(configHash) > 0) {
        locker.unlock();
        save();
        emit entryCleared(configHash);
        emit entriesChanged(m_entries.size());
        qDebug() << "[Zereca] Cleared probation entry:" << configHash;
    }
}

void ProbationLedger::clearAll()
{
    QMutexLocker locker(&m_mutex);
    
    m_entries.clear();
    
    locker.unlock();
    save();
    emit entriesChanged(0);
    
    qWarning() << "[Zereca] All probation entries cleared (manual reset)";
}

std::vector<ProbationEntry> ProbationLedger::allEntries() const
{
    QMutexLocker locker(&m_mutex);
    
    std::vector<ProbationEntry> result;
    result.reserve(m_entries.size());
    
    for (const auto& entry : m_entries) {
        result.push_back(entry);
    }
    
    return result;
}

int ProbationLedger::entryCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_entries.size();
}

bool ProbationLedger::load()
{
    QFile file(m_storagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[Zereca] Failed to parse probation ledger:" << error.errorString();
        return false;
    }
    
    QMutexLocker locker(&m_mutex);
    m_entries.clear();
    
    QJsonArray entries = doc.array();
    for (const auto& val : entries) {
        QJsonObject obj = val.toObject();
        ProbationEntry entry;
        entry.configHash = obj["config_hash"].toVariant().toULongLong();
        entry.lastFailureTs = obj["last_failure_ts"].toVariant().toULongLong();
        entry.severity = static_cast<Severity>(obj["severity"].toInt());
        entry.driverVersion = obj["driver_version"].toVariant().toULongLong();
        entry.osBuild = obj["os_build"].toVariant().toULongLong();
        entry.backoff = static_cast<float>(obj["backoff"].toDouble(1.0));
        
        m_entries[entry.configHash] = entry;
    }
    
    qDebug() << "[Zereca] Loaded" << m_entries.size() << "probation entries";
    return true;
}

bool ProbationLedger::save()
{
    QMutexLocker locker(&m_mutex);
    
    QJsonArray entries;
    for (const auto& entry : m_entries) {
        QJsonObject obj;
        obj["config_hash"] = static_cast<qint64>(entry.configHash);
        obj["last_failure_ts"] = static_cast<qint64>(entry.lastFailureTs);
        obj["severity"] = static_cast<int>(entry.severity);
        obj["driver_version"] = static_cast<qint64>(entry.driverVersion);
        obj["os_build"] = static_cast<qint64>(entry.osBuild);
        obj["backoff"] = entry.backoff;
        entries.append(obj);
    }
    
    locker.unlock();
    
    QFile file(m_storagePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[Zereca] Failed to save probation ledger";
        return false;
    }
    
    QJsonDocument doc(entries);
    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();
    
    return true;
}

bool ProbationLedger::canResurrect(const ProbationEntry& entry, const SystemContext& currentContext) const
{
    // NEVER resurrect Severity 3 (BSOD)
    if (entry.severity == Severity::CRITICAL) {
        return false;
    }
    
    // Severity 1 (FPS regression) - allow auto-retry after backoff
    if (entry.severity == Severity::LOW) {
        uint64_t now = QDateTime::currentMSecsSinceEpoch();
        uint64_t baseBackoff = 5 * 60 * 1000;  // 5 minutes base
        uint64_t actualBackoff = static_cast<uint64_t>(baseBackoff * entry.backoff);
        return now > (entry.lastFailureTs + actualBackoff);
    }
    
    // Severity 2 (App crash) - context shift only
    if (entry.severity == Severity::MEDIUM) {
        SystemContext failureContext;
        failureContext.gpuDriverVersion = entry.driverVersion;
        failureContext.osBuild = entry.osBuild;
        
        return currentContext.hasShiftedFrom(failureContext);
    }
    
    return false;
}

} // namespace Zereca
