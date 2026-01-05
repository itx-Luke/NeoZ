#include "ShadowMode.h"
#include <QDebug>
#include <numeric>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Zereca {

ShadowMode::ShadowMode(TelemetryReader* telemetry,
                         EmulatorDetector* detector,
                         QObject* parent)
    : QObject(parent)
    , m_telemetry(telemetry)
    , m_emulatorDetector(detector)
{
    m_trialTimer = new QTimer(this);
    m_trialTimer->setSingleShot(true);
    connect(m_trialTimer, &QTimer::timeout, this, &ShadowMode::onTrialEnd);
    
    m_tickTimer = new QTimer(this);
    connect(m_tickTimer, &QTimer::timeout, this, &ShadowMode::onTrialTick);
    
    if (m_emulatorDetector) {
        connect(m_emulatorDetector, &EmulatorDetector::emulatorLost,
                this, &ShadowMode::onEmulatorLost);
    }
}

ShadowMode::~ShadowMode()
{
    if (m_active) {
        abortTrial();
    }
}

bool ShadowMode::startTrial(const OptimizationProposal& proposal, uint32_t targetPid)
{
    if (m_active) {
        qWarning() << "[Zereca] ShadowMode: trial already in progress";
        return false;
    }
    
    if (!canShadowTest(proposal.type)) {
        qWarning() << "[Zereca] ShadowMode: change type not allowed for shadow testing";
        return false;
    }
    
    m_currentProposal = proposal;
    m_currentPid = targetPid;
    m_trialSamples.clear();
    
    // Collect "before" metrics
    m_beforeMetrics = collectMetrics();
    
    // Apply the change
    if (!applyChange(proposal, targetPid)) {
        qWarning() << "[Zereca] ShadowMode: failed to apply change";
        return false;
    }
    
    m_active = true;
    m_elapsed.start();
    
    // Start timers
    m_trialTimer->start(static_cast<int>(m_config.trialDurationMs));
    m_tickTimer->start(500);  // Sample every 500ms
    
    qDebug() << "[Zereca] ShadowMode: trial started for PID:" << targetPid
             << "type:" << static_cast<int>(proposal.type);
    
    emit activeChanged(true);
    return true;
}

void ShadowMode::abortTrial()
{
    if (!m_active) return;
    
    m_trialTimer->stop();
    m_tickTimer->stop();
    
    // Revert the change
    revertChange();
    
    m_active = false;
    
    qDebug() << "[Zereca] ShadowMode: trial aborted";
    emit activeChanged(false);
    emit trialAborted("User requested abort");
}

bool ShadowMode::canShadowTest(ChangeType type)
{
    // Per spec: only process-scoped, reversible, low-risk changes
    switch (type) {
        case ChangeType::PRIORITY:
        case ChangeType::AFFINITY:
        case ChangeType::IO_PRIORITY:
            return true;
        
        case ChangeType::TIMER:
        case ChangeType::POWER_PLAN:
        case ChangeType::HPET:
            return false;  // System-wide, not allowed
    }
    return false;
}

void ShadowMode::onTrialTick()
{
    if (!m_active) return;
    
    // Skip first few ticks for stabilization
    if (m_elapsed.elapsed() < static_cast<qint64>(m_config.stabilizationMs)) {
        return;
    }
    
    // Collect sample
    if (m_telemetry) {
        m_trialSamples.push_back(m_telemetry->latestMetrics());
    }
    
    // Calculate current delta
    float currentDelta = 0.0f;
    if (!m_trialSamples.empty() && m_beforeMetrics.fps > 0) {
        double avgFps = 0.0;
        for (const auto& s : m_trialSamples) {
            avgFps += s.fps;
        }
        avgFps /= m_trialSamples.size();
        currentDelta = static_cast<float>((avgFps - m_beforeMetrics.fps) / m_beforeMetrics.fps);
    }
    
    float progress = static_cast<float>(m_elapsed.elapsed()) / m_config.trialDurationMs;
    emit trialProgress(progress, currentDelta);
}

void ShadowMode::onTrialEnd()
{
    if (!m_active) return;
    
    m_tickTimer->stop();
    
    // Revert the change first
    revertChange();
    
    // Compute result
    ShadowTrialResult result;
    result.proposal = m_currentProposal;
    result.beforeMetrics = m_beforeMetrics;
    result.durationMs = m_elapsed.elapsed();
    result.completed = true;
    
    // Compute "after" metrics from samples
    if (!m_trialSamples.empty()) {
        result.afterMetrics.fps = 0;
        result.afterMetrics.avgFrameTime = 0;
        result.afterMetrics.fpsVariance = 0;
        
        for (const auto& s : m_trialSamples) {
            result.afterMetrics.fps += s.fps;
            result.afterMetrics.avgFrameTime += s.avgFrameTimeMs;
        }
        result.afterMetrics.fps /= m_trialSamples.size();
        result.afterMetrics.avgFrameTime /= m_trialSamples.size();
        result.afterMetrics.observationDurationMs = result.durationMs;
        
        // Compute variance
        double mean = result.afterMetrics.fps;
        double variance = 0.0;
        for (const auto& s : m_trialSamples) {
            variance += (s.fps - mean) * (s.fps - mean);
        }
        if (m_trialSamples.size() > 1) {
            variance /= (m_trialSamples.size() - 1);
        }
        result.afterMetrics.fpsVariance = variance;
        
        // Compute performance delta
        if (m_beforeMetrics.fps > 0) {
            result.performanceDelta = static_cast<float>(
                (result.afterMetrics.fps - m_beforeMetrics.fps) / m_beforeMetrics.fps);
        }
    }
    
    m_lastResult = result;
    m_trialCount++;
    m_active = false;
    
    qDebug() << "[Zereca] ShadowMode: trial complete, delta:" << result.performanceDelta;
    
    emit activeChanged(false);
    emit trialComplete(result);
}

void ShadowMode::onEmulatorLost(uint32_t pid)
{
    if (m_active && pid == m_currentPid) {
        qWarning() << "[Zereca] ShadowMode: target emulator exited during trial";
        
        m_trialTimer->stop();
        m_tickTimer->stop();
        
        m_active = false;
        
        m_lastResult = ShadowTrialResult();
        m_lastResult.proposal = m_currentProposal;
        m_lastResult.completed = false;
        m_lastResult.failureReason = "Target emulator exited";
        
        emit activeChanged(false);
        emit trialAborted("Target emulator exited");
    }
}

bool ShadowMode::applyChange(const OptimizationProposal& proposal, uint32_t pid)
{
#ifdef Q_OS_WIN
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION,
                                   FALSE, pid);
    if (!hProcess) {
        return false;
    }
    
    bool success = false;
    
    switch (proposal.type) {
        case ChangeType::PRIORITY: {
            // Save original priority
            m_originalValue = GetPriorityClass(hProcess);
            // Set new priority
            success = SetPriorityClass(hProcess, static_cast<DWORD>(proposal.proposedValue));
            break;
        }
        
        case ChangeType::AFFINITY: {
            DWORD_PTR procAffinity, sysAffinity;
            GetProcessAffinityMask(hProcess, &procAffinity, &sysAffinity);
            m_originalValue = procAffinity;
            
            DWORD_PTR newAffinity;
            if (proposal.proposedValue == 1) {
                // Gold cores (top half)
                SYSTEM_INFO si;
                GetSystemInfo(&si);
                newAffinity = 0;
                for (DWORD i = si.dwNumberOfProcessors / 2; i < si.dwNumberOfProcessors; i++) {
                    newAffinity |= (1ULL << i);
                }
            } else {
                newAffinity = sysAffinity;  // All cores
            }
            success = SetProcessAffinityMask(hProcess, newAffinity);
            break;
        }
        
        case ChangeType::IO_PRIORITY: {
            // IO priority is per-thread, not per-process
            // This is a simplified implementation
            m_originalValue = 1;  // Normal
            success = true;
            break;
        }
        
        default:
            break;
    }
    
    CloseHandle(hProcess);
    return success;
#else
    Q_UNUSED(proposal);
    Q_UNUSED(pid);
    return false;
#endif
}

bool ShadowMode::revertChange()
{
#ifdef Q_OS_WIN
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, m_currentPid);
    if (!hProcess) {
        return false;
    }
    
    bool success = false;
    
    switch (m_currentProposal.type) {
        case ChangeType::PRIORITY:
            success = SetPriorityClass(hProcess, static_cast<DWORD>(m_originalValue));
            break;
        
        case ChangeType::AFFINITY:
            success = SetProcessAffinityMask(hProcess, m_originalValue);
            break;
        
        case ChangeType::IO_PRIORITY:
            success = true;  // Simplified
            break;
        
        default:
            break;
    }
    
    CloseHandle(hProcess);
    
    qDebug() << "[Zereca] ShadowMode: reverted change";
    return success;
#else
    return false;
#endif
}

BaselineMetrics ShadowMode::collectMetrics()
{
    BaselineMetrics m;
    
    if (m_telemetry) {
        AggregatedMetrics current = m_telemetry->latestMetrics();
        m.fps = current.fps;
        m.avgFrameTime = current.avgFrameTimeMs;
        m.fpsVariance = 0;  // Single sample, no variance
        m.cpuResidency = current.coreUtilization;
        m.memoryPressure = current.memoryPressure;
    }
    
    return m;
}

} // namespace Zereca
