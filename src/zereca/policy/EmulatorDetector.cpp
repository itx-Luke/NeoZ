#include "EmulatorDetector.h"
#include "../types/ContextHash.h"
#include <QDebug>
#include <algorithm>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#endif

namespace Zereca {

EmulatorDetector::EmulatorDetector(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &EmulatorDetector::onScanTick);
    
    initDefaultSignatures();
}

EmulatorDetector::~EmulatorDetector()
{
    stopScanning();
}

void EmulatorDetector::initDefaultSignatures()
{
    // Bluestacks / MSI App Player
    m_signatures.push_back({
        "Bluestacks",
        {"HD-Player.exe", "Bluestacks.exe", "BluestacksHelper.exe"},
        {"BlueStacksApp", "BS2CHINAPCKGBDUI"},
        {"aow_exe.dll", "libGLESv2.dll"},
        0.6f
    });
    
    // LDPlayer
    m_signatures.push_back({
        "LDPlayer",
        {"dnplayer.exe", "LdVBoxHeadless.exe", "LdBoxHeadless.exe"},
        {"LDPlayerMainFrame"},
        {"dnconsole.dll"},
        0.6f
    });
    
    // Nox Player
    m_signatures.push_back({
        "NoxPlayer",
        {"Nox.exe", "NoxVMHandle.exe", "NoxVMSVC.exe"},
        {"Qt5QWindowIcon", "Nox"},
        {"libegl.dll"},
        0.6f
    });
    
    // MEmu
    m_signatures.push_back({
        "MEmu",
        {"MEmu.exe", "MEmuHeadless.exe", "MEmuConsole.exe"},
        {"Qt5QWindowIcon"},
        {"MEmuSVC.dll"},
        0.6f
    });
    
    // SmartGaGa
    m_signatures.push_back({
        "SmartGaGa",
        {"SmartGaGa.exe", "TurboAndroidPlayer.exe"},
        {"SmartGaGaWindow"},
        {},
        0.5f
    });
}

void EmulatorDetector::startScanning(int intervalMs)
{
    if (m_scanning) return;
    
    m_scanning = true;
    m_timer->start(intervalMs);
    
    qDebug() << "[Zereca] EmulatorDetector started, interval:" << intervalMs << "ms";
    emit scanningChanged(true);
    
    // Immediate first scan
    onScanTick();
}

void EmulatorDetector::stopScanning()
{
    if (!m_scanning) return;
    
    m_scanning = false;
    m_timer->stop();
    
    qDebug() << "[Zereca] EmulatorDetector stopped";
    emit scanningChanged(false);
}

std::vector<EmulatorInfo> EmulatorDetector::scanNow()
{
    // Step 1: Find processes matching known executables
    std::vector<EmulatorInfo> detected = detectByExecutable();
    
    // Step 2: Boost confidence with additional signals
    for (auto& info : detected) {
        // Window class check (+0.15)
        info.confidence += boostConfidenceByWindowClass(info);
        
        // Loaded modules check (+0.10)
        info.confidence += boostConfidenceByModules(info);
        
        // Child process topology (+0.10)
        info.confidence += boostConfidenceByChildProcesses(info);
        
        // Clamp to [0, 1]
        info.confidence = std::min(1.0f, std::max(0.0f, info.confidence));
        
        // Generate binary hash for context
        info.binaryHash = ContextHash::hashExecutable(info.executablePath);
    }
    
    return detected;
}

EmulatorInfo EmulatorDetector::primaryEmulator() const
{
    if (m_detected.empty()) {
        return EmulatorInfo();
    }
    
    // Return highest confidence emulator
    auto it = std::max_element(m_detected.begin(), m_detected.end(),
        [](const EmulatorInfo& a, const EmulatorInfo& b) {
            return a.confidence < b.confidence;
        });
    
    return *it;
}

void EmulatorDetector::addSignature(const EmulatorSignature& sig)
{
    m_signatures.push_back(sig);
}

void EmulatorDetector::onScanTick()
{
    std::vector<EmulatorInfo> newDetected = scanNow();
    
    // Check for new emulators
    for (const auto& info : newDetected) {
        auto it = m_tracked.find(info.processId);
        if (it == m_tracked.end()) {
            // New emulator detected
            m_tracked[info.processId] = info;
            emit emulatorDetected(info);
            qDebug() << "[Zereca] Emulator detected:" << info.name 
                     << "PID:" << info.processId 
                     << "Confidence:" << info.confidence;
        }
    }
    
    // Check for lost emulators
    QList<uint32_t> lostPids;
    for (auto it = m_tracked.begin(); it != m_tracked.end(); ++it) {
        bool stillRunning = std::any_of(newDetected.begin(), newDetected.end(),
            [&](const EmulatorInfo& info) { return info.processId == it.key(); });
        
        if (!stillRunning) {
            lostPids.append(it.key());
        }
    }
    
    for (uint32_t pid : lostPids) {
        m_tracked.remove(pid);
        emit emulatorLost(pid);
        qDebug() << "[Zereca] Emulator lost, PID:" << pid;
    }
    
    m_detected = newDetected;
    emit scanComplete(static_cast<int>(m_detected.size()));
}

std::vector<EmulatorInfo> EmulatorDetector::detectByExecutable()
{
    std::vector<EmulatorInfo> result;
    
#ifdef Q_OS_WIN
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return result;
    }
    
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    
    if (Process32FirstW(snapshot, &pe)) {
        do {
            QString exeName = QString::fromWCharArray(pe.szExeFile);
            
            // Check against all signatures
            for (const auto& sig : m_signatures) {
                for (const auto& knownExe : sig.executableNames) {
                    if (exeName.compare(knownExe, Qt::CaseInsensitive) == 0) {
                        EmulatorInfo info;
                        info.name = sig.name;
                        info.processId = pe.th32ProcessID;
                        info.confidence = sig.baseConfidence;
                        
                        // Get full path
                        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 
                                                       FALSE, pe.th32ProcessID);
                        if (hProcess) {
                            wchar_t path[MAX_PATH];
                            DWORD size = MAX_PATH;
                            if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
                                info.executablePath = QString::fromWCharArray(path);
                            }
                            CloseHandle(hProcess);
                        }
                        
                        // Get child processes
                        info.childPids = getChildProcesses(pe.th32ProcessID);
                        
                        result.push_back(info);
                        break;  // Found match for this process
                    }
                }
            }
        } while (Process32NextW(snapshot, &pe));
    }
    
    CloseHandle(snapshot);
#endif
    
    return result;
}

float EmulatorDetector::boostConfidenceByWindowClass(const EmulatorInfo& info)
{
    QString windowClass = getWindowClassForProcess(info.processId);
    if (windowClass.isEmpty()) return 0.0f;
    
    for (const auto& sig : m_signatures) {
        if (sig.name == info.name) {
            for (const auto& known : sig.windowClasses) {
                if (windowClass.contains(known, Qt::CaseInsensitive)) {
                    return 0.15f;
                }
            }
        }
    }
    
    return 0.0f;
}

float EmulatorDetector::boostConfidenceByModules(const EmulatorInfo& info)
{
    QStringList modules = getLoadedModules(info.processId);
    if (modules.isEmpty()) return 0.0f;
    
    for (const auto& sig : m_signatures) {
        if (sig.name == info.name) {
            int matchCount = 0;
            for (const auto& required : sig.requiredModules) {
                for (const auto& loaded : modules) {
                    if (loaded.contains(required, Qt::CaseInsensitive)) {
                        matchCount++;
                        break;
                    }
                }
            }
            
            if (!sig.requiredModules.isEmpty()) {
                return 0.10f * matchCount / sig.requiredModules.size();
            }
        }
    }
    
    return 0.0f;
}

float EmulatorDetector::boostConfidenceByChildProcesses(const EmulatorInfo& info)
{
    // More child processes = higher confidence (emulators spawn many helpers)
    if (info.childPids.size() >= 3) return 0.10f;
    if (info.childPids.size() >= 1) return 0.05f;
    return 0.0f;
}

std::vector<uint32_t> EmulatorDetector::getChildProcesses(uint32_t parentPid)
{
    std::vector<uint32_t> children;
    
#ifdef Q_OS_WIN
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return children;
    }
    
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    
    if (Process32FirstW(snapshot, &pe)) {
        do {
            if (pe.th32ParentProcessID == parentPid) {
                children.push_back(pe.th32ProcessID);
            }
        } while (Process32NextW(snapshot, &pe));
    }
    
    CloseHandle(snapshot);
#endif
    
    return children;
}

QStringList EmulatorDetector::getLoadedModules(uint32_t pid)
{
    QStringList modules;
    
#ifdef Q_OS_WIN
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                                   FALSE, pid);
    if (!hProcess) return modules;
    
    HMODULE hMods[1024];
    DWORD cbNeeded;
    
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        DWORD count = cbNeeded / sizeof(HMODULE);
        for (DWORD i = 0; i < count && i < 1024; i++) {
            wchar_t modName[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, hMods[i], modName, MAX_PATH)) {
                modules.append(QString::fromWCharArray(modName));
            }
        }
    }
    
    CloseHandle(hProcess);
#endif
    
    return modules;
}

QString EmulatorDetector::getWindowClassForProcess(uint32_t pid)
{
#ifdef Q_OS_WIN
    struct EnumData {
        DWORD targetPid;
        QString windowClass;
    };
    
    EnumData data = { pid, QString() };
    
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* data = reinterpret_cast<EnumData*>(lParam);
        DWORD windowPid;
        GetWindowThreadProcessId(hwnd, &windowPid);
        
        if (windowPid == data->targetPid && IsWindowVisible(hwnd)) {
            wchar_t className[256];
            if (GetClassNameW(hwnd, className, 256)) {
                data->windowClass = QString::fromWCharArray(className);
                return FALSE;  // Stop enumeration
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&data));
    
    return data.windowClass;
#else
    Q_UNUSED(pid);
    return QString();
#endif
}

} // namespace Zereca
