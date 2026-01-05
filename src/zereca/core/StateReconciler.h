#ifndef ZERECA_STATE_RECONCILER_H
#define ZERECA_STATE_RECONCILER_H

#include "../types/ZerecaTypes.h"
#include "TargetState.h"
#include <QObject>
#include <QTimer>
#include <memory>

namespace Zereca {

/**
 * @brief Current system state snapshot.
 */
struct CurrentState {
    QString powerMode;
    QString timerResolution;
    bool cpuParking = true;
    QString standbyPurge;
    QHash<QString, QString> processAffinity;
    uint64_t timestamp = 0;
};

/**
 * @brief State Reconciler - The heart of System A.
 * 
 * Runs a continuous reconciliation loop (1-5 seconds) that:
 * 1. Reads the Target State Document (TSD)
 * 2. Audits the current OS state
 * 3. Compares desired vs actual
 * 4. Re-applies on drift detection
 * 5. Logs external interference
 * 
 * The reconciler NEVER makes policy decisions. It only enforces
 * whatever state the TSD declares.
 */
class StateReconciler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(int intervalMs READ intervalMs WRITE setIntervalMs NOTIFY intervalChanged)
    Q_PROPERTY(int driftCount READ driftCount NOTIFY driftDetected)
    
public:
    explicit StateReconciler(TargetStateManager* targetState, QObject* parent = nullptr);
    ~StateReconciler() override;
    
    /**
     * @brief Start the reconciliation loop.
     */
    Q_INVOKABLE void start();
    
    /**
     * @brief Stop the reconciliation loop.
     */
    Q_INVOKABLE void stop();
    
    /**
     * @brief Force immediate reconciliation (don't wait for timer).
     */
    Q_INVOKABLE void reconcileNow();
    
    /**
     * @brief Check if the reconciler is running.
     */
    bool isRunning() const { return m_running; }
    
    /**
     * @brief Get the reconciliation interval in milliseconds.
     */
    int intervalMs() const { return m_intervalMs; }
    
    /**
     * @brief Set the reconciliation interval.
     * @param ms Interval between 1000-5000ms
     */
    void setIntervalMs(int ms);
    
    /**
     * @brief Get the number of drift events detected.
     */
    int driftCount() const { return m_driftCount; }
    
    /**
     * @brief Get the last known current state.
     */
    const CurrentState& lastKnownState() const { return m_currentState; }
    
signals:
    void runningChanged(bool running);
    void intervalChanged(int ms);
    
    /**
     * @brief Emitted when drift is detected and corrected.
     * @param component Which component drifted (e.g., "power_mode")
     * @param expected Expected value
     * @param actual Actual value found
     */
    void driftDetected(const QString& component, const QString& expected, const QString& actual);
    
    /**
     * @brief Emitted after each reconciliation cycle.
     * @param changesApplied Number of changes made
     */
    void reconciliationComplete(int changesApplied);
    
    /**
     * @brief Emitted when reconciliation fails.
     * @param error Error description
     */
    void reconciliationError(const QString& error);
    
private slots:
    void onReconciliationTick();
    
private:
    // State reading
    CurrentState readCurrentState();
    QString readCurrentPowerMode();
    QString readTimerResolution();
    bool readCpuParkingEnabled();
    
    // State enforcement
    int enforceState(const TargetState& target, const CurrentState& current);
    bool enforcePowerMode(const QString& mode);
    bool enforceTimerResolution(const QString& resolution);
    bool enforceCpuParking(bool enabled);
    bool enforceProcessAffinity(const QString& process, const QString& coreGroup);
    
    TargetStateManager* m_targetState = nullptr;
    QTimer* m_timer = nullptr;
    CurrentState m_currentState;
    
    bool m_running = false;
    int m_intervalMs = 2000;  // Default 2 seconds
    int m_driftCount = 0;
};

} // namespace Zereca

#endif // ZERECA_STATE_RECONCILER_H
