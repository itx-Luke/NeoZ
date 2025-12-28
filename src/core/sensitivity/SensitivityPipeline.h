#ifndef NEOZ_SENSITIVITYPIPELINE_H
#define NEOZ_SENSITIVITYPIPELINE_H

#include <QObject>
#include <QElapsedTimer>
#include <memory>
#include <deque>
#include "../input/InputState.h"
#include "VelocityCurve.h"
#include "HostNormalizer.h"
#include "EmulatorTranslator.h"
#include "SensitivityCalculator.h"

namespace NeoZ {

class WindowsInputReader;

/**
 * @brief Main orchestrator for the Neo-Z Input Pipeline.
 * 
 * Implements the Neo-Z Precision Axis Control formal model:
 *   Raw Δ → DPI norm → Win speed → Res norm → Axis gains → Curve → Smoothing → DRCS → Output
 * 
 * Center-Zero Multiplier Model:
 *   G_x = 1 + k * M_x  where M ∈ [-1, +1] and k ≤ 0.6
 *   At M=0, gain is neutral (1.0)
 * 
 * Time-Based Smoothing:
 *   λ = e^(-Δt/τ)
 *   Δ_final = λ * Δ_prev + (1-λ) * Δ_curve
 */
class SensitivityPipeline : public QObject
{
    Q_OBJECT
    
    // Main sensitivity controls
    Q_PROPERTY(double sensitivityX READ sensitivityX WRITE setSensitivityX NOTIFY settingsChanged)
    Q_PROPERTY(double sensitivityY READ sensitivityY WRITE setSensitivityY NOTIFY settingsChanged)
    Q_PROPERTY(int mouseDpi READ mouseDpi WRITE setMouseDpi NOTIFY settingsChanged)
    
    // ========== INPUT AUTHORITY CONTROL ==========
    // Master gate - when OFF, no mouse modification occurs (safe mode)
    Q_PROPERTY(bool inputAuthorityEnabled READ inputAuthorityEnabled WRITE setInputAuthorityEnabled NOTIFY inputAuthorityChanged)
    Q_PROPERTY(double latencyMs READ latencyMs NOTIFY latencyChanged)
    Q_PROPERTY(bool safeZoneClampEnabled READ safeZoneClampEnabled WRITE setSafeZoneClampEnabled NOTIFY settingsChanged)
    
    // Preset confidence: 0=Native, 1=Scaled, 2=Mismatch
    Q_PROPERTY(int presetConfidence READ presetConfidence NOTIFY presetConfidenceChanged)
    
    // Effective angular sensitivity (degrees per cm)
    Q_PROPERTY(double effectiveAngularSensitivity READ effectiveAngularSensitivity NOTIFY settingsChanged)
    
    // Center-Zero Axis Multipliers (range: -1.0 to +1.0)
    Q_PROPERTY(double axisMultiplierX READ axisMultiplierX WRITE setAxisMultiplierX NOTIFY settingsChanged)
    Q_PROPERTY(double axisMultiplierY READ axisMultiplierY WRITE setAxisMultiplierY NOTIFY settingsChanged)
    Q_PROPERTY(double gainFactor READ gainFactor WRITE setGainFactor NOTIFY settingsChanged)
    
    // Time-based smoothing (0-200ms UI, non-linear internal mapping)
    Q_PROPERTY(double smoothingMs READ smoothingMs WRITE setSmoothingMs NOTIFY settingsChanged)
    
    // Slow zone (1-100%)
    Q_PROPERTY(double slowZonePercent READ slowZonePercent WRITE setSlowZonePercent NOTIFY settingsChanged)
    
    // Sub-components exposed for QML
    Q_PROPERTY(NeoZ::VelocityCurve* velocityCurve READ velocityCurve CONSTANT)
    Q_PROPERTY(NeoZ::HostNormalizer* hostNormalizer READ hostNormalizer CONSTANT)
    Q_PROPERTY(NeoZ::EmulatorTranslator* emulatorTranslator READ emulatorTranslator CONSTANT)
    
    // Computed values
    Q_PROPERTY(double effectiveSensitivity READ effectiveSensitivity NOTIFY settingsChanged)
    Q_PROPERTY(double cm360 READ cm360 NOTIFY settingsChanged)
    Q_PROPERTY(double gainX READ gainX NOTIFY settingsChanged)
    Q_PROPERTY(double gainY READ gainY NOTIFY settingsChanged)
    
public:
    explicit SensitivityPipeline(QObject* parent = nullptr);
    ~SensitivityPipeline() override;
    
    // Process input through the full pipeline
    InputState process(const InputState& rawInput);
    
    // Getters
    double sensitivityX() const { return m_sensitivityX; }
    double sensitivityY() const { return m_sensitivityY; }
    int mouseDpi() const { return m_mouseDpi; }
    
    // Input Authority getters
    bool inputAuthorityEnabled() const { return m_inputAuthorityEnabled; }
    double latencyMs() const { return m_latencyMs; }
    bool safeZoneClampEnabled() const { return m_safeZoneClampEnabled; }
    int presetConfidence() const { return m_presetConfidence; }
    double effectiveAngularSensitivity() const;  // Degrees per cm
    
    // Center-zero multipliers
    double axisMultiplierX() const { return m_axisMultiplierX; }
    double axisMultiplierY() const { return m_axisMultiplierY; }
    double gainFactor() const { return m_gainFactor; }
    double gainX() const { return 1.0 + m_gainFactor * m_axisMultiplierX; }
    double gainY() const { return 1.0 + m_gainFactor * m_axisMultiplierY; }
    
    // Smoothing (UI value in ms, internal tau is computed)
    double smoothingMs() const { return m_smoothingMs; }
    double smoothingTau() const;  // Returns computed τ = max(1, S^1.35)
    
    // Slow zone
    double slowZonePercent() const { return m_slowZonePercent; }
    
    VelocityCurve* velocityCurve() const { return m_velocityCurve.get(); }
    HostNormalizer* hostNormalizer() const { return m_hostNormalizer.get(); }
    EmulatorTranslator* emulatorTranslator() const { return m_emulatorTranslator.get(); }
    
    double effectiveSensitivity() const;
    double cm360() const;
    
    // Setters
    void setSensitivityX(double value);
    void setSensitivityY(double value);
    void setMouseDpi(int dpi);
    void setAxisMultiplierX(double value);
    void setAxisMultiplierY(double value);
    void setGainFactor(double value);
    void setSmoothingMs(double value);  // UI value 0-200ms
    void setSlowZonePercent(double value);  // 1-100%
    
    // Input Authority setters
    void setInputAuthorityEnabled(bool enabled);
    void setSafeZoneClampEnabled(bool enabled);
    
    // Logic Path Selection
    void setAdbMode(bool enabled); // True = Emulator/ADB (Full), False = Desktop (Assistive)
    bool isAdbMode() const { return m_adbMode; }
    
    // Link to Windows input reader (for auto-updating W_s)
    void linkWindowsInputReader(WindowsInputReader* reader);
    
    // Build parameters for calculator
    SensitivityCalculator::Parameters buildParameters(double velocity = 0.0) const;
    
    // Reset to defaults
    Q_INVOKABLE void resetToDefaults();
    
    // ========== SNAPSHOT / ROLLBACK ==========
    Q_INVOKABLE void takeSnapshot();
    Q_INVOKABLE void rollback();
    Q_INVOKABLE bool hasSnapshot() const { return m_hasSnapshot; }
    
    // Simulate mode (visual only, no actual modification)
    Q_INVOKABLE void enableSimulateMode(bool enable);
    bool isSimulating() const { return m_simulateMode; }
    
signals:
    void settingsChanged();
    void inputProcessed(const InputState& finalState);
    void pipelineRecalculated();
    void inputAuthorityChanged();
    void latencyChanged();
    void presetConfidenceChanged();
    
private slots:
    void onSubComponentChanged();
    
private:
    // Components
    std::unique_ptr<VelocityCurve> m_velocityCurve;
    std::unique_ptr<HostNormalizer> m_hostNormalizer;
    std::unique_ptr<EmulatorTranslator> m_emulatorTranslator;
    
    // Direct parameters
    double m_sensitivityX = 1.0;
    double m_sensitivityY = 1.0;
    int m_mouseDpi = 800;
    
    // Center-zero axis multipliers
    double m_axisMultiplierX = 0.0;  // Range: -1.0 to +1.0, 0 = neutral
    double m_axisMultiplierY = 0.0;
    double m_gainFactor = 0.6;       // Safe scaling cap (k ≤ 0.6)
    
    // Time-based smoothing (extended range 0-200ms)
    double m_smoothingMs = 16.0;     // UI value in ms (0-200)
    double m_prevDeltaX = 0.0;
    double m_prevDeltaY = 0.0;
    QElapsedTimer m_smoothingTimer;
    
    // Slow zone (1-100%)
    double m_slowZonePercent = 20.0;  // Default 20% - headshot sweet spot
    static constexpr double SLOW_ZONE_GAMMA = 0.35;  // Attenuation factor
    static constexpr double SLOW_ZONE_V_MAX = 100.0; // Max velocity reference
    
    // ========== INPUT AUTHORITY STATE ==========
    bool m_inputAuthorityEnabled = false;  // OFF by default (safe mode)
    bool m_adbMode = false;                // OFF by default (Desktop/Assistive)
    double m_latencyMs = 0.0;
    bool m_safeZoneClampEnabled = true;    // ON by default (recommended)
    int m_presetConfidence = 0;            // 0=Native, 1=Scaled, 2=Mismatch
    QElapsedTimer m_latencyTimer;
    
    // Repetition Drag Limiter history buffer
    std::deque<std::pair<double, double>> m_dragHistory;
    static constexpr size_t DRAG_HISTORY_SIZE = 8;
    static constexpr double DRAG_SIMILARITY_THRESHOLD = 0.95;
    static constexpr double DRAG_DAMPING = 0.85;
    
    // Snapshot for rollback
    struct Snapshot {
        double sensitivityX, sensitivityY;
        double axisMultiplierX, axisMultiplierY;
        double gainFactor, smoothingMs, slowZonePercent;
        int mouseDpi;
    };
    Snapshot m_snapshot{};
    bool m_hasSnapshot = false;
    bool m_simulateMode = false;
    
    // Game constants
    double m_pixelToAngular = 0.022;  // Free Fire default at 1080p
    
    // Linked external components
    WindowsInputReader* m_windowsReader = nullptr;
};

} // namespace NeoZ

#endif // NEOZ_SENSITIVITYPIPELINE_H
