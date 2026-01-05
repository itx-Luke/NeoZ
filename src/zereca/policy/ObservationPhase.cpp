#include "ObservationPhase.h"
#include <QDebug>
#include <numeric>
#include <cmath>

namespace Zereca {

ObservationPhase::ObservationPhase(TelemetryReader* telemetry,
                                     EmulatorDetector* emulatorDetector,
                                     QObject* parent)
    : QObject(parent)
    , m_telemetry(telemetry)
    , m_emulatorDetector(emulatorDetector)
{
    m_sampleTimer = new QTimer(this);
    connect(m_sampleTimer, &QTimer::timeout, this, &ObservationPhase::onSampleTick);
    
    if (m_emulatorDetector) {
        connect(m_emulatorDetector, &EmulatorDetector::emulatorLost,
                this, &ObservationPhase::onEmulatorLost);
    }
}

ObservationPhase::~ObservationPhase()
{
    stop();
}

void ObservationPhase::start(uint32_t targetPid)
{
    if (m_observing) {
        qWarning() << "[Zereca] ObservationPhase already in progress";
        return;
    }
    
    m_targetPid = targetPid;
    m_samples.clear();
    m_samples.reserve(m_config.maxDurationMs / m_config.sampleIntervalMs);
    m_baseline = BaselineMetrics();
    
    m_elapsed.start();
    m_observing = true;
    
    m_sampleTimer->start(static_cast<int>(m_config.sampleIntervalMs));
    
    qDebug() << "[Zereca] ObservationPhase started for PID:" << targetPid;
    emit observingChanged(true);
}

void ObservationPhase::stop()
{
    if (!m_observing) return;
    
    m_sampleTimer->stop();
    m_observing = false;
    
    qDebug() << "[Zereca] ObservationPhase stopped,"
             << m_samples.size() << "samples collected";
    
    emit observingChanged(false);
}

float ObservationPhase::progress() const
{
    if (!m_observing) return 0.0f;
    
    qint64 elapsed = m_elapsed.elapsed();
    return std::min(1.0f, static_cast<float>(elapsed) / m_config.maxDurationMs);
}

int ObservationPhase::elapsedMs() const
{
    if (!m_observing) return 0;
    return static_cast<int>(m_elapsed.elapsed());
}

void ObservationPhase::onSampleTick()
{
    collectSample();
    emit progressChanged(progress());
    emit sampleCollected(static_cast<int>(m_samples.size()));
    
    qint64 elapsed = m_elapsed.elapsed();
    
    // Check for early exit due to stability
    if (m_samples.size() >= static_cast<size_t>(m_config.minSamplesForStability) &&
        elapsed >= static_cast<qint64>(m_config.minDurationMs)) {
        if (checkStabilityReached()) {
            qDebug() << "[Zereca] ObservationPhase: stability reached, completing early";
            m_baseline = computeBaseline();
            stop();
            emit observationComplete(m_baseline);
            return;
        }
    }
    
    // Check for max duration
    if (elapsed >= static_cast<qint64>(m_config.maxDurationMs)) {
        qDebug() << "[Zereca] ObservationPhase: max duration reached";
        m_baseline = computeBaseline();
        stop();
        emit observationComplete(m_baseline);
    }
}

void ObservationPhase::onEmulatorLost(uint32_t pid)
{
    if (m_observing && pid == m_targetPid) {
        qWarning() << "[Zereca] ObservationPhase: target emulator exited";
        stop();
        emit observationFailed("Target emulator exited during observation");
    }
}

void ObservationPhase::collectSample()
{
    if (!m_telemetry) return;
    
    AggregatedMetrics metrics = m_telemetry->latestMetrics();
    
    Sample sample;
    sample.timestamp = metrics.timestamp;
    sample.fps = metrics.fps;
    sample.frameTimeMs = metrics.avgFrameTimeMs;
    sample.cpuUsage = metrics.coreUtilization;
    sample.gpuUsage = metrics.gpuUtilization;
    sample.memoryPressure = metrics.memoryPressure;
    
    m_samples.push_back(sample);
}

BaselineMetrics ObservationPhase::computeBaseline()
{
    BaselineMetrics baseline;
    
    if (m_samples.empty()) {
        return baseline;
    }
    
    // Collect FPS values
    std::vector<double> fpsValues;
    std::vector<double> frameTimeValues;
    std::vector<double> cpuValues;
    std::vector<double> gpuValues;
    std::vector<double> memValues;
    
    for (const auto& sample : m_samples) {
        if (sample.fps > 0) fpsValues.push_back(sample.fps);
        if (sample.frameTimeMs > 0) frameTimeValues.push_back(sample.frameTimeMs);
        cpuValues.push_back(sample.cpuUsage);
        gpuValues.push_back(sample.gpuUsage);
        memValues.push_back(sample.memoryPressure);
    }
    
    // Compute averages
    auto avg = [](const std::vector<double>& v) {
        if (v.empty()) return 0.0;
        return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    };
    
    baseline.fps = avg(fpsValues);
    baseline.avgFrameTime = avg(frameTimeValues);
    baseline.fpsVariance = computeVariance(fpsValues);
    baseline.cpuResidency = avg(cpuValues);
    baseline.gpuQueueDepth = avg(gpuValues);  // Using as proxy
    baseline.memoryPressure = avg(memValues);
    baseline.observationDurationMs = m_elapsed.elapsed();
    
    // Thermal headroom (placeholder - would need actual thermal data)
    baseline.thermalHeadroom = 20.0;  // Assume 20°C headroom
    
    qDebug() << "[Zereca] Baseline computed:"
             << "FPS:" << baseline.fps
             << "FrameTime:" << baseline.avgFrameTime << "ms"
             << "Variance:" << baseline.fpsVariance;
    
    return baseline;
}

bool ObservationPhase::checkStabilityReached()
{
    if (m_samples.size() < 30) return false;
    
    // Take last 30 samples and check variance
    std::vector<double> recentFps;
    for (size_t i = m_samples.size() - 30; i < m_samples.size(); i++) {
        if (m_samples[i].fps > 0) {
            recentFps.push_back(m_samples[i].fps);
        }
    }
    
    if (recentFps.empty()) return false;
    
    double variance = computeVariance(recentFps);
    double mean = std::accumulate(recentFps.begin(), recentFps.end(), 0.0) / recentFps.size();
    
    // Coefficient of variation < threshold means stable
    if (mean > 0) {
        double cv = std::sqrt(variance) / mean;
        return cv < m_config.stabilityThreshold;
    }
    
    return false;
}

double ObservationPhase::computeVariance(const std::vector<double>& values)
{
    if (values.size() < 2) return 0.0;
    
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double variance = 0.0;
    
    for (double v : values) {
        variance += (v - mean) * (v - mean);
    }
    
    return variance / (values.size() - 1);
}

} // namespace Zereca
