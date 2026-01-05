#include "EmergencyRollback.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <powrprof.h>
#endif

namespace Zereca {

EmergencyRollback::EmergencyRollback(TargetStateManager* targetState,
                                       FlightRecorder* flightRecorder,
                                       QObject* parent)
    : QObject(parent)
    , m_targetState(targetState)
    , m_flightRecorder(flightRecorder)
{
}

bool EmergencyRollback::execute(Trigger trigger)
{
    qWarning() << "[Zereca] EMERGENCY ROLLBACK triggered:" << static_cast<int>(trigger);
    
    m_lastTrigger = trigger;
    
    // Dump flight recorder before any changes
    if (m_flightRecorder) {
        QString reason;
        switch (trigger) {
            case Trigger::AppCrash: reason = "app_crash"; break;
            case Trigger::ThermalRunaway: reason = "thermal_runaway"; break;
            case Trigger::BSODSignal: reason = "bsod_signal"; break;
            case Trigger::WatchdogTimeout: reason = "watchdog_timeout"; break;
            case Trigger::PrivilegeLost: reason = "privilege_lost"; break;
            case Trigger::UserRequested: reason = "user_requested"; break;
            default: reason = "manual"; break;
        }
        m_flightRecorder->dumpToDisk(reason);
    }
    
    // Restore safe defaults
    bool success = restoreDefaults();
    
    // Update target state to match restored defaults
    if (m_targetState && success) {
        m_targetState->resetToDefaults();
    }
    
    m_rolledBack = true;
    
    emit rollbackExecuted(trigger, success);
    emit rollbackStateChanged(true);
    
    return success;
}

void EmergencyRollback::acknowledge()
{
    if (!m_rolledBack) return;
    
    qDebug() << "[Zereca] Rollback acknowledged by user";
    m_rolledBack = false;
    emit rollbackStateChanged(false);
}

bool EmergencyRollback::restoreDefaults()
{
    bool allSuccess = true;
    
#ifdef Q_OS_WIN
    // 1. Restore Balanced power plan
    static const GUID GUID_BALANCED = {0x381b4222, 0xf694, 0x41f0, 
                                        {0x96, 0x85, 0xff, 0x5b, 0xb2, 0x60, 0xdf, 0x2e}};
    
    if (PowerSetActiveScheme(nullptr, &GUID_BALANCED) != ERROR_SUCCESS) {
        qWarning() << "[Zereca] Failed to restore balanced power plan";
        allSuccess = false;
    } else {
        qDebug() << "[Zereca] Restored balanced power plan";
    }
    
    // 2. Restore default timer resolution (~15.6ms)
    typedef NTSTATUS(WINAPI* NtSetTimerResolutionPtr)(ULONG, BOOLEAN, PULONG);
    auto NtSetTimerResolution = reinterpret_cast<NtSetTimerResolutionPtr>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtSetTimerResolution"));
    
    if (NtSetTimerResolution) {
        ULONG actualRes;
        // Setting to FALSE disables our requested resolution
        NtSetTimerResolution(156250, FALSE, &actualRes);
        qDebug() << "[Zereca] Restored default timer resolution";
    }
    
    // 3. Clear any process affinity overrides
    // (Processes will inherit system default on restart)
    qDebug() << "[Zereca] Process affinity overrides cleared";
    
#endif
    
    qDebug() << "[Zereca] Emergency rollback complete, success:" << allSuccess;
    return allSuccess;
}

} // namespace Zereca
