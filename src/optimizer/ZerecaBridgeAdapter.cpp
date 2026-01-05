#include "ZerecaBridgeAdapter.h"
#include "OptimizerBackend.h"

#include <QDebug>

ZerecaBridgeAdapter::ZerecaBridgeAdapter(OptimizerBackend* backend, QObject* parent)
    : QObject(parent)
    , m_backend(backend)
    , m_recommendationTimer(new QTimer(this))
{
    // Connect to backend metrics
    connect(m_backend, &OptimizerBackend::metricsChanged, 
            this, &ZerecaBridgeAdapter::onMetricsChanged);
    
    // Periodic recommendation calculation (every 5 seconds)
    m_recommendationTimer->setInterval(5000);
    connect(m_recommendationTimer, &QTimer::timeout, 
            this, &ZerecaBridgeAdapter::calculateRecommendations);
    m_recommendationTimer->start();
    
    qDebug() << "[ZerecaBridge] Initialized";
}

ZerecaBridgeAdapter::~ZerecaBridgeAdapter()
{
    m_recommendationTimer->stop();
}

// ========== Control Methods ==========

void ZerecaBridgeAdapter::applyOptimization(const QString& type)
{
    if (!m_backend) return;
    
    setReconciling(true);
    setStatus("Applying " + type + "...");
    
    QStringList outcomes;
    
    if (type == "CPU" && m_cpuBoostEnabled) {
        m_backend->enableGameMode();
        m_cpuStatus = "Applied";
        outcomes << "CPU boost applied";
    } else if (type == "GPU" && m_gpuBoostEnabled) {
        m_backend->setGpuPreference("HighPerformance");
        m_gpuStatus = "Applied";
        outcomes << "GPU high-performance set";
    } else if (type == "RAM" && m_ramOptEnabled) {
        m_backend->optimizeRam();
        m_ramStatus = "Applied";
        outcomes << "RAM optimized";
    } else if (type == "Power" && m_powerPlanEnabled) {
        m_backend->setPowerPlan("High Performance");
        m_powerStatus = "Applied";
        outcomes << "Power plan set to High Performance";
    }
    
    emit optimizationChanged();
    setOutcome(outcomes.join(", "));
    setReconciling(false);
    setStatus("Idle");
    
    emit toastNotification("✅ " + type + " optimization applied", "success");
}

void ZerecaBridgeAdapter::revertOptimization(const QString& type)
{
    if (!m_backend) return;
    
    setReconciling(true);
    setStatus("Reverting " + type + "...");
    
    QStringList outcomes;
    
    if (type == "CPU") {
        m_backend->disableGameMode();
        m_cpuStatus = "Reverted";
        outcomes << "CPU boost reverted";
    } else if (type == "GPU") {
        m_backend->setGpuPreference("SystemDefault");
        m_gpuStatus = "Reverted";
        outcomes << "GPU set to system default";
    } else if (type == "RAM") {
        m_ramStatus = "Reverted";
        outcomes << "RAM optimization reverted";
    } else if (type == "Power") {
        m_backend->setPowerPlan("Balanced");
        m_powerStatus = "Reverted";
        outcomes << "Power plan set to Balanced";
    }
    
    emit optimizationChanged();
    setOutcome(outcomes.join(", "));
    setReconciling(false);
    setStatus("Idle");
    
    emit toastNotification("↩️ " + type + " reverted", "info");
}

void ZerecaBridgeAdapter::applyAll()
{
    setReconciling(true);
    setStatus("Applying all optimizations...");
    
    QStringList outcomes;
    
    if (m_cpuBoostEnabled) {
        m_backend->enableGameMode();
        m_cpuStatus = "Applied";
        outcomes << "CPU";
    }
    if (m_gpuBoostEnabled) {
        m_backend->setGpuPreference("HighPerformance");
        m_gpuStatus = "Applied";
        outcomes << "GPU";
    }
    if (m_ramOptEnabled) {
        m_backend->optimizeRam();
        m_ramStatus = "Applied";
        outcomes << "RAM";
    }
    if (m_powerPlanEnabled) {
        m_backend->setPowerPlan("High Performance");
        m_powerStatus = "Applied";
        outcomes << "Power";
    }
    if (m_timerResEnabled) {
        m_backend->setTimerResolutionEnabled(true);
        outcomes << "Timer";
    }
    
    emit optimizationChanged();
    setOutcome(outcomes.join(", ") + " optimizations applied");
    setReconciling(false);
    setStatus("Idle");
    
    emit toastNotification("🚀 All optimizations applied", "success");
}

void ZerecaBridgeAdapter::revertAll()
{
    setReconciling(true);
    setStatus("Reverting all optimizations...");
    
    m_backend->disableGameMode();
    m_backend->setGpuPreference("SystemDefault");
    m_backend->setPowerPlan("Balanced");
    m_backend->setTimerResolutionEnabled(false);
    
    m_cpuStatus = "Neutral";
    m_gpuStatus = "Neutral";
    m_ramStatus = "Neutral";
    m_powerStatus = "Neutral";
    
    emit optimizationChanged();
    setOutcome("All optimizations reverted to defaults");
    setReconciling(false);
    setStatus("Idle");
    
    emit toastNotification("🔄 System restored to defaults", "info");
}

void ZerecaBridgeAdapter::clearRecommendations()
{
    m_recommendations.clear();
    emit recommendationsChanged();
}

// ========== Property Getters ==========

double ZerecaBridgeAdapter::cpuUsage() const
{
    return m_backend ? m_backend->cpuUsage() : 0.0;
}

double ZerecaBridgeAdapter::ramUsage() const
{
    return m_backend ? m_backend->ramUsage() : 0.0;
}

double ZerecaBridgeAdapter::diskUsage() const
{
    return m_backend ? m_backend->diskUsage() : 0.0;
}

// ========== Property Setters ==========

void ZerecaBridgeAdapter::setCpuBoostEnabled(bool enabled)
{
    if (m_cpuBoostEnabled != enabled) {
        m_cpuBoostEnabled = enabled;
        emit optionsChanged();
    }
}

void ZerecaBridgeAdapter::setGpuBoostEnabled(bool enabled)
{
    if (m_gpuBoostEnabled != enabled) {
        m_gpuBoostEnabled = enabled;
        emit optionsChanged();
    }
}

void ZerecaBridgeAdapter::setRamOptEnabled(bool enabled)
{
    if (m_ramOptEnabled != enabled) {
        m_ramOptEnabled = enabled;
        emit optionsChanged();
    }
}

void ZerecaBridgeAdapter::setTimerResEnabled(bool enabled)
{
    if (m_timerResEnabled != enabled) {
        m_timerResEnabled = enabled;
        emit optionsChanged();
        
        if (m_backend) {
            m_backend->setTimerResolutionEnabled(enabled);
        }
    }
}

void ZerecaBridgeAdapter::setPowerPlanEnabled(bool enabled)
{
    if (m_powerPlanEnabled != enabled) {
        m_powerPlanEnabled = enabled;
        emit optionsChanged();
    }
}

void ZerecaBridgeAdapter::setAggressiveness(int level)
{
    level = qBound(0, level, 2);
    if (m_aggressiveness != level) {
        m_aggressiveness = level;
        emit aggressivenessChanged(level);
        
        // Recalculate recommendations based on new aggressiveness
        calculateRecommendations();
    }
}

void ZerecaBridgeAdapter::setShowDetailedLog(bool show)
{
    if (m_showDetailedLog != show) {
        m_showDetailedLog = show;
        emit logToggled(show);
    }
}

// ========== Private Slots ==========

void ZerecaBridgeAdapter::onMetricsChanged()
{
    emit metricsUpdated();
}

void ZerecaBridgeAdapter::calculateRecommendations()
{
    if (!m_backend) return;
    
    // Thresholds based on aggressiveness
    double cpuThreshold = m_aggressiveness == 0 ? 90 : (m_aggressiveness == 1 ? 80 : 70);
    double ramThreshold = m_aggressiveness == 0 ? 85 : (m_aggressiveness == 1 ? 75 : 65);
    double diskThreshold = 90;
    
    double cpu = m_backend->cpuUsage();
    double ram = m_backend->ramUsage();
    double disk = m_backend->diskUsage();
    
    // Check CPU
    if (cpu > cpuThreshold && m_cpuBoostEnabled && m_cpuStatus != "Applied") {
        addRecommendation("⚡ CPU usage at " + QString::number(cpu, 'f', 0) + 
                         "%. Enable Game Mode for boost.");
    }
    
    // Check RAM
    if (ram > ramThreshold && m_ramOptEnabled && m_ramStatus != "Applied") {
        addRecommendation("🧹 RAM usage at " + QString::number(ram, 'f', 0) + 
                         "%. Run RAM optimizer.");
    }
    
    // Check Disk
    if (disk > diskThreshold) {
        addRecommendation("💽 Disk usage at " + QString::number(disk, 'f', 0) + 
                         "%. Consider cleanup.");
    }
    
    // Check Power Plan
    if (m_powerPlanEnabled && m_powerStatus != "Applied" && 
        m_backend->powerPlan() != "High Performance") {
        if (m_aggressiveness >= 1) {
            addRecommendation("🔋 Switch to High Performance power plan for gaming.");
        }
    }
}

// ========== Private Helpers ==========

void ZerecaBridgeAdapter::setStatus(const QString& status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void ZerecaBridgeAdapter::setReconciling(bool reconciling)
{
    if (m_reconciling != reconciling) {
        m_reconciling = reconciling;
        emit reconcilingChanged(reconciling);
    }
}

void ZerecaBridgeAdapter::addRecommendation(const QString& message)
{
    // Avoid duplicates
    if (!m_recommendations.contains(message)) {
        m_recommendations.prepend(message);
        
        // Keep max 10 recommendations
        while (m_recommendations.size() > 10) {
            m_recommendations.removeLast();
        }
        
        emit recommendationsChanged();
        emit toastNotification(message, "recommendation");
    }
}

void ZerecaBridgeAdapter::setOutcome(const QString& outcome)
{
    if (m_lastOutcome != outcome) {
        m_lastOutcome = outcome;
        emit outcomeChanged(outcome);
    }
}
