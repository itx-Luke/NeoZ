#include "ZerecaController.h"
#include <QDebug>
#include <QDateTime>

namespace Zereca {

ZerecaController::ZerecaController(QObject* parent)
    : QObject(parent)
{
    initializeSubsystems();
}

ZerecaController::~ZerecaController()
{
    stop();
}

void ZerecaController::initializeSubsystems()
{
    // System A - Enforcement
    m_targetState = new TargetStateManager(this);
    m_flightRecorder = new FlightRecorder(this);
    m_stateReconciler = new StateReconciler(m_targetState, this);
    m_emergencyRollback = new EmergencyRollback(m_targetState, m_flightRecorder, this);
    m_telemetryReader = new TelemetryReader(this);
    
    // System C - Arbiter (before B, as B uses it)
    m_probationLedger = new ProbationLedger(this);
    m_arbiter = new OptimizationArbiter(m_probationLedger, m_flightRecorder, this);
    m_outcomeClassifier = new OutcomeClassifier(this);
    
    // System B - Learning
    m_emulatorDetector = new EmulatorDetector(this);
    m_observationPhase = new ObservationPhase(m_telemetryReader, m_emulatorDetector, this);
    m_hypothesisEngine = new HypothesisEngine(this);
    m_shadowMode = new ShadowMode(m_telemetryReader, m_emulatorDetector, this);
    
    // Connect signals
    connect(m_emulatorDetector, &EmulatorDetector::emulatorDetected,
            this, &ZerecaController::onEmulatorDetected);
    connect(m_emulatorDetector, &EmulatorDetector::emulatorLost,
            this, &ZerecaController::onEmulatorLost);
    
    connect(m_observationPhase, &ObservationPhase::observationComplete,
            this, &ZerecaController::onObservationComplete);
    connect(m_observationPhase, &ObservationPhase::progressChanged,
            this, [this](float p) { emit observationProgressChanged(p); });
    
    connect(m_telemetryReader, &TelemetryReader::metricsUpdated,
            this, &ZerecaController::onMetricsUpdated);
    
    connect(m_shadowMode, &ShadowMode::trialComplete,
            this, &ZerecaController::onTrialComplete);
    
    connect(m_stateReconciler, &StateReconciler::reconciliationComplete,
            this, &ZerecaController::onReconciliationComplete);
    connect(m_stateReconciler, &StateReconciler::driftDetected,
            this, &ZerecaController::onDriftDetected);
    
    connect(m_emergencyRollback, &EmergencyRollback::rollbackExecuted,
            this, &ZerecaController::onRollbackExecuted);
    connect(m_emergencyRollback, &EmergencyRollback::rollbackStateChanged,
            this, &ZerecaController::rollbackStateChanged);
    
    addLogEntry("INFO", "Zereca subsystems initialized");
}

void ZerecaController::start()
{
    if (m_running) return;
    
    m_running = true;
    m_status = "Starting...";
    emit runningChanged(true);
    emit statusChanged(m_status);
    
    // Start System A
    m_telemetryReader->start();
    m_stateReconciler->start();
    
    // Start emulator detection
    m_emulatorDetector->startScanning(2000);
    
    transitionToMode("SCANNING");
    addLogEntry("INFO", "Zereca control plane started");
}

void ZerecaController::stop()
{
    if (!m_running) return;
    
    // Stop all subsystems
    m_emulatorDetector->stopScanning();
    m_observationPhase->stop();
    m_shadowMode->abortTrial();
    m_stateReconciler->stop();
    m_telemetryReader->stop();
    
    m_running = false;
    m_status = "Stopped";
    transitionToMode("STANDBY");
    
    emit runningChanged(false);
    emit statusChanged(m_status);
    
    addLogEntry("INFO", "Zereca control plane stopped");
}

void ZerecaController::forceReconcile()
{
    if (m_stateReconciler) {
        m_stateReconciler->reconcileNow();
        addLogEntry("INFO", "Forced reconciliation triggered");
    }
}

void ZerecaController::acknowledgeRollback()
{
    if (m_emergencyRollback) {
        m_emergencyRollback->acknowledge();
        addLogEntry("INFO", "Rollback acknowledged by user");
    }
}

void ZerecaController::clearProbation()
{
    if (m_probationLedger) {
        m_probationLedger->clearAll();
        addLogEntry("WARNING", "Probation ledger cleared manually");
    }
}

void ZerecaController::resetLearning()
{
    if (m_hypothesisEngine) {
        m_hypothesisEngine->resetPriors();
        m_trialsCompleted = 0;
        m_optimizationsApplied = 0;
        emit trialsChanged(0);
        emit optimizationsChanged(0);
        addLogEntry("WARNING", "Learning engine priors reset");
    }
}

float ZerecaController::emulatorConfidence() const
{
    return m_currentEmulator.confidence;
}

QString ZerecaController::emulatorName() const
{
    return m_currentEmulator.name;
}

bool ZerecaController::hasAdminPrivileges() const
{
    return TelemetryReader::hasAdminPrivileges();
}

float ZerecaController::observationProgress() const
{
    return m_observationPhase ? m_observationPhase->progress() : 0.0f;
}

int ZerecaController::hypothesesCount() const
{
    return m_hypothesisEngine ? m_hypothesisEngine->hypothesisCount() : 0;
}

int ZerecaController::driftCount() const
{
    return m_stateReconciler ? m_stateReconciler->driftCount() : 0;
}

int ZerecaController::probationCount() const
{
    return m_probationLedger ? m_probationLedger->entryCount() : 0;
}

bool ZerecaController::isRollbackActive() const
{
    return m_emergencyRollback && m_emergencyRollback->isRolledBack();
}

void ZerecaController::onEmulatorDetected(const EmulatorInfo& info)
{
    m_currentEmulator = info;
    
    m_status = QString("Detected %1 (%.0f%%)").arg(info.name).arg(info.confidence * 100);
    emit statusChanged(m_status);
    emit emulatorDetected(info.name);
    emit emulatorConfidenceChanged(info.confidence);
    
    addLogEntry("INFO", QString("Emulator detected: %1, PID: %2, Confidence: %3%")
                        .arg(info.name)
                        .arg(info.processId)
                        .arg(qRound(info.confidence * 100)));
    
    // Start observation if confidence meets threshold
    if (info.confidence >= 0.75f && !m_observationPhase->isObserving()) {
        transitionToMode("OBSERVING");
        m_observationPhase->start(info.processId);
    }
}

void ZerecaController::onEmulatorLost(uint32_t pid)
{
    Q_UNUSED(pid);
    m_currentEmulator = EmulatorInfo();
    m_status = "Scanning for emulators...";
    transitionToMode("SCANNING");
    
    emit statusChanged(m_status);
    emit emulatorConfidenceChanged(0.0f);
    
    addLogEntry("WARNING", "Emulator process exited");
}

void ZerecaController::onObservationComplete(const BaselineMetrics& baseline)
{
    m_baseline = baseline;
    
    m_status = QString("Baseline: %.1f FPS (±%.1f)")
               .arg(baseline.fps).arg(std::sqrt(baseline.fpsVariance));
    emit statusChanged(m_status);
    
    addLogEntry("INFO", QString("Observation complete: FPS=%.1f, Variance=%.2f")
                        .arg(baseline.fps).arg(baseline.fpsVariance));
    
    // Generate hypotheses
    transitionToMode("LEARNING");
    m_hypothesisEngine->generateHypotheses(baseline, m_currentEmulator.name);
    emit hypothesesChanged(m_hypothesisEngine->hypothesisCount());
    
    // Start testing hypotheses
    runNextHypothesis();
}

void ZerecaController::onMetricsUpdated(const AggregatedMetrics& metrics)
{
    m_fps = metrics.fps;
    m_fpsVariance = metrics.fpsVariance;
    m_cpuUsage = metrics.coreUtilization;
    m_memoryPressure = metrics.memoryPressure;
    emit metricsUpdated();
}

void ZerecaController::onTrialComplete(const ShadowTrialResult& result)
{
    m_trialsCompleted++;
    emit trialsChanged(m_trialsCompleted);
    
    // Classify outcome
    auto classification = m_outcomeClassifier->classify(
        result.beforeMetrics, result.afterMetrics, false, false);
    
    // Update priors
    m_hypothesisEngine->updatePriors(result.proposal, classification.outcome, 
                                      result.performanceDelta);
    
    // Record outcome in arbiter
    m_arbiter->recordOutcome(result.proposal, classification.outcome, 
                             result.performanceDelta);
    
    // Log result
    QString outcomeStr;
    switch (classification.outcome) {
        case Outcome::POSITIVE: outcomeStr = "POSITIVE"; break;
        case Outcome::NEUTRAL: outcomeStr = "NEUTRAL"; break;
        case Outcome::NEGATIVE_STABILITY: outcomeStr = "NEGATIVE_STABILITY"; break;
        case Outcome::NEGATIVE_SAFETY: outcomeStr = "NEGATIVE_SAFETY"; break;
    }
    
    addLogEntry(classification.outcome == Outcome::POSITIVE ? "SUCCESS" : "INFO",
                QString("Trial %1: %2 (delta: %3%)")
                .arg(m_trialsCompleted)
                .arg(outcomeStr)
                .arg(result.performanceDelta * 100, 0, 'f', 1));
    
    // If positive, count as applied optimization
    if (classification.outcome == Outcome::POSITIVE) {
        m_optimizationsApplied++;
        emit optimizationsChanged(m_optimizationsApplied);
        
        // Commit to target state
        // TODO: Apply to target state permanently
    }
    
    // Run next hypothesis
    runNextHypothesis();
}

void ZerecaController::onReconciliationComplete(int changes)
{
    if (changes > 0) {
        addLogEntry("INFO", QString("Reconciliation: %1 changes applied").arg(changes));
    }
}

void ZerecaController::onDriftDetected(const QString& component, 
                                        const QString& expected, 
                                        const QString& actual)
{
    addLogEntry("WARNING", QString("Drift detected in %1: expected %2, found %3")
                           .arg(component).arg(expected).arg(actual));
    emit driftDetected(component);
}

void ZerecaController::onRollbackExecuted(EmergencyRollback::Trigger trigger, bool success)
{
    QString triggerStr;
    switch (trigger) {
        case EmergencyRollback::Trigger::AppCrash: triggerStr = "app crash"; break;
        case EmergencyRollback::Trigger::ThermalRunaway: triggerStr = "thermal"; break;
        case EmergencyRollback::Trigger::BSODSignal: triggerStr = "BSOD signal"; break;
        case EmergencyRollback::Trigger::WatchdogTimeout: triggerStr = "watchdog"; break;
        case EmergencyRollback::Trigger::PrivilegeLost: triggerStr = "privilege lost"; break;
        default: triggerStr = "manual"; break;
    }
    
    addLogEntry("CRITICAL", QString("Emergency rollback: %1 (%2)")
                            .arg(triggerStr).arg(success ? "success" : "failed"));
    
    transitionToMode("ROLLBACK");
    m_status = "Emergency Rollback Active";
    emit statusChanged(m_status);
}

void ZerecaController::transitionToMode(const QString& newMode)
{
    if (m_mode == newMode) return;
    
    m_mode = newMode;
    emit modeChanged(newMode);
    
    addLogEntry("INFO", QString("Mode: %1").arg(newMode));
}

void ZerecaController::addLogEntry(const QString& level, const QString& message)
{
    QVariantMap entry;
    entry["timestamp"] = QDateTime::currentDateTime().toString("HH:mm:ss");
    entry["level"] = level;
    entry["message"] = message;
    
    m_eventLog.prepend(entry);
    
    // Keep last 100 entries
    while (m_eventLog.size() > 100) {
        m_eventLog.removeLast();
    }
    
    emit eventLogChanged();
    
    // Also log to console
    qDebug() << "[Zereca]" << level << message;
}

void ZerecaController::runNextHypothesis()
{
    if (!m_running || m_mode == "ROLLBACK") return;
    
    // Get next hypothesis
    Hypothesis h = m_hypothesisEngine->nextHypothesis();
    if (h.proposal.type == ChangeType::PRIORITY && h.proposal.proposedValue == 0) {
        // No more hypotheses
        transitionToMode("MONITORING");
        m_status = QString("Optimized: +%1 applied").arg(m_optimizationsApplied);
        emit statusChanged(m_status);
        return;
    }
    
    // Check with arbiter
    auto decision = m_arbiter->evaluate(h.proposal, m_currentEmulator.confidence);
    
    if (!decision.approved) {
        addLogEntry("INFO", QString("Proposal rejected: %1").arg(decision.explanation));
        // Try next
        runNextHypothesis();
        return;
    }
    
    // Run shadow trial
    if (ShadowMode::canShadowTest(h.proposal.type)) {
        transitionToMode("TESTING");
        m_shadowMode->startTrial(h.proposal, m_currentEmulator.processId);
    } else {
        // Skip non-shadow-testable for now
        runNextHypothesis();
    }
}

} // namespace Zereca
