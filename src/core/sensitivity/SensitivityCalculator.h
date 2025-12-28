#ifndef NEOZ_SENSITIVITYCALCULATOR_H
#define NEOZ_SENSITIVITYCALCULATOR_H

#include "../input/InputState.h"
#include <QObject>

namespace NeoZ {

/**
 * @brief Pure calculation class implementing the Neo-Z Master Equation.
 * 
 * The Master Equation:
 *   Input_final(t) = C(v) · S · α · E_s · E_r · W_s · (m_raw(t) / D_hw)
 * 
 * Where:
 * - C(v) = velocity curve multiplier
 * - S = X/Y sensitivity multipliers
 * - α = pixel → angular conversion (game-specific)
 * - E_s = emulator sensitivity scalar
 * - E_r = emulator resolution scale
 * - W_s = Windows pointer scale
 * - D_hw = mouse hardware DPI
 * - m_raw(t) = raw mouse input
 * 
 * This class is stateless and thread-safe.
 */
class SensitivityCalculator
{
public:
    /**
     * @brief Parameters for the master equation calculation.
     */
    struct Parameters {
        // Velocity curve value C(v) - computed from velocity
        double velocityCurve = 1.0;
        
        // X/Y multipliers (S)
        double sensitivityX = 1.0;
        double sensitivityY = 1.0;
        
        // Pixel to angular conversion (α)
        // Free Fire uses ~0.022 degrees per pixel at 1080p
        double pixelToAngular = 0.022;
        
        // Emulator scalars
        double emulatorSensitivity = 1.0;  // E_s
        double resolutionScale = 1.0;       // E_r
        
        // Host scalars
        double windowsPointerScale = 1.0;   // W_s
        int mouseDpi = 800;                  // D_hw
        
        // Reference DPI for normalization
        static constexpr int REFERENCE_DPI = 800;
        
        // Get DPI normalization factor
        double dpiNormalization() const {
            return static_cast<double>(REFERENCE_DPI) / static_cast<double>(mouseDpi);
        }
        
        // Calculate total multiplier (excludes velocity curve, which is applied separately)
        double totalMultiplier() const {
            return sensitivityX * pixelToAngular * emulatorSensitivity 
                   * resolutionScale * windowsPointerScale * dpiNormalization();
        }
    };
    
    /**
     * @brief Result of sensitivity calculation.
     */
    struct Result {
        InputState finalState;
        
        // Breakdown for debugging/display
        double velocityCurveApplied = 1.0;
        double totalMultiplierX = 1.0;
        double totalMultiplierY = 1.0;
        double angularDeltaX = 0.0;  // Final delta in degrees
        double angularDeltaY = 0.0;
    };
    
    SensitivityCalculator() = default;
    ~SensitivityCalculator() = default;
    
    /**
     * @brief Calculate final sensitivity-adjusted input.
     * 
     * @param input Raw input state
     * @param velocityCurveValue Pre-computed C(v) value
     * @param params Pipeline parameters
     * @return Result with final state and breakdown
     */
    static Result calculate(const InputState& input, 
                            double velocityCurveValue,
                            const Parameters& params);
    
    /**
     * @brief Calculate effective sensitivity (for display).
     * 
     * Returns the total multiplier that would be applied to input.
     */
    static double effectiveSensitivity(const Parameters& params, double velocityCurve = 1.0);
    
    /**
     * @brief Calculate 360° distance in cm for a given sensitivity.
     * 
     * Common metric for comparing sensitivities across games.
     */
    static double calculate360Distance(const Parameters& params, double velocityCurve = 1.0);
};

} // namespace NeoZ

#endif // NEOZ_SENSITIVITYCALCULATOR_H
