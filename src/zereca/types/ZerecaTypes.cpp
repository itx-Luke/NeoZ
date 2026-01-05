#include "ZerecaTypes.h"
#include <QJsonArray>
#include <QCryptographicHash>
#include <QByteArray>

namespace Zereca {

// ============================================================================
// TargetState Implementation
// ============================================================================

QJsonObject TargetState::toJson() const
{
    QJsonObject json;
    json["power_mode"] = powerMode;
    json["timer_resolution"] = timerResolution;
    json["cpu_parking"] = cpuParking;
    json["standby_purge"] = standbyPurge;
    
    QJsonObject affinityObj;
    for (auto it = processAffinity.constBegin(); it != processAffinity.constEnd(); ++it) {
        affinityObj[it.key()] = it.value();
    }
    json["process_affinity"] = affinityObj;
    
    return json;
}

TargetState TargetState::fromJson(const QJsonObject& json)
{
    TargetState state;
    state.powerMode = json["power_mode"].toString("balanced");
    state.timerResolution = json["timer_resolution"].toString("default");
    state.cpuParking = json["cpu_parking"].toBool(true);
    state.standbyPurge = json["standby_purge"].toString("off");
    
    QJsonObject affinityObj = json["process_affinity"].toObject();
    for (auto it = affinityObj.constBegin(); it != affinityObj.constEnd(); ++it) {
        state.processAffinity[it.key()] = it.value().toString();
    }
    
    return state;
}

uint64_t TargetState::hash() const
{
    QByteArray data;
    data.append(powerMode.toUtf8());
    data.append(timerResolution.toUtf8());
    data.append(cpuParking ? "1" : "0");
    data.append(standbyPurge.toUtf8());
    
    // Sort keys for deterministic hash
    QStringList keys = processAffinity.keys();
    keys.sort();
    for (const QString& key : keys) {
        data.append(key.toUtf8());
        data.append(processAffinity[key].toUtf8());
    }
    
    QByteArray hashBytes = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    // Take first 8 bytes as uint64_t
    uint64_t result = 0;
    memcpy(&result, hashBytes.constData(), sizeof(result));
    return result;
}

// ============================================================================
// SystemContext Implementation
// ============================================================================

uint64_t SystemContext::hash() const
{
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&gpuDriverVersion), sizeof(gpuDriverVersion));
    data.append(reinterpret_cast<const char*>(&osBuild), sizeof(osBuild));
    data.append(reinterpret_cast<const char*>(&biosVersion), sizeof(biosVersion));
    data.append(reinterpret_cast<const char*>(&emulatorBinaryHash), sizeof(emulatorBinaryHash));
    
    QByteArray hashBytes = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    uint64_t result = 0;
    memcpy(&result, hashBytes.constData(), sizeof(result));
    return result;
}

bool SystemContext::hasShiftedFrom(const SystemContext& other) const
{
    // Context has shifted if ANY of these have changed
    return gpuDriverVersion != other.gpuDriverVersion ||
           osBuild != other.osBuild ||
           biosVersion != other.biosVersion ||
           emulatorBinaryHash != other.emulatorBinaryHash;
}

} // namespace Zereca
