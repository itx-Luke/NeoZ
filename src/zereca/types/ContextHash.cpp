#include "ContextHash.h"
#include <QFile>
#include <QCryptographicHash>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
#endif

namespace Zereca {

SystemContext ContextHash::capture()
{
    SystemContext ctx;
    ctx.gpuDriverVersion = getGpuDriverVersion();
    ctx.osBuild = getOsBuild();
    ctx.biosVersion = getBiosVersion();
    // emulatorBinaryHash is set separately when an emulator is detected
    ctx.emulatorBinaryHash = 0;
    return ctx;
}

uint64_t ContextHash::getGpuDriverVersion()
{
#ifdef Q_OS_WIN
    IDXGIFactory* factory = nullptr;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) {
        return 0;
    }
    
    IDXGIAdapter* adapter = nullptr;
    if (FAILED(factory->EnumAdapters(0, &adapter))) {
        factory->Release();
        return 0;
    }
    
    DXGI_ADAPTER_DESC desc;
    if (FAILED(adapter->GetDesc(&desc))) {
        adapter->Release();
        factory->Release();
        return 0;
    }
    
    // Pack vendor ID and device ID into a single uint64
    uint64_t version = (static_cast<uint64_t>(desc.VendorId) << 32) | desc.DeviceId;
    
    adapter->Release();
    factory->Release();
    return version;
#else
    return 0;
#endif
}

uint64_t ContextHash::getOsBuild()
{
#ifdef Q_OS_WIN
    OSVERSIONINFOEXW osvi = { sizeof(osvi) };
    
    // Use RtlGetVersion to bypass compatibility mode
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll) {
        auto RtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
            GetProcAddress(hNtdll, "RtlGetVersion"));
        if (RtlGetVersion) {
            RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi));
        }
    }
    
    // Pack major.minor.build into uint64
    return (static_cast<uint64_t>(osvi.dwMajorVersion) << 32) |
           (static_cast<uint64_t>(osvi.dwMinorVersion) << 16) |
           osvi.dwBuildNumber;
#else
    return 0;
#endif
}

uint64_t ContextHash::getBiosVersion()
{
#ifdef Q_OS_WIN
    // Read BIOS version from registry
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"HARDWARE\\DESCRIPTION\\System\\BIOS",
                      0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return 0;
    }
    
    wchar_t biosVersion[256] = {0};
    DWORD size = sizeof(biosVersion);
    DWORD type = REG_SZ;
    
    RegQueryValueExW(hKey, L"BIOSVersion", nullptr, &type,
                     reinterpret_cast<LPBYTE>(biosVersion), &size);
    RegCloseKey(hKey);
    
    // Hash the BIOS version string
    QString biosStr = QString::fromWCharArray(biosVersion);
    QByteArray hashBytes = QCryptographicHash::hash(biosStr.toUtf8(), QCryptographicHash::Sha256);
    uint64_t result = 0;
    memcpy(&result, hashBytes.constData(), sizeof(result));
    return result;
#else
    return 0;
#endif
}

uint64_t ContextHash::hashExecutable(const QString& exePath)
{
    QFile file(exePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }
    
    // Hash first 64KB of the file (enough to detect different versions)
    constexpr qint64 HASH_SIZE = 64 * 1024;
    QByteArray data = file.read(HASH_SIZE);
    file.close();
    
    QByteArray hashBytes = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    uint64_t result = 0;
    memcpy(&result, hashBytes.constData(), sizeof(result));
    return result;
}

} // namespace Zereca
