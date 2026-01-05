#ifndef ZERECA_CONTROLLER_H
#define ZERECA_CONTROLLER_H

#include "types/ZerecaTypes.h"
#include "core/TargetState.h"
#include "core/StateReconciler.h"
#include "core/FlightRecorder.h"
#include "core/EmergencyRollback.h"
#include "core/TelemetryReader.h"
#include "arbiter/OptimizationArbiter.h"
#include "arbiter/ProbationLedger.h"
#include "arbiter/OutcomeClassifier.h"
#include "policy/EmulatorDetector.h"
#include "policy/ObservationPhase.h"
#include "policy/HypothesisEngine.h"
#include "policy/ShadowMode.h"

#include <QObject>
#include <QVariantList>

namespace Zereca {

/**
 * @brief ZerecaController - QML-exposed bridge to the Zereca control plane.
 * 
 * This is the single entry point for the UI to interact with all three systems:
 * - System A (Enforcement)
 * - System B (Learning)
 * - System C (Arbiter)
 */
class ZerecaController : public QObject
{
    Q_OBJECT
    
    // ========== Status Properties ==========
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(float emulatorConfidence READ emulatorConfidence NOTIFY emulatorConfidenceChanged)
    Q_PROPERTY(QString emulatorName READ emulatorName NOTIFY emulatorDetected)
    Q_PROPERTY(bool adminMode READ hasAdminPrivileges CONSTANT)
    
    // ========== Metrics Properties ==========
    Q_PROPERTY(double fps READ fps NOTIFY metricsUpdated)
    Q_PROPERTY(double fpsVariance READ fpsVariance NOTIFY metricsUpdated)
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY metricsUpdated)
    Q_PROPERTY(double memoryPressure READ memoryPressure NOTIFY metricsUpdated)
    
    // ========== Learning Properties ==========
    Q_PROPERTY(float observationProgress READ observationProgress NOTIFY observationProgressChanged)
    Q_PROPERTY(int hypothesesCount READ hypothesesCount NOTIFY hypothesesChanged)
    Q_PROPERTY(int trialsCompleted READ trialsCompleted NOTIFY trialsChanged)
    Q_PROPERTY(int optimizationsApplied READ optimizationsApplied NOTIFY optimizationsChanged)
    
    // ========== Safety Properties ==========
    Q_PROPERTY(int driftCount READ driftCount NOTIFY driftDetected)
    Q_PROPERTY(int probationCount READ probationCount NOTIFY probationChanged)
    Q_PROPERTY(bool rollbackActive READ isRollbackActive NOTIFY rollbackStateChanged)
    Q_PROPERTY(QVariantList eventLog READ eventLog NOTIFY eventLogChanged)
    
public:
    explicit ZerecaController(QObject* parent = nullptr);
    ~ZerecaController() override;
    
    // ========== Control Methods ==========
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void forceReconcile();
    Q_INVOKABLE void acknowledgeRollback();
    Q_INVOKABLE void clearProbation();
    Q_INVOKABLE void resetLearning();
    
    // ========== Property Getters ==========
    bool isRunning() const { return m_running; }
    QString status() const { return m_status; }
    QString mode() const { return m_mode; }
    float emulatorConfidence() const;
    QString emulatorName() const;
    bool hasAdminPrivileges() const;
    
    double fps() const { return m_fps; }
    double fpsVariance() const { return m_fpsVariance; }
    double cpuUsage() const { return m_cpuUsage; }
    double memoryPressure() const { return m_memoryPressure; }
    
    float observationProgress() const;
    int hypothesesCount() const;
    int trialsCompleted() const { return m_trialsCompleted; }
    int optimizationsApplied() const { return m_optimizationsApplied; }
    
    int driftCount() const;
    int probationCount() const;
    bool isRollbackActive() const;
    QVariantList eventLog() const { return m_eventLog; }
    
signals:
    void runningChanged(bool running);
    void statusChanged(const QString& status);
    void modeChanged(const QString& mode);
    void emulatorConfidenceChanged(float confidence);
    void emulatorDetected(const QString& name);
    void metricsUpdated();
    void observationProgressChanged(float progress);
    void hypothesesChanged(int count);
    void trialsChanged(int count);
    void optimizationsChanged(int count);
    void driftDetected(const QString& component);
    void probationChanged(int count);
    void rollbackStateChanged(bool active);
    void eventLogChanged();
    
private slots:
    void onEmulatorDetected(const EmulatorInfo& info);
    void onEmulatorLost(uint32_t pid);
    void onObservationComplete(const BaselineMetrics& baseline);
    void onMetricsUpdated(const AggregatedMetrics& metrics);
    void onTrialComplete(const ShadowTrialResult& result);
    void onReconciliationComplete(int changes);
    void onDriftDetected(const QString& component, const QString& expected, const QString& actual);
    void onRollbackExecuted(EmergencyRollback::Trigger trigger, bool success);
    
private:
    void initializeSubsystems();
    void transitionToMode(const QString& newMode);
    void addLogEntry(const QString& level, const QString& message);
    void runNextHypothesis();
    
    // State
    bool m_running = false;
    QString m_status = "Idle";
    QString m_mode = "STANDBY";
    
    // Metrics cache
    double m_fps = 0.0;
    double m_fpsVariance = 0.0;
    double m_cpuUsage = 0.0;
    double m_memoryPressure = 0.0;
    
    // Counters
    int m_trialsCompleted = 0;
    int m_optimizationsApplied = 0;
    
    // Event log (last 100 entries)
    QVariantList m_eventLog;
    
    // ========== Subsystems ==========
    // System A (Enforcement)
    TargetStateManager* m_targetState = nullptr;
    StateReconciler* m_stateReconciler = nullptr;
    FlightRecorder* m_flightRecorder = nullptr;
    EmergencyRollback* m_emergencyRollback = nullptr;
    TelemetryReader* m_telemetryReader = nullptr;
    
    // System B (Learning)
    EmulatorDetector* m_emulatorDetector = nullptr;
    ObservationPhase* m_observationPhase = nullptr;
    HypothesisEngine* m_hypothesisEngine = nullptr;
    ShadowMode* m_shadowMode = nullptr;
    
    // System C (Arbiter)
    ProbationLedger* m_probationLedger = nullptr;
    OptimizationArbiter* m_arbiter = nullptr;
    OutcomeClassifier* m_outcomeClassifier = nullptr;
    
    // Current context
    BaselineMetrics m_baseline;
    EmulatorInfo m_currentEmulator;
};

} // namespace Zereca

#endif // ZERECA_CONTROLLER_H
