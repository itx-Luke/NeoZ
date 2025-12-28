#ifndef NEOZ_HOSTNORMALIZER_H
#define NEOZ_HOSTNORMALIZER_H

#include <QObject>
#include <QSize>
#include "../input/InputState.h"

namespace NeoZ {

/**
 * @brief Normalizes input for Windows host environment.
 * 
 * This class implements the host normalization layer:
 *   n_host(t) = (m_acc(t) / D_hw) * W_s
 * 
 * Where:
 * - m_acc(t) = input after Windows acceleration (if any)
 * - D_hw = mouse hardware DPI
 * - W_s = Windows pointer speed multiplier
 */
class HostNormalizer : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int mouseDpi READ mouseDpi WRITE setMouseDpi NOTIFY parametersChanged)
    Q_PROPERTY(double windowsPointerScale READ windowsPointerScale WRITE setWindowsPointerScale NOTIFY parametersChanged)
    Q_PROPERTY(bool accelerationEnabled READ accelerationEnabled WRITE setAccelerationEnabled NOTIFY parametersChanged)
    
    // Monitor properties for angular delta
    Q_PROPERTY(int screenWidth READ screenWidth WRITE setScreenWidth NOTIFY parametersChanged)
    Q_PROPERTY(int screenHeight READ screenHeight WRITE setScreenHeight NOTIFY parametersChanged)
    Q_PROPERTY(int refreshRate READ refreshRate WRITE setRefreshRate NOTIFY parametersChanged)
    Q_PROPERTY(int presetConfidence READ presetConfidence NOTIFY presetConfidenceChanged)
    
    // FOV-based angular calculation (Neo-Z Precision Engine)
    Q_PROPERTY(double fovX READ fovX WRITE setFovX NOTIFY parametersChanged)
    
    // Angular delta output (degrees per cm)
    Q_PROPERTY(double angularSensitivity READ angularSensitivity NOTIFY parametersChanged)
    
public:
    explicit HostNormalizer(QObject* parent = nullptr);
    ~HostNormalizer() override = default;
    
    // Process input through host normalization
    InputState normalize(const InputState& input) const;
    
    // Getters
    int mouseDpi() const { return m_mouseDpi; }
    double windowsPointerScale() const { return m_windowsPointerScale; }
    bool accelerationEnabled() const { return m_accelerationEnabled; }
    int screenWidth() const { return m_screenWidth; }
    int screenHeight() const { return m_screenHeight; }
    int refreshRate() const { return m_refreshRate; }
    double fovX() const { return m_fovX; }
    
    // Preset confidence: 0=Native, 1=Scaled, 2=Mismatch
    enum PresetConfidence { Native = 0, Scaled = 1, Mismatch = 2 };
    Q_ENUM(PresetConfidence)
    int presetConfidence() const { return m_presetConfidence; }
    
    // Setters
    void setMouseDpi(int dpi);
    void setWindowsPointerScale(double scale);
    void setAccelerationEnabled(bool enabled);
    void setScreenWidth(int width);
    void setScreenHeight(int height);
    void setRefreshRate(int hz);
    void setFovX(double fov);
    
    // Calculate DPI normalization factor
    double dpiNormalizationFactor() const;
    
    // ========== NEO-Z PRECISION ENGINE - ANGULAR DELTA ==========
    // FOV-based formula: Δθ = (Δcounts / DPI) × (FOVx / screenWidth)
    // This makes sensitivity resolution-independent
    struct AngularDelta {
        double x;
        double y;
    };
    AngularDelta calculateAngularDelta(double deltaX, double deltaY) const;
    
    // Returns angular sensitivity in degrees per cm
    double angularSensitivity() const;
    
    // Refresh rate compensation factor (reference 120Hz)
    double refreshRateFactor() const;
    
signals:
    void parametersChanged();
    void presetConfidenceChanged();
    
private:
    // Apply acceleration curve (approximation of Windows EPP)
    double applyAccelerationCurve(double velocity) const;
    
    // Update preset confidence based on current settings
    void updatePresetConfidence();
    
    int m_mouseDpi = 800;           // Default mouse DPI
    double m_windowsPointerScale = 1.0;  // Windows pointer speed (W_s)
    bool m_accelerationEnabled = false;   // EPP status
    
    // Monitor properties
    int m_screenWidth = 1920;
    int m_screenHeight = 1080;
    int m_refreshRate = 60;
    int m_presetConfidence = Native;
    double m_fovX = 90.0;  // Horizontal FOV in degrees (Free Fire ≈ 90°)
    
    // Reference values for normalization (Neo-Z Precision Engine)
    static constexpr int REFERENCE_DPI = 800;
    static constexpr int REFERENCE_HZ = 120;
    static constexpr int REFERENCE_WIDTH = 1920;
    static constexpr int REFERENCE_HEIGHT = 1080;
    static constexpr double REFERENCE_FOV = 90.0;
};

} // namespace NeoZ

#endif // NEOZ_HOSTNORMALIZER_H
