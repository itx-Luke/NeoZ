#ifndef ZERECA_EMERGENCY_ROLLBACK_H
#define ZERECA_EMERGENCY_ROLLBACK_H

#include "../types/ZerecaTypes.h"
#include "TargetState.h"
#include "FlightRecorder.h"
#include <QObject>

namespace Zereca {

/**
 * @brief Emergency Rollback Handler
 * 
 * Triggered on critical failures:
 * - App crash
 * - Thermal runaway
 * - BSOD signal
 * - Watchdog timeout
 * - Privilege loss
 * 
 * Always restores:
 * - Default power plan (Balanced)
 * - Default scheduling
 * - Default timers
 */
class EmergencyRollback : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief Rollback trigger reasons.
     */
    enum class Trigger {
        AppCrash,
        ThermalRunaway,
        BSODSignal,
        WatchdogTimeout,
        PrivilegeLost,
        UserRequested,
        Manual
    };
    Q_ENUM(Trigger)
    
    explicit EmergencyRollback(TargetStateManager* targetState,
                                FlightRecorder* flightRecorder,
                                QObject* parent = nullptr);
    ~EmergencyRollback() override = default;
    
    /**
     * @brief Execute emergency rollback.
     * @param trigger What triggered the rollback
     * @return true if rollback succeeded
     */
    Q_INVOKABLE bool execute(Trigger trigger);
    
    /**
     * @brief Check if we're currently in rollback state.
     */
    bool isRolledBack() const { return m_rolledBack; }
    
    /**
     * @brief Get the last rollback trigger.
     */
    Trigger lastTrigger() const { return m_lastTrigger; }
    
    /**
     * @brief Clear rollback state (after user acknowledges).
     */
    Q_INVOKABLE void acknowledge();
    
signals:
    /**
     * @brief Emitted when rollback is executed.
     */
    void rollbackExecuted(Trigger trigger, bool success);
    
    /**
     * @brief Emitted when rollback state changes.
     */
    void rollbackStateChanged(bool isRolledBack);
    
private:
    bool restoreDefaults();
    
    TargetStateManager* m_targetState = nullptr;
    FlightRecorder* m_flightRecorder = nullptr;
    bool m_rolledBack = false;
    Trigger m_lastTrigger = Trigger::Manual;
};

} // namespace Zereca

#endif // ZERECA_EMERGENCY_ROLLBACK_H
