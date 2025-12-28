#include "WindowsInputReader.h"
#include <QDebug>

namespace NeoZ {

WindowsInputReader::WindowsInputReader(QObject* parent)
    : QObject(parent)
{
    refresh();
}

void WindowsInputReader::refresh()
{
    int oldSpeed = m_pointerSpeed;
    bool oldEnhance = m_enhancePrecision;
    int oldDpi = m_systemDpi;
    
    m_pointerSpeed = readPointerSpeed();
    m_pointerSpeedMultiplier = speedToMultiplier(m_pointerSpeed);
    m_enhancePrecision = readEnhancePrecision();
    m_systemDpi = readSystemDpi();
    
    // Check for problematic settings
    if (m_enhancePrecision) {
        emit warningDetected(tr("Windows Mouse Acceleration is enabled. "
                                "For consistent aim, disable 'Enhance Pointer Precision' "
                                "in Windows Mouse Settings."));
    }
    
    if (m_pointerSpeed != 10) {
        emit warningDetected(tr("Windows Pointer Speed is not default (currently %1/20). "
                                "Neo-Z will compensate, but default (10) is recommended.")
                             .arg(m_pointerSpeed));
    }
    
    // Emit change signal if anything changed
    if (oldSpeed != m_pointerSpeed || 
        oldEnhance != m_enhancePrecision || 
        oldDpi != m_systemDpi) {
        emit settingsChanged();
    }
    
    qDebug() << "[WindowsInputReader] Pointer Speed:" << m_pointerSpeed 
             << "(" << m_pointerSpeedMultiplier << "x)"
             << "| Acceleration:" << (m_enhancePrecision ? "ON" : "OFF")
             << "| System DPI:" << m_systemDpi;
}

int WindowsInputReader::readPointerSpeed()
{
    int speed = 10; // Default
    
    // SPI_GETMOUSESPEED returns a value from 1-20
    if (!SystemParametersInfo(SPI_GETMOUSESPEED, 0, &speed, 0)) {
        qWarning() << "[WindowsInputReader] Failed to read pointer speed. Error:" << GetLastError();
        return 10;
    }
    
    return speed;
}

bool WindowsInputReader::readEnhancePrecision()
{
    // Read mouse parameters (3 integers: threshold1, threshold2, acceleration)
    int mouseParams[3] = {0, 0, 0};
    
    if (!SystemParametersInfo(SPI_GETMOUSE, 0, mouseParams, 0)) {
        qWarning() << "[WindowsInputReader] Failed to read mouse params. Error:" << GetLastError();
        return false;
    }
    
    // mouseParams[2] = acceleration (0 = off, 1 or 2 = on)
    return mouseParams[2] != 0;
}

int WindowsInputReader::readSystemDpi()
{
    // Get the DPI for the primary monitor
    HDC hdc = GetDC(NULL);
    if (!hdc) {
        qWarning() << "[WindowsInputReader] Failed to get DC for DPI reading";
        return 96;
    }
    
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    
    return dpi;
}

double WindowsInputReader::speedToMultiplier(int speed)
{
    // Windows pointer speed 1-20 maps to multipliers:
    // 1-5:   0.03125 to 0.5 (slow)
    // 6-10:  0.625 to 1.0 (normal progression)
    // 11-20: 1.5 to 3.5 (fast)
    
    // This is an approximation of Windows' actual curve
    static const double multipliers[21] = {
        0.0,    // 0 (unused)
        0.03125, 0.0625, 0.125, 0.25, 0.5,   // 1-5
        0.625, 0.75, 0.875, 0.9375, 1.0,     // 6-10
        1.5, 1.75, 2.0, 2.25, 2.5,           // 11-15
        2.75, 3.0, 3.25, 3.375, 3.5          // 16-20
    };
    
    if (speed < 1) speed = 1;
    if (speed > 20) speed = 20;
    
    return multipliers[speed];
}

} // namespace NeoZ
