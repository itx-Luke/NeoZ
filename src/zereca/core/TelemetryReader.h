#ifndef ZERECA_TELEMETRY_READER_H
#define ZERECA_TELEMETRY_READER_H

#include "../types/ZerecaTypes.h"
#include <QObject>
#include <QTimer>
#include <QMutex>

namespace Zereca {

/**
 * @brief Aggregated metrics exposed to System B.
 * 
 * CRITICAL: System B never sees raw ETW events.
 * TelemetryReader aggregates kernel telemetry into high-level metrics.
 */
struct AggregatedMetrics {
    // CPU Metrics
    double cpuResidencyPercent = 0.0;     ///< % time in high-perf state
    double contextSwitchRate = 0.0;        ///< Context switches per second
    double coreUtilization = 0.0;          ///< Average core utilization %
    
    // GPU Metrics
    double gpuQueueDepth = 0.0;            ///< Average GPU queue depth
    double gpuUtilization = 0.0;           ///< GPU engine utilization %
    
    // Memory Metrics
    double memoryPressure = 0.0;           ///< 0.0–1.0 (low to high)
    double standbyListSize = 0.0;          ///< Standby list in MB
    
    // Thermal Metrics
    double thermalHeadroomCelsius = 0.0;   ///< Degrees below throttle
    
    // Frame Metrics (from emulator hooks, not ETW)
    double avgFrameTimeMs = 0.0;
    double fpsVariance = 0.0;
    double fps = 0.0;
    
    // Timestamp
    uint64_t timestamp = 0;
};

/**
 * @brief Privilege tier for telemetry collection.
 */
enum class PrivilegeTier {
    Standard,   ///< No elevation - limited telemetry
    Operator    ///< Admin elevation - full ETW access
};

/**
 * @brief Telemetry Reader - Collects and aggregates system metrics.
 * 
 * Implements the ETW isolation principle:
 * - ETW → TelemetryReader → AggregatedMetrics → System B
 * - System B never sees raw kernel events
 * 
 * Tier 1 (Standard Mode):
 * - Process CPU usage, FPS, frame time, performance counters
 * 
 * Tier 2 (Operator Mode):
 * - ETW Kernel Scheduler, DxgKrnl GPU queues, CPU residency
 */
class TelemetryReader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PrivilegeTier tier READ tier NOTIFY tierChanged)
    Q_PROPERTY(bool collecting READ isCollecting NOTIFY collectingChanged)
    
public:
    explicit TelemetryReader(QObject* parent = nullptr);
    ~TelemetryReader() override;
    
    /**
     * @brief Start telemetry collection.
     */
    Q_INVOKABLE void start();
    
    /**
     * @brief Stop telemetry collection.
     */
    Q_INVOKABLE void stop();
    
    /**
     * @brief Check if collection is active.
     */
    bool isCollecting() const { return m_collecting; }
    
    /**
     * @brief Get current privilege tier.
     */
    PrivilegeTier tier() const { return m_tier; }
    
    /**
     * @brief Get the latest aggregated metrics.
     * Thread-safe.
     */
    AggregatedMetrics latestMetrics() const;
    
    /**
     * @brief Check if admin privileges are available.
     */
    static bool hasAdminPrivileges();
    
signals:
    void tierChanged(PrivilegeTier tier);
    void collectingChanged(bool collecting);
    
    /**
     * @brief Emitted when new metrics are available.
     */
    void metricsUpdated(const AggregatedMetrics& metrics);
    
    /**
     * @brief Emitted if privileges are lost at runtime.
     */
    void privilegesLost();
    
private slots:
    void onCollectionTick();
    
private:
    void detectPrivilegeTier();
    void collectStandardMetrics();
    void collectOperatorMetrics();
    
    // Standard tier collection (no elevation)
    double readCpuUsage();
    double readGpuUsage();
    double readMemoryPressure();
    
    // Operator tier collection (requires elevation)
    void startEtwSession();
    void stopEtwSession();
    void processEtwEvents();
    
    QTimer* m_timer = nullptr;
    mutable QMutex m_mutex;
    AggregatedMetrics m_metrics;
    PrivilegeTier m_tier = PrivilegeTier::Standard;
    bool m_collecting = false;
    
    // ETW session handle (Operator mode only)
    void* m_etwSession = nullptr;
};

} // namespace Zereca

#endif // ZERECA_TELEMETRY_READER_H
