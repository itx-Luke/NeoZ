#include "TelemetryReader.h"
#include <QDateTime>
#include <QDebug>
#include <QMutexLocker>

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#include <psapi.h>
#pragma comment(lib, "pdh.lib")
#endif

namespace Zereca {

TelemetryReader::TelemetryReader(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->setInterval(500);  // 2Hz collection rate
    connect(m_timer, &QTimer::timeout, this, &TelemetryReader::onCollectionTick);
    
    detectPrivilegeTier();
}

TelemetryReader::~TelemetryReader()
{
    stop();
}

void TelemetryReader::start()
{
    if (m_collecting) return;
    
    detectPrivilegeTier();
    
    if (m_tier == PrivilegeTier::Operator) {
        startEtwSession();
    }
    
    m_collecting = true;
    m_timer->start();
    
    qDebug() << "[Zereca] TelemetryReader started, tier:" 
             << (m_tier == PrivilegeTier::Operator ? "Operator" : "Standard");
    
    emit collectingChanged(true);
}

void TelemetryReader::stop()
{
    if (!m_collecting) return;
    
    m_timer->stop();
    
    if (m_tier == PrivilegeTier::Operator) {
        stopEtwSession();
    }
    
    m_collecting = false;
    
    qDebug() << "[Zereca] TelemetryReader stopped";
    emit collectingChanged(false);
}

AggregatedMetrics TelemetryReader::latestMetrics() const
{
    QMutexLocker locker(&m_mutex);
    return m_metrics;
}

bool TelemetryReader::hasAdminPrivileges()
{
#ifdef Q_OS_WIN
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    return isAdmin;
#else
    return false;
#endif
}

void TelemetryReader::detectPrivilegeTier()
{
    PrivilegeTier oldTier = m_tier;
    m_tier = hasAdminPrivileges() ? PrivilegeTier::Operator : PrivilegeTier::Standard;
    
    if (oldTier != m_tier) {
        if (oldTier == PrivilegeTier::Operator && m_tier == PrivilegeTier::Standard) {
            qWarning() << "[Zereca] Privileges lost at runtime!";
            emit privilegesLost();
        }
        emit tierChanged(m_tier);
    }
}

void TelemetryReader::onCollectionTick()
{
    // Re-check privileges periodically
    detectPrivilegeTier();
    
    QMutexLocker locker(&m_mutex);
    
    m_metrics.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    // Standard tier metrics (always available)
    collectStandardMetrics();
    
    // Operator tier metrics (if elevated)
    if (m_tier == PrivilegeTier::Operator) {
        collectOperatorMetrics();
    }
    
    locker.unlock();
    emit metricsUpdated(m_metrics);
}

void TelemetryReader::collectStandardMetrics()
{
    m_metrics.coreUtilization = readCpuUsage();
    m_metrics.gpuUtilization = readGpuUsage();
    m_metrics.memoryPressure = readMemoryPressure();
}

void TelemetryReader::collectOperatorMetrics()
{
    // Process ETW events from ring buffer
    processEtwEvents();
}

double TelemetryReader::readCpuUsage()
{
#ifdef Q_OS_WIN
    static ULARGE_INTEGER lastIdleTime = {0};
    static ULARGE_INTEGER lastKernelTime = {0};
    static ULARGE_INTEGER lastUserTime = {0};
    
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 0.0;
    }
    
    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idleTime.dwLowDateTime;
    idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;
    
    ULONGLONG sysTime = (kernel.QuadPart - lastKernelTime.QuadPart) +
                        (user.QuadPart - lastUserTime.QuadPart);
    ULONGLONG idleDelta = idle.QuadPart - lastIdleTime.QuadPart;
    
    lastIdleTime = idle;
    lastKernelTime = kernel;
    lastUserTime = user;
    
    if (sysTime == 0) return 0.0;
    
    return 100.0 * (1.0 - static_cast<double>(idleDelta) / sysTime);
#else
    return 0.0;
#endif
}

double TelemetryReader::readGpuUsage()
{
#ifdef Q_OS_WIN
    // Use DXGI to query GPU engine utilization
    // Simplified: return 0 for now, full implementation requires D3D11 query
    return 0.0;
#else
    return 0.0;
#endif
}

double TelemetryReader::readMemoryPressure()
{
#ifdef Q_OS_WIN
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    
    if (!GlobalMemoryStatusEx(&memInfo)) {
        return 0.0;
    }
    
    // Memory pressure = used / total (0.0–1.0)
    return 1.0 - (static_cast<double>(memInfo.ullAvailPhys) / memInfo.ullTotalPhys);
#else
    return 0.0;
#endif
}

void TelemetryReader::startEtwSession()
{
#ifdef Q_OS_WIN
    // ETW session setup would go here
    // This requires significant Windows-specific code
    // For now, we mark it as a TODO
    qDebug() << "[Zereca] ETW session start (stub)";
#endif
}

void TelemetryReader::stopEtwSession()
{
#ifdef Q_OS_WIN
    if (m_etwSession) {
        // Clean up ETW session
        qDebug() << "[Zereca] ETW session stop";
        m_etwSession = nullptr;
    }
#endif
}

void TelemetryReader::processEtwEvents()
{
    // Process events from ETW ring buffer and aggregate
    // This populates:
    // - cpuResidencyPercent
    // - contextSwitchRate
    // - gpuQueueDepth
    // - thermalHeadroomCelsius
    
    // Stub implementation for now
}

} // namespace Zereca
