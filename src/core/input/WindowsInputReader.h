#ifndef NEOZ_WINDOWSINPUTREADER_H
#define NEOZ_WINDOWSINPUTREADER_H

#include <QObject>
#include <windows.h>

namespace NeoZ {

/**
 * @brief Reads Windows mouse system settings via WinAPI.
 * 
 * This class provides access to Windows-level input settings that
 * affect how mouse input is transformed before reaching applications.
 * These values are critical for the Neo-Z master equation.
 * 
 * Windows Mouse Settings:
 * - Pointer Speed (1-20 scale, 10 = 1.0x multiplier)
 * - Enhance Pointer Precision (acceleration)
 * - System DPI
 */
class WindowsInputReader : public QObject
{
    Q_OBJECT
    
    // QML-accessible properties
    Q_PROPERTY(int pointerSpeed READ pointerSpeed NOTIFY settingsChanged)
    Q_PROPERTY(double pointerSpeedMultiplier READ pointerSpeedMultiplier NOTIFY settingsChanged)
    Q_PROPERTY(bool enhancePrecisionEnabled READ enhancePrecisionEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int systemDpi READ systemDpi NOTIFY settingsChanged)
    
public:
    explicit WindowsInputReader(QObject* parent = nullptr);
    ~WindowsInputReader() override = default;
    
    // Getters
    int pointerSpeed() const { return m_pointerSpeed; }
    double pointerSpeedMultiplier() const { return m_pointerSpeedMultiplier; }
    bool enhancePrecisionEnabled() const { return m_enhancePrecision; }
    int systemDpi() const { return m_systemDpi; }
    
    // Refresh settings from Windows
    Q_INVOKABLE void refresh();
    
    // Convert pointer speed (1-20) to multiplier
    static double speedToMultiplier(int speed);
    
signals:
    void settingsChanged();
    void warningDetected(const QString& message);
    
private:
    // Read Windows pointer speed (1-20)
    int readPointerSpeed();
    
    // Check if "Enhance Pointer Precision" is enabled
    bool readEnhancePrecision();
    
    // Read system DPI
    int readSystemDpi();
    
    // Cached values
    int m_pointerSpeed = 10;
    double m_pointerSpeedMultiplier = 1.0;
    bool m_enhancePrecision = false;
    int m_systemDpi = 96;
};

} // namespace NeoZ

#endif // NEOZ_WINDOWSINPUTREADER_H
