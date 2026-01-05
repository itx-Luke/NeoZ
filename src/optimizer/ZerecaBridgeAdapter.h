#ifndef ZERECA_BRIDGE_ADAPTER_H
#define ZERECA_BRIDGE_ADAPTER_H

#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QStringList>
#include <QTimer>

class OptimizerBackend;

/**
 * @brief ZerecaBridgeAdapter - Bridge between OptimizerBackend and Zereca UI
 * 
 * Provides:
 * - Real-time metrics from OptimizerBackend
 * - Calculated recommendations with toast notifications
 * - User-configurable optimization toggles
 * - Aggressiveness levels (Safe/Balanced/Extreme)
 */
class ZerecaBridgeAdapter : public QObject
{
    Q_OBJECT
    
    // ========== Status Properties ==========
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool reconciling READ isReconciling NOTIFY reconcilingChanged)
    
    // ========== Optimization States ==========
    Q_PROPERTY(QString cpuStatus READ cpuStatus NOTIFY optimizationChanged)
    Q_PROPERTY(QString gpuStatus READ gpuStatus NOTIFY optimizationChanged)
    Q_PROPERTY(QString ramStatus READ ramStatus NOTIFY optimizationChanged)
    Q_PROPERTY(QString powerStatus READ powerStatus NOTIFY optimizationChanged)
    
    // ========== Metrics ==========
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY metricsUpdated)
    Q_PROPERTY(double ramUsage READ ramUsage NOTIFY metricsUpdated)
    Q_PROPERTY(double diskUsage READ diskUsage NOTIFY metricsUpdated)
    Q_PROPERTY(double gpuUsage READ gpuUsage NOTIFY metricsUpdated)
    
    // ========== User Options (Toggles) ==========
    Q_PROPERTY(bool cpuBoostEnabled READ cpuBoostEnabled WRITE setCpuBoostEnabled NOTIFY optionsChanged)
    Q_PROPERTY(bool gpuBoostEnabled READ gpuBoostEnabled WRITE setGpuBoostEnabled NOTIFY optionsChanged)
    Q_PROPERTY(bool ramOptEnabled READ ramOptEnabled WRITE setRamOptEnabled NOTIFY optionsChanged)
    Q_PROPERTY(bool timerResEnabled READ timerResEnabled WRITE setTimerResEnabled NOTIFY optionsChanged)
    Q_PROPERTY(bool powerPlanEnabled READ powerPlanEnabled WRITE setPowerPlanEnabled NOTIFY optionsChanged)
    
    // ========== Aggressiveness ==========
    Q_PROPERTY(int aggressiveness READ aggressiveness WRITE setAggressiveness NOTIFY aggressivenessChanged)
    
    // ========== Recommendations & Log ==========
    Q_PROPERTY(QStringList recommendations READ recommendations NOTIFY recommendationsChanged)
    Q_PROPERTY(QString lastOutcome READ lastOutcome NOTIFY outcomeChanged)
    Q_PROPERTY(bool showDetailedLog READ showDetailedLog WRITE setShowDetailedLog NOTIFY logToggled)

public:
    explicit ZerecaBridgeAdapter(OptimizerBackend* backend, QObject* parent = nullptr);
    ~ZerecaBridgeAdapter() override;
    
    // ========== Control Methods ==========
    Q_INVOKABLE void applyOptimization(const QString& type);
    Q_INVOKABLE void revertOptimization(const QString& type);
    Q_INVOKABLE void applyAll();
    Q_INVOKABLE void revertAll();
    Q_INVOKABLE void clearRecommendations();
    
    // ========== Property Getters ==========
    QString status() const { return m_status; }
    bool isReconciling() const { return m_reconciling; }
    
    QString cpuStatus() const { return m_cpuStatus; }
    QString gpuStatus() const { return m_gpuStatus; }
    QString ramStatus() const { return m_ramStatus; }
    QString powerStatus() const { return m_powerStatus; }
    
    double cpuUsage() const;
    double ramUsage() const;
    double diskUsage() const;
    double gpuUsage() const { return m_gpuUsage; }
    
    bool cpuBoostEnabled() const { return m_cpuBoostEnabled; }
    bool gpuBoostEnabled() const { return m_gpuBoostEnabled; }
    bool ramOptEnabled() const { return m_ramOptEnabled; }
    bool timerResEnabled() const { return m_timerResEnabled; }
    bool powerPlanEnabled() const { return m_powerPlanEnabled; }
    
    int aggressiveness() const { return m_aggressiveness; }
    
    QStringList recommendations() const { return m_recommendations; }
    QString lastOutcome() const { return m_lastOutcome; }
    bool showDetailedLog() const { return m_showDetailedLog; }
    
    // ========== Property Setters ==========
    void setCpuBoostEnabled(bool enabled);
    void setGpuBoostEnabled(bool enabled);
    void setRamOptEnabled(bool enabled);
    void setTimerResEnabled(bool enabled);
    void setPowerPlanEnabled(bool enabled);
    void setAggressiveness(int level);
    void setShowDetailedLog(bool show);

signals:
    void statusChanged(const QString& status);
    void reconcilingChanged(bool reconciling);
    void optimizationChanged();
    void metricsUpdated();
    void optionsChanged();
    void aggressivenessChanged(int level);
    void recommendationsChanged();
    void outcomeChanged(const QString& outcome);
    void logToggled(bool show);
    void toastNotification(const QString& message, const QString& type);

private slots:
    void onMetricsChanged();
    void calculateRecommendations();

private:
    void setStatus(const QString& status);
    void setReconciling(bool reconciling);
    void addRecommendation(const QString& message);
    void setOutcome(const QString& outcome);
    
    OptimizerBackend* m_backend;
    QTimer* m_recommendationTimer;
    
    // Status
    QString m_status = "Idle";
    bool m_reconciling = false;
    
    // Optimization states: "Applied", "Reverted", "Neutral"
    QString m_cpuStatus = "Neutral";
    QString m_gpuStatus = "Neutral";
    QString m_ramStatus = "Neutral";
    QString m_powerStatus = "Neutral";
    
    // GPU usage (simulated for now)
    double m_gpuUsage = 0.0;
    
    // User options
    bool m_cpuBoostEnabled = true;
    bool m_gpuBoostEnabled = true;
    bool m_ramOptEnabled = true;
    bool m_timerResEnabled = false;
    bool m_powerPlanEnabled = true;
    
    // Aggressiveness: 0=Safe, 1=Balanced, 2=Extreme
    int m_aggressiveness = 1;
    
    // Recommendations
    QStringList m_recommendations;
    QString m_lastOutcome;
    bool m_showDetailedLog = false;
};

#endif // ZERECA_BRIDGE_ADAPTER_H
