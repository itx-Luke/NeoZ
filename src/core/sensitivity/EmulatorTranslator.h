#ifndef NEOZ_EMULATORTRANSLATOR_H
#define NEOZ_EMULATORTRANSLATOR_H

#include <QObject>
#include <QSize>
#include "../input/InputState.h"

class AdbConnector;  // Forward declaration

namespace NeoZ {

/**
 * @brief Handles emulator-specific input translation.
 * 
 * Emulators convert mouse delta → touch look input with their own
 * scaling factors. This class models:
 *   m_emu(t) = E_s * E_r * n_host(t)
 * 
 * Where:
 * - E_s = emulator sensitivity scalar
 * - E_r = emulator resolution scale factor
 */
class EmulatorTranslator : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(EmulatorPreset preset READ preset WRITE setPreset NOTIFY parametersChanged)
    Q_PROPERTY(double sensitivityScalar READ sensitivityScalar WRITE setSensitivityScalar NOTIFY parametersChanged)
    Q_PROPERTY(double resolutionScale READ resolutionScale WRITE setResolutionScale NOTIFY parametersChanged)
    Q_PROPERTY(QSize emulatorResolution READ emulatorResolution WRITE setEmulatorResolution NOTIFY parametersChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    
public:
    enum EmulatorPreset {
        Unknown,
        BlueStacks,
        MSIAppPlayer,
        LDPlayer,
        NoxPlayer,
        MEmu,
        HDPlayer,  // User's primary emulator
        Custom
    };
    Q_ENUM(EmulatorPreset)
    
    explicit EmulatorTranslator(QObject* parent = nullptr);
    ~EmulatorTranslator() override = default;
    
    // Process input through emulator translation
    InputState translate(const InputState& input) const;
    
    // Getters
    EmulatorPreset preset() const { return m_preset; }
    double sensitivityScalar() const { return m_sensitivityScalar; }
    double resolutionScale() const { return m_resolutionScale; }
    QSize emulatorResolution() const { return m_emulatorResolution; }
    bool isConnected() const { return m_connected; }
    
    // Setters
    void setPreset(EmulatorPreset preset);
    void setSensitivityScalar(double scalar);
    void setResolutionScale(double scale);
    void setEmulatorResolution(const QSize& resolution);
    void setConnected(bool connected);
    
    // Apply preset values based on emulator type
    Q_INVOKABLE void applyPreset(EmulatorPreset preset);
    
    // Calculate resolution scale from emulator resolution
    Q_INVOKABLE double calculateResolutionScale(const QSize& resolution) const;
    
    // Get preset name for display
    Q_INVOKABLE static QString presetName(EmulatorPreset preset);
    
    // ========== ADB DPI SYNC ==========
    // Sync emulator DPI via ADB shell command
    Q_INVOKABLE bool syncEmulatorDpi(int densityDpi);
    Q_INVOKABLE int readEmulatorDpi();
    void setAdbConnector(AdbConnector* connector) { m_adbConnector = connector; }
    int emulatorDpi() const { return m_emulatorDpi; }
    
signals:
    void parametersChanged();
    void connectionChanged();
    
private:
    EmulatorPreset m_preset = Unknown;
    double m_sensitivityScalar = 1.0;   // E_s
    double m_resolutionScale = 1.0;     // E_r
    QSize m_emulatorResolution{1280, 720};
    bool m_connected = false;
    
    // ADB DPI Sync
    AdbConnector* m_adbConnector = nullptr;
    int m_emulatorDpi = 320;  // Default android DPI
    
    // Reference resolution for scaling
    static constexpr int REFERENCE_WIDTH = 1920;
    static constexpr int REFERENCE_HEIGHT = 1080;
};

} // namespace NeoZ

#endif // NEOZ_EMULATORTRANSLATOR_H
