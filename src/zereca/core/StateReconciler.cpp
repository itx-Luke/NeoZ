#include "StateReconciler.h"
#include <QDateTime>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>   // For CreateToolhelp32Snapshot, PROCESSENTRY32, etc.
#include <powrprof.h>
#pragma comment(lib, "PowrProf.lib")
#endif

namespace Zereca {

StateReconciler::StateReconciler(TargetStateManager* targetState, QObject* parent)
    : QObject(parent)
    , m_targetState(targetState)
{
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &StateReconciler::onReconciliationTick);
    
    // Listen for target state changes to trigger immediate reconciliation
    if (m_targetState) {
        connect(m_targetState, &TargetStateManager::stateChanged,
                this, &StateReconciler::reconcileNow);
    }
}

StateReconciler::~StateReconciler()
{
    stop();
}

void StateReconciler::start()
{
    if (m_running) return;
    
    m_running = true;
    m_timer->start(m_intervalMs);
    qDebug() << "[Zereca] StateReconciler started (interval:" << m_intervalMs << "ms)";
    
    // Immediate first reconciliation
    reconcileNow();
    
    emit runningChanged(true);
}

void StateReconciler::stop()
{
    if (!m_running) return;
    
    m_running = false;
    m_timer->stop();
    qDebug() << "[Zereca] StateReconciler stopped";
    
    emit runningChanged(false);
}

void StateReconciler::reconcileNow()
{
    if (!m_targetState) {
        emit reconciliationError("No TargetStateManager configured");
        return;
    }
    
    onReconciliationTick();
}

void StateReconciler::setIntervalMs(int ms)
{
    // Clamp to valid range (1-5 seconds per spec)
    ms = qBound(1000, ms, 5000);
    
    if (m_intervalMs == ms) return;
    
    m_intervalMs = ms;
    
    if (m_running) {
        m_timer->setInterval(m_intervalMs);
    }
    
    emit intervalChanged(m_intervalMs);
}

void StateReconciler::onReconciliationTick()
{
    if (!m_targetState) return;
    
    // Step 1: Read current OS state
    m_currentState = readCurrentState();
    
    // Step 2: Get target state
    const TargetState& target = m_targetState->current();
    
    // Step 3: Compare and enforce
    int changesApplied = enforceState(target, m_currentState);
    
    emit reconciliationComplete(changesApplied);
}

CurrentState StateReconciler::readCurrentState()
{
    CurrentState state;
    state.timestamp = QDateTime::currentMSecsSinceEpoch();
    state.powerMode = readCurrentPowerMode();
    state.timerResolution = readTimerResolution();
    state.cpuParking = readCpuParkingEnabled();
    // Note: Process affinity is read per-process during enforcement
    return state;
}

QString StateReconciler::readCurrentPowerMode()
{
#ifdef Q_OS_WIN
    GUID* activeScheme = nullptr;
    if (PowerGetActiveScheme(nullptr, &activeScheme) != ERROR_SUCCESS) {
        return "unknown";
    }
    
    // Known power scheme GUIDs
    static const GUID GUID_HIGH_PERF = {0x8c5e7fda, 0xe8bf, 0x4a96, {0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8c, 0x63, 0x5c}};
    static const GUID GUID_BALANCED = {0x381b4222, 0xf694, 0x41f0, {0x96, 0x85, 0xff, 0x5b, 0xb2, 0x60, 0xdf, 0x2e}};
    static const GUID GUID_POWER_SAVER = {0xa1841308, 0x3541, 0x4fab, {0xbc, 0x81, 0xf7, 0x15, 0x56, 0xf2, 0x0b, 0x4a}};
    
    QString result;
    if (IsEqualGUID(*activeScheme, GUID_HIGH_PERF)) {
        result = "performance";
    } else if (IsEqualGUID(*activeScheme, GUID_BALANCED)) {
        result = "balanced";
    } else if (IsEqualGUID(*activeScheme, GUID_POWER_SAVER)) {
        result = "power_saver";
    } else {
        result = "custom";
    }
    
    LocalFree(activeScheme);
    return result;
#else
    return "balanced";
#endif
}

QString StateReconciler::readTimerResolution()
{
#ifdef Q_OS_WIN
    // Read current timer resolution via NtQueryTimerResolution
    typedef NTSTATUS(WINAPI* NtQueryTimerResolutionPtr)(PULONG, PULONG, PULONG);
    
    static auto NtQueryTimerResolution = reinterpret_cast<NtQueryTimerResolutionPtr>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryTimerResolution"));
    
    if (!NtQueryTimerResolution) {
        return "unknown";
    }
    
    ULONG minRes, maxRes, currentRes;
    if (NtQueryTimerResolution(&minRes, &maxRes, &currentRes) == 0) {
        // Resolution is in 100ns units (10000 = 1ms)
        if (currentRes <= 5000) {
            return "0.5ms";
        } else if (currentRes <= 10000) {
            return "1ms";
        } else {
            return "default";  // ~15.6ms default
        }
    }
    
    return "unknown";
#else
    return "default";
#endif
}

bool StateReconciler::readCpuParkingEnabled()
{
#ifdef Q_OS_WIN
    // CPU parking is controlled by power plan settings
    // Check the CPMINCORES setting
    GUID* activeScheme = nullptr;
    if (PowerGetActiveScheme(nullptr, &activeScheme) != ERROR_SUCCESS) {
        return true;  // Assume enabled on error
    }
    
    // Processor settings subgroup GUID
    static const GUID GUID_PROCESSOR = {0x54533251, 0x82be, 0x4824, {0x96, 0xc1, 0x47, 0xb6, 0x0b, 0x74, 0x0d, 0x00}};
    // CPMINCORES (Core Parking Min Cores) setting GUID
    static const GUID GUID_CPMINCORES = {0x0cc5b647, 0xc1df, 0x4637, {0x89, 0x1a, 0xde, 0xc3, 0x5c, 0x31, 0x85, 0x83}};
    
    DWORD acValue = 0;
    DWORD size = sizeof(acValue);
    
    DWORD result = PowerReadACValueIndex(nullptr, activeScheme,
                                          &GUID_PROCESSOR, &GUID_CPMINCORES, &acValue);
    
    LocalFree(activeScheme);
    
    if (result == ERROR_SUCCESS) {
        // If min cores is 100%, parking is effectively disabled
        return acValue < 100;
    }
    
    return true;  // Assume enabled on error
#else
    return true;
#endif
}

int StateReconciler::enforceState(const TargetState& target, const CurrentState& current)
{
    int changes = 0;
    
    // Enforce power mode
    if (target.powerMode != current.powerMode && current.powerMode != "unknown") {
        if (enforcePowerMode(target.powerMode)) {
            emit driftDetected("power_mode", target.powerMode, current.powerMode);
            m_driftCount++;
            changes++;
        }
    }
    
    // Enforce timer resolution
    if (target.timerResolution != current.timerResolution && current.timerResolution != "unknown") {
        if (enforceTimerResolution(target.timerResolution)) {
            emit driftDetected("timer_resolution", target.timerResolution, current.timerResolution);
            m_driftCount++;
            changes++;
        }
    }
    
    // Enforce CPU parking
    if (target.cpuParking != current.cpuParking) {
        if (enforceCpuParking(target.cpuParking)) {
            emit driftDetected("cpu_parking", 
                              target.cpuParking ? "enabled" : "disabled",
                              current.cpuParking ? "enabled" : "disabled");
            m_driftCount++;
            changes++;
        }
    }
    
    // Enforce process affinity for each configured process
    for (auto it = target.processAffinity.constBegin(); 
         it != target.processAffinity.constEnd(); ++it) {
        if (enforceProcessAffinity(it.key(), it.value())) {
            changes++;
        }
    }
    
    return changes;
}

bool StateReconciler::enforcePowerMode(const QString& mode)
{
#ifdef Q_OS_WIN
    GUID schemeGuid;
    
    if (mode == "performance") {
        schemeGuid = {0x8c5e7fda, 0xe8bf, 0x4a96, {0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8c, 0x63, 0x5c}};
    } else if (mode == "balanced") {
        schemeGuid = {0x381b4222, 0xf694, 0x41f0, {0x96, 0x85, 0xff, 0x5b, 0xb2, 0x60, 0xdf, 0x2e}};
    } else if (mode == "power_saver") {
        schemeGuid = {0xa1841308, 0x3541, 0x4fab, {0xbc, 0x81, 0xf7, 0x15, 0x56, 0xf2, 0x0b, 0x4a}};
    } else {
        qWarning() << "[Zereca] Unknown power mode:" << mode;
        return false;
    }
    
    DWORD result = PowerSetActiveScheme(nullptr, &schemeGuid);
    if (result == ERROR_SUCCESS) {
        qDebug() << "[Zereca] Enforced power mode:" << mode;
        return true;
    }
    
    qWarning() << "[Zereca] Failed to set power mode:" << result;
    return false;
#else
    Q_UNUSED(mode);
    return false;
#endif
}

bool StateReconciler::enforceTimerResolution(const QString& resolution)
{
#ifdef Q_OS_WIN
    typedef NTSTATUS(WINAPI* NtSetTimerResolutionPtr)(ULONG, BOOLEAN, PULONG);
    
    static auto NtSetTimerResolution = reinterpret_cast<NtSetTimerResolutionPtr>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtSetTimerResolution"));
    
    if (!NtSetTimerResolution) {
        return false;
    }
    
    ULONG desiredRes;
    if (resolution == "0.5ms") {
        desiredRes = 5000;   // 0.5ms in 100ns units
    } else if (resolution == "1ms") {
        desiredRes = 10000;  // 1ms in 100ns units
    } else {
        desiredRes = 156250; // Default ~15.6ms
    }
    
    ULONG actualRes;
    NTSTATUS status = NtSetTimerResolution(desiredRes, TRUE, &actualRes);
    
    if (status == 0) {
        qDebug() << "[Zereca] Enforced timer resolution:" << resolution;
        return true;
    }
    
    qWarning() << "[Zereca] Failed to set timer resolution";
    return false;
#else
    Q_UNUSED(resolution);
    return false;
#endif
}

bool StateReconciler::enforceCpuParking(bool enabled)
{
    // CPU parking requires modifying power plan settings
    // This is a privileged operation
    qDebug() << "[Zereca] CPU parking enforcement:" << (enabled ? "enabled" : "disabled");
    // TODO: Implement via PowerWriteACValueIndex
    return false;
}

bool StateReconciler::enforceProcessAffinity(const QString& process, const QString& coreGroup)
{
#ifdef Q_OS_WIN
    // Find process by name
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    
    if (Process32FirstW(snapshot, &pe)) {
        do {
            if (QString::fromWCharArray(pe.szExeFile).compare(process, Qt::CaseInsensitive) == 0) {
                processId = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &pe));
    }
    CloseHandle(snapshot);
    
    if (processId == 0) {
        return false;  // Process not running
    }
    
    // Open process and set affinity
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION,
                                   FALSE, processId);
    if (!hProcess) {
        return false;
    }
    
    // Parse core group to affinity mask
    DWORD_PTR affinityMask = 0;
    if (coreGroup == "gold_cores") {
        // Use higher-numbered cores (P-cores on hybrid CPUs)
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        DWORD numCores = sysInfo.dwNumberOfProcessors;
        // Use top half of cores
        for (DWORD i = numCores / 2; i < numCores; i++) {
            affinityMask |= (1ULL << i);
        }
    } else if (coreGroup == "all") {
        DWORD_PTR procAffinity, sysAffinity;
        GetProcessAffinityMask(hProcess, &procAffinity, &sysAffinity);
        affinityMask = sysAffinity;
    } else {
        // Try to parse as hex bitmask
        bool ok;
        affinityMask = coreGroup.toULongLong(&ok, 16);
        if (!ok) {
            CloseHandle(hProcess);
            return false;
        }
    }
    
    BOOL result = SetProcessAffinityMask(hProcess, affinityMask);
    CloseHandle(hProcess);
    
    if (result) {
        qDebug() << "[Zereca] Set affinity for" << process << "to" << coreGroup;
        return true;
    }
    
    return false;
#else
    Q_UNUSED(process);
    Q_UNUSED(coreGroup);
    return false;
#endif
}

} // namespace Zereca
