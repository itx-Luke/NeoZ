#ifndef ZERECA_OBSERVATION_PHASE_H
#define ZERECA_OBSERVATION_PHASE_H

#include "../types/ZerecaTypes.h"
#include "../core/TelemetryReader.h"
#include "EmulatorDetector.h"
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <vector>

namespace Zereca {

/**
 * @brief Observation Phase - Baseline metrics collection (System B).
 * 
 * Before any optimization is applied, Zereca observes the system
 * for 2-5 minutes to establish baseline performance metrics.
 * 
 * During observation:
 * - No changes are applied
 * - Metrics are collected continuously
 * - Baseline is established for comparison
 * 
 * After observation completes, the HypothesisEngine uses
 * the baseline to evaluate potential optimizations.
 */
class ObservationPhase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool observing READ isObserving NOTIFY observingChanged)
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int elapsedMs READ elapsedMs NOTIFY progressChanged)
    
public:
    /**
     * @brief Observation configuration.
     */
    struct Config {
        uint64_t minDurationMs = 2 * 60 * 1000;   ///< Minimum 2 minutes
        uint64_t maxDurationMs = 5 * 60 * 1000;   ///< Maximum 5 minutes
        uint64_t sampleIntervalMs = 500;          ///< Sample every 500ms
        float stabilityThreshold = 0.05f;          ///< Variance threshold for early exit
        int minSamplesForStability = 60;           ///< Min samples before checking stability
    };
    
    /**
     * @brief Raw sample collected during observation.
     */
    struct Sample {
        uint64_t timestamp;
        double fps;
        double frameTimeMs;
        double cpuUsage;
        double gpuUsage;
        double memoryPressure;
    };
    
    explicit ObservationPhase(TelemetryReader* telemetry, 
                               EmulatorDetector* emulatorDetector,
                               QObject* parent = nullptr);
    ~ObservationPhase() override;
    
    /**
     * @brief Start observation phase for a detected emulator.
     * @param targetPid Process ID to observe
     */
    Q_INVOKABLE void start(uint32_t targetPid);
    
    /**
     * @brief Stop observation early.
     */
    Q_INVOKABLE void stop();
    
    /**
     * @brief Check if observation is in progress.
     */
    bool isObserving() const { return m_observing; }
    
    /**
     * @brief Get observation progress (0.0–1.0).
     */
    float progress() const;
    
    /**
     * @brief Get elapsed time in milliseconds.
     */
    int elapsedMs() const;
    
    /**
     * @brief Get the computed baseline metrics.
     * Only valid after observation completes.
     */
    const BaselineMetrics& baseline() const { return m_baseline; }
    
    /**
     * @brief Get all collected samples.
     */
    const std::vector<Sample>& samples() const { return m_samples; }
    
    /**
     * @brief Get/set configuration.
     */
    const Config& config() const { return m_config; }
    void setConfig(const Config& config) { m_config = config; }
    
signals:
    void observingChanged(bool observing);
    void progressChanged(float progress);
    
    /**
     * @brief Emitted when observation completes successfully.
     */
    void observationComplete(const BaselineMetrics& baseline);
    
    /**
     * @brief Emitted if observation fails (e.g., emulator exits).
     */
    void observationFailed(const QString& reason);
    
    /**
     * @brief Emitted on each sample collected.
     */
    void sampleCollected(int sampleCount);
    
private slots:
    void onSampleTick();
    void onEmulatorLost(uint32_t pid);
    
private:
    void collectSample();
    BaselineMetrics computeBaseline();
    bool checkStabilityReached();
    double computeVariance(const std::vector<double>& values);
    
    TelemetryReader* m_telemetry = nullptr;
    EmulatorDetector* m_emulatorDetector = nullptr;
    
    QTimer* m_sampleTimer = nullptr;
    QElapsedTimer m_elapsed;
    
    Config m_config;
    bool m_observing = false;
    uint32_t m_targetPid = 0;
    
    std::vector<Sample> m_samples;
    BaselineMetrics m_baseline;
};

} // namespace Zereca

#endif // ZERECA_OBSERVATION_PHASE_H
