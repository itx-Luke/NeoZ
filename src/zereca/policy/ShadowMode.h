#ifndef ZERECA_SHADOW_MODE_H
#define ZERECA_SHADOW_MODE_H

#include "../types/ZerecaTypes.h"
#include "../core/TelemetryReader.h"
#include "EmulatorDetector.h"
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

namespace Zereca {

/**
 * @brief Shadow trial result.
 */
struct ShadowTrialResult {
    OptimizationProposal proposal;
    BaselineMetrics beforeMetrics;
    BaselineMetrics afterMetrics;
    float performanceDelta = 0.0f;
    uint64_t durationMs = 0;
    bool completed = false;
    QString failureReason;
};

/**
 * @brief Shadow Mode - A/B testing infrastructure (System B).
 * 
 * Allows reversible, process-scoped optimizations to be tested
 * without user visibility.
 * 
 * Rules per spec:
 * - Allowed ONLY for process-scoped changes
 * - Reversible, low-risk knobs only
 * - Short duration (30-60 seconds)
 * - Automatic revert on completion
 * - Invisible to user
 * 
 * Shadow mode is the "try before you commit" mechanism.
 */
class ShadowMode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(int trialCount READ trialCount NOTIFY trialComplete)
    
public:
    /**
     * @brief Configuration.
     */
    struct Config {
        uint64_t trialDurationMs = 30000;    ///< 30 seconds default
        uint64_t stabilizationMs = 5000;     ///< Wait before measuring
        uint64_t maxTrialDurationMs = 60000; ///< Never exceed 60 seconds
    };
    
    explicit ShadowMode(TelemetryReader* telemetry, 
                         EmulatorDetector* detector,
                         QObject* parent = nullptr);
    ~ShadowMode() override;
    
    /**
     * @brief Start a shadow trial for a proposal.
     * @param proposal The optimization to test
     * @param targetPid Process to apply to
     * @return true if trial started
     */
    Q_INVOKABLE bool startTrial(const OptimizationProposal& proposal, uint32_t targetPid);
    
    /**
     * @brief Abort the current trial.
     */
    Q_INVOKABLE void abortTrial();
    
    /**
     * @brief Check if a trial is active.
     */
    bool isActive() const { return m_active; }
    
    /**
     * @brief Get completed trial count.
     */
    int trialCount() const { return m_trialCount; }
    
    /**
     * @brief Get the last trial result.
     */
    const ShadowTrialResult& lastResult() const { return m_lastResult; }
    
    /**
     * @brief Check if a change type can be shadow tested.
     */
    static bool canShadowTest(ChangeType type);
    
    /**
     * @brief Get/set configuration.
     */
    const Config& config() const { return m_config; }
    void setConfig(const Config& config) { m_config = config; }
    
signals:
    void activeChanged(bool active);
    
    /**
     * @brief Emitted when a trial completes.
     */
    void trialComplete(const ShadowTrialResult& result);
    
    /**
     * @brief Emitted when a trial is aborted.
     */
    void trialAborted(const QString& reason);
    
    /**
     * @brief Emitted periodically during trial.
     */
    void trialProgress(float progress, float currentDelta);
    
private slots:
    void onTrialTick();
    void onTrialEnd();
    void onEmulatorLost(uint32_t pid);
    
private:
    bool applyChange(const OptimizationProposal& proposal, uint32_t pid);
    bool revertChange();
    BaselineMetrics collectMetrics();
    
    TelemetryReader* m_telemetry = nullptr;
    EmulatorDetector* m_emulatorDetector = nullptr;
    
    QTimer* m_trialTimer = nullptr;
    QTimer* m_tickTimer = nullptr;
    QElapsedTimer m_elapsed;
    
    Config m_config;
    bool m_active = false;
    int m_trialCount = 0;
    
    // Current trial state
    OptimizationProposal m_currentProposal;
    uint32_t m_currentPid = 0;
    uint64_t m_originalValue = 0;
    
    // Metrics
    BaselineMetrics m_beforeMetrics;
    std::vector<AggregatedMetrics> m_trialSamples;
    
    ShadowTrialResult m_lastResult;
};

} // namespace Zereca

#endif // ZERECA_SHADOW_MODE_H
