#include "TargetState.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDebug>

namespace Zereca {

TargetStateManager::TargetStateManager(QObject* parent)
    : QObject(parent)
{
    // Use app data location for persistence
    m_configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(m_configDir);
    
    // Try to load existing state, otherwise use defaults
    if (!load()) {
        resetToDefaults();
    }
}

void TargetStateManager::update(const TargetState& newState)
{
    TargetState oldState = m_state;
    m_state = newState;
    
    if (!save()) {
        qWarning() << "[Zereca] Failed to persist target state to disk";
    }
    
    emit stateChanged(oldState, m_state);
}

void TargetStateManager::patch(const QJsonObject& partial)
{
    // Merge partial update with current state
    QJsonObject current = m_state.toJson();
    
    for (auto it = partial.constBegin(); it != partial.constEnd(); ++it) {
        current[it.key()] = it.value();
    }
    
    update(TargetState::fromJson(current));
}

bool TargetStateManager::load()
{
    QFile file(statePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "[Zereca] No existing target state found, using defaults";
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[Zereca] Failed to parse target state:" << error.errorString();
        return false;
    }
    
    m_state = TargetState::fromJson(doc.object());
    qDebug() << "[Zereca] Loaded target state from" << statePath();
    return true;
}

bool TargetStateManager::save()
{
    QFile file(statePath());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[Zereca] Failed to open target state file for writing:" << statePath();
        return false;
    }
    
    QJsonDocument doc(m_state.toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "[Zereca] Saved target state to" << statePath();
    return true;
}

void TargetStateManager::resetToDefaults()
{
    m_state = TargetState();  // Default-constructed state
    m_state.powerMode = "balanced";
    m_state.timerResolution = "default";
    m_state.cpuParking = true;
    m_state.standbyPurge = "off";
    m_state.processAffinity.clear();
    
    qDebug() << "[Zereca] Target state reset to defaults";
}

QString TargetStateManager::statePath() const
{
    return m_configDir + "/zereca_target_state.json";
}

} // namespace Zereca
