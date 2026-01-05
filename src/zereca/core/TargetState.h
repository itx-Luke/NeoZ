#ifndef ZERECA_TARGET_STATE_H
#define ZERECA_TARGET_STATE_H

#include "../types/ZerecaTypes.h"
#include <QObject>
#include <QJsonDocument>
#include <QString>

namespace Zereca {

/**
 * @brief Target State Document Manager
 * 
 * Manages the Target State Document (TSD) - the single source of truth
 * for desired system configuration. System A reconciles to this state.
 * 
 * The TSD is persisted to disk and survives application restarts.
 */
class TargetStateManager : public QObject
{
    Q_OBJECT
    
public:
    explicit TargetStateManager(QObject* parent = nullptr);
    ~TargetStateManager() override = default;
    
    /**
     * @brief Get the current target state.
     */
    const TargetState& current() const { return m_state; }
    
    /**
     * @brief Update the target state.
     * Emits stateChanged() and persists to disk.
     */
    void update(const TargetState& newState);
    
    /**
     * @brief Apply a partial update (merge with current state).
     */
    void patch(const QJsonObject& partial);
    
    /**
     * @brief Load state from disk.
     * @return true if loaded successfully
     */
    bool load();
    
    /**
     * @brief Save state to disk.
     * @return true if saved successfully
     */
    bool save();
    
    /**
     * @brief Reset to safe defaults.
     */
    void resetToDefaults();
    
    /**
     * @brief Get the path where state is persisted.
     */
    QString statePath() const;
    
signals:
    /**
     * @brief Emitted when target state changes.
     * @param oldState Previous state
     * @param newState New state
     */
    void stateChanged(const TargetState& oldState, const TargetState& newState);
    
private:
    TargetState m_state;
    QString m_configDir;
};

} // namespace Zereca

#endif // ZERECA_TARGET_STATE_H
