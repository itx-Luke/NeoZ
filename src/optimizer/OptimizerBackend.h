#ifndef OPTIMIZER_BACKEND_H
#define OPTIMIZER_BACKEND_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QTimer>

/**
 * @brief OptimizerBackend - C++ backend for Win10 Optimizer
 * 
 * @deprecated LEGACY: Do not extend. Superseded by Zereca.
 * This class is being replaced by the Zereca control plane.
 * All new optimization logic should go into src/zereca/.
 * 
 * Deprecation Path:
 * - v0.2.x: Parallel (bridge mode with Zereca)
 * - v0.3.x: Zereca default, legacy fallback disabled
 * - v0.4.x: OptimizerBackend removed
 * 
 * Provides all optimization functionality:
 * - System metrics (CPU, RAM, Disk, Network, Temp, Power)
 * - BlueStacks process priority management
 * - RAM & Svchost optimization
 * - FPS boost settings
 * - Power plan management
 * - Service optimization
 */
class OptimizerBackend : public QObject
{
    Q_OBJECT
    
    // System Health
    Q_PROPERTY(int systemHealth READ systemHealth NOTIFY metricsChanged)
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY metricsChanged)
    Q_PROPERTY(double ramUsage READ ramUsage NOTIFY metricsChanged)
    Q_PROPERTY(double diskUsage READ diskUsage NOTIFY metricsChanged)
    Q_PROPERTY(double networkSpeed READ networkSpeed NOTIFY metricsChanged)
    Q_PROPERTY(double cpuTemp READ cpuTemp NOTIFY metricsChanged)
    Q_PROPERTY(double powerDraw READ powerDraw NOTIFY metricsChanged)
    Q_PROPERTY(int totalRamGB READ totalRamGB NOTIFY metricsChanged)
    Q_PROPERTY(int usedRamGB READ usedRamGB NOTIFY metricsChanged)
    
    // BlueStacks Status
    Q_PROPERTY(bool bluestacksRunning READ bluestacksRunning NOTIFY bluestacksChanged)
    Q_PROPERTY(int bluestacksFps READ bluestacksFps NOTIFY bluestacksChanged)
    Q_PROPERTY(bool bluestacksOptimized READ bluestacksOptimized NOTIFY bluestacksChanged)
    
    // Svchost Status
    Q_PROPERTY(int svchostCount READ svchostCount NOTIFY svchostChanged)
    Q_PROPERTY(double svchostRamMB READ svchostRamMB NOTIFY svchostChanged)
    
    // FPS Boost Status
    Q_PROPERTY(bool gameModeActive READ gameModeActive NOTIFY fpsBoostChanged)
    Q_PROPERTY(QString powerPlan READ powerPlan NOTIFY fpsBoostChanged)
    Q_PROPERTY(int estimatedFpsGain READ estimatedFpsGain NOTIFY fpsBoostChanged)
    
    // Active Profile
    Q_PROPERTY(QString activeProfile READ activeProfile WRITE setActiveProfile NOTIFY profileChanged)
    
    // Event Log
    Q_PROPERTY(QVariantList eventLog READ eventLog NOTIFY eventLogChanged)
    
    // Safety & Admin
    Q_PROPERTY(bool adminMode READ hasAdminPrivileges NOTIFY safetyChanged)
    Q_PROPERTY(bool restorePointCreated READ restorePointCreated NOTIFY safetyChanged)
    
    // Cleanup
    Q_PROPERTY(qint64 estimatedCleanupBytes READ estimatedCleanupBytes NOTIFY cleanupChanged)
    
    // Visual Quality Preference
    Q_PROPERTY(bool visualQualityMode READ visualQualityMode WRITE setVisualQualityMode NOTIFY visualQualityChanged)
    
    // Toggle States
    Q_PROPERTY(bool telemetryDisabled READ telemetryDisabled NOTIFY privacyChanged)
    Q_PROPERTY(bool cortanaDisabled READ cortanaDisabled NOTIFY privacyChanged)
    Q_PROPERTY(bool locationDisabled READ locationDisabled NOTIFY privacyChanged)
    
    // ==================== ELITE OPTIMIZATION STATES ====================
    Q_PROPERTY(bool advancedPanelVisible READ advancedPanelVisible WRITE setAdvancedPanelVisible NOTIFY advancedPanelChanged)
    Q_PROPERTY(bool timerResolutionEnabled READ timerResolutionEnabled WRITE setTimerResolutionEnabled NOTIFY eliteOptimizationChanged)
    Q_PROPERTY(bool msiModeEnabled READ msiModeEnabled WRITE setMsiModeEnabled NOTIFY eliteOptimizationChanged)
    Q_PROPERTY(bool hpetDisabled READ hpetDisabled WRITE setHpetDisabled NOTIFY eliteOptimizationChanged)
    Q_PROPERTY(bool spectreDisabled READ spectreDisabled WRITE setSpectreDisabled NOTIFY eliteOptimizationChanged)
    Q_PROPERTY(bool dmaRemappingDisabled READ dmaRemappingDisabled WRITE setDmaRemappingDisabled NOTIFY eliteOptimizationChanged)
    Q_PROPERTY(bool powerThrottlingDisabled READ powerThrottlingDisabled WRITE setPowerThrottlingDisabled NOTIFY eliteOptimizationChanged)
    Q_PROPERTY(QString lastCleanupResult READ lastCleanupResult NOTIFY cleanupChanged)

public:
    explicit OptimizerBackend(QObject* parent = nullptr);
    ~OptimizerBackend();
    
    // Property Getters
    int systemHealth() const { return m_systemHealth; }
    double cpuUsage() const { return m_cpuUsage; }
    double ramUsage() const { return m_ramUsage; }
    double diskUsage() const { return m_diskUsage; }
    double networkSpeed() const { return m_networkSpeed; }
    double cpuTemp() const { return m_cpuTemp; }
    double powerDraw() const { return m_powerDraw; }
    int totalRamGB() const { return m_totalRamGB; }
    int usedRamGB() const { return m_usedRamGB; }
    
    bool bluestacksRunning() const { return m_bluestacksRunning; }
    int bluestacksFps() const { return m_bluestacksFps; }
    bool bluestacksOptimized() const { return m_bluestacksOptimized; }
    
    int svchostCount() const { return m_svchostCount; }
    double svchostRamMB() const { return m_svchostRamMB; }
    
    bool gameModeActive() const { return m_gameModeActive; }
    QString powerPlan() const { return m_powerPlan; }
    int estimatedFpsGain() const { return m_estimatedFpsGain; }
    
    QString activeProfile() const { return m_activeProfile; }
    void setActiveProfile(const QString& profile);
    
    QVariantList eventLog() const { return m_eventLog; }
    
    // Safety getters
    bool hasAdminPrivileges() const;
    bool restorePointCreated() const { return m_restorePointCreated; }
    qint64 estimatedCleanupBytes() const { return m_estimatedCleanupBytes; }
    
    // Visual Quality
    bool visualQualityMode() const { return m_visualQualityMode; }
    void setVisualQualityMode(bool enable);
    
    // Privacy toggle getters
    bool telemetryDisabled() const { return m_telemetryDisabled; }
    bool cortanaDisabled() const { return m_cortanaDisabled; }
    bool locationDisabled() const { return m_locationDisabled; }
    
    // Elite optimization getters
    bool advancedPanelVisible() const { return m_advancedPanelVisible; }
    void setAdvancedPanelVisible(bool visible);
    bool timerResolutionEnabled() const { return m_timerResolutionEnabled; }
    void setTimerResolutionEnabled(bool enable);
    bool msiModeEnabled() const { return m_msiModeEnabled; }
    void setMsiModeEnabled(bool enable);
    bool hpetDisabled() const { return m_hpetDisabled; }
    void setHpetDisabled(bool disable);
    bool spectreDisabled() const { return m_spectreDisabled; }
    void setSpectreDisabled(bool disable);
    bool dmaRemappingDisabled() const { return m_dmaRemappingDisabled; }
    void setDmaRemappingDisabled(bool disable);
    bool powerThrottlingDisabled() const { return m_powerThrottlingDisabled; }
    void setPowerThrottlingDisabled(bool disable);
    QString lastCleanupResult() const { return m_lastCleanupResult; }
    
    // ==================== Q_INVOKABLE METHODS ====================
    
    // --- System Metrics ---
    Q_INVOKABLE void refreshMetrics();
    Q_INVOKABLE QVariantMap getDetailedCpuInfo();
    Q_INVOKABLE QVariantMap getDetailedRamInfo();
    Q_INVOKABLE QVariantMap getDetailedDiskInfo();
    Q_INVOKABLE QVariantMap getDetailedNetworkInfo();
    
    // --- Safety Features ---
    Q_INVOKABLE bool createRestorePoint(const QString& description = "NeoZ Optimizer Backup");
    Q_INVOKABLE void requestAdminElevation();
    
    // --- BlueStacks Optimization ---
    Q_INVOKABLE void optimizeBluestacks();
    Q_INVOKABLE void setBluestacksPriority(const QString& priority);
    Q_INVOKABLE void setGpuPreference(const QString& preference);
    Q_INVOKABLE QVariantList getBluestacksProcesses();
    Q_INVOKABLE void killBluestacksProcess(const QString& processName);
    
    // --- RAM & Svchost Optimization ---
    Q_INVOKABLE void optimizeRam();
    Q_INVOKABLE void clearStandbyMemory();
    Q_INVOKABLE void setSvchostThreshold(int ramSizeGB);
    Q_INVOKABLE void optimizeFileSystemCache();
    Q_INVOKABLE QVariantList getSvchostInstances();
    Q_INVOKABLE void restartSvchostInstance(int pid);
    
    // --- FPS Boost ---
    Q_INVOKABLE void enableGameMode();
    Q_INVOKABLE void disableGameMode();
    Q_INVOKABLE void setPowerPlan(const QString& plan);
    Q_INVOKABLE void disableVisualEffects();
    Q_INVOKABLE void enableVisualEffects();
    Q_INVOKABLE void disableBackgroundApps();
    Q_INVOKABLE void applyNetworkOptimizations();
    Q_INVOKABLE void applyFpsBoostProfile(const QString& profile);
    
    // --- Services Management ---
    Q_INVOKABLE QVariantList getWindowsServices();
    Q_INVOKABLE void setServiceStartType(const QString& serviceName, const QString& startType);
    Q_INVOKABLE void stopService(const QString& serviceName);
    Q_INVOKABLE void startService(const QString& serviceName);
    
    // --- Privacy & Security ---
    Q_INVOKABLE void disableTelemetry();
    Q_INVOKABLE void enableTelemetry();
    Q_INVOKABLE void disableCortana();
    Q_INVOKABLE void enableCortana();
    Q_INVOKABLE void disableLocationTracking();
    Q_INVOKABLE void enableLocationTracking();
    Q_INVOKABLE void disableAdvertisingId();
    Q_INVOKABLE void disableWiFiSense();
    Q_INVOKABLE void clearActivityHistory();
    Q_INVOKABLE int getTelemetryBlockedCount();
    Q_INVOKABLE int getThreatsBlockedToday();
    
    // --- Cleanup ---
    Q_INVOKABLE void cleanTempFiles();
    Q_INVOKABLE void cleanPrefetch();
    Q_INVOKABLE void cleanWindowsUpdateCache();
    Q_INVOKABLE void cleanThumbnailCache();
    Q_INVOKABLE void removeMemoryDumps();
    Q_INVOKABLE qint64 calculateCleanupSize();
    
    // --- Network & Gaming Advanced ---
    Q_INVOKABLE void setFastDNS(bool enable);
    Q_INVOKABLE void disableQoSPacketScheduler();
    Q_INVOKABLE void enableLowLatencyMode();
    Q_INVOKABLE void optimizeMMCSS();
    Q_INVOKABLE void disableGameDVRCompletely();
    Q_INVOKABLE void disablePowerThrottling();
    Q_INVOKABLE void disableFullscreenOptimizations();
    
    // --- Profile Management ---
    Q_INVOKABLE void applyProfile(const QString& profileName);
    Q_INVOKABLE void saveCurrentAsProfile(const QString& profileName);
    Q_INVOKABLE QVariantList getSavedProfiles();
    Q_INVOKABLE void deleteProfile(const QString& profileName);
    
    // --- Category-based Optimization ---
    Q_INVOKABLE void applyCategory(const QString& category);
    Q_INVOKABLE void runOptimizationScript(const QString& scriptName);
    
    // --- One-Click Actions ---
    Q_INVOKABLE void launchGameMode();
    Q_INVOKABLE void runFullSystemScan();
    Q_INVOKABLE void cleanAndOptimizeAll();
    Q_INVOKABLE void restoreDefaults();
    
    // --- Benchmark ---
    Q_INVOKABLE void runBenchmark();
    Q_INVOKABLE void cancelBenchmark();
    
    // ==================== ELITE OPTIMIZATIONS ====================
    // Kernel-level optimizations for pro users
    Q_INVOKABLE void toggleAdvancedPanel();
    Q_INVOKABLE void performDeepCleanup(bool cleanTemp, bool cleanPrefetch, bool cleanLogs, 
                                         bool cleanUpdateCache, bool cleanDumps, bool cleanThumbnails);
    Q_INVOKABLE void applyPreset(const QString& presetName);  // minimal, balanced, aggressive
    Q_INVOKABLE void setEmulatorAffinity(int coreStart, int coreEnd);
    Q_INVOKABLE void optimizeIrqPriority();
    Q_INVOKABLE QVariantMap getEliteOptimizationStatus();
    
signals:
    void metricsChanged();
    void bluestacksChanged();
    void svchostChanged();
    void fpsBoostChanged();
    void profileChanged();
    void eventLogChanged();
    
    // Progress signals
    void optimizationProgress(int percent, const QString& step);
    void scanProgress(int percent, const QString& currentItem);
    void benchmarkProgress(int percent, int currentFps);
    void benchmarkComplete(int beforeFps, int afterFps, double stutterReduction);
    
    // Notification signals
    void optimizationComplete(const QString& summary);
    void warningDetected(const QString& warning);
    void errorOccurred(const QString& error);
    void aiSuggestion(const QString& suggestion, const QString& actionType);
    
    // New signals
    void safetyChanged();
    void privacyChanged();
    void cleanupChanged();
    void cleanupProgress(int percent, const QString& currentItem);
    void visualQualityChanged();
    void advancedPanelChanged();
    void eliteOptimizationChanged();
    void eliteOptimizationWarning(const QString& warning);

private slots:
    void updateMetrics();
    
private:
    void logEvent(const QString& type, const QString& message);
    void detectBluestacks();
    void calculateSystemHealth();
    
    // Metrics
    int m_systemHealth = 85;
    double m_cpuUsage = 0;
    double m_ramUsage = 0;
    double m_diskUsage = 0;
    double m_networkSpeed = 0;
    double m_cpuTemp = 0;
    double m_powerDraw = 0;
    int m_totalRamGB = 16;
    int m_usedRamGB = 8;
    
    // BlueStacks
    bool m_bluestacksRunning = false;
    int m_bluestacksFps = 60;
    bool m_bluestacksOptimized = false;
    
    // Svchost
    int m_svchostCount = 4;
    double m_svchostRamMB = 1200;
    
    // FPS Boost
    bool m_gameModeActive = false;
    QString m_powerPlan = "Balanced";
    int m_estimatedFpsGain = 0;
    
    // Profile
    QString m_activeProfile = "Custom";
    
    // Event Log
    QVariantList m_eventLog;
    
    // Safety & Admin
    bool m_restorePointCreated = false;
    
    // Visual Quality
    bool m_visualQualityMode = true; // Default to high quality
    
    // Privacy states
    bool m_telemetryDisabled = false;
    bool m_cortanaDisabled = false;
    bool m_locationDisabled = false;
    
    // Cleanup
    qint64 m_estimatedCleanupBytes = 0;
    QString m_lastCleanupResult;
    
    // Elite Optimization states
    bool m_advancedPanelVisible = false;
    bool m_timerResolutionEnabled = false;
    bool m_msiModeEnabled = false;
    bool m_hpetDisabled = false;
    bool m_spectreDisabled = false;
    bool m_dmaRemappingDisabled = false;
    bool m_powerThrottlingDisabled = false;
    
    // Timers
    QTimer* m_metricsTimer = nullptr;
};

#endif // OPTIMIZER_BACKEND_H
