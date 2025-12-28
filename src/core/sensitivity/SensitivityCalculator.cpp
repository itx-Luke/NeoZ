#include "SensitivityCalculator.h"
#include <cmath>

namespace NeoZ {

SensitivityCalculator::Result SensitivityCalculator::calculate(
    const InputState& input,
    double velocityCurveValue,
    const Parameters& params)
{
    Result result;
    result.velocityCurveApplied = velocityCurveValue;
    
    // Calculate DPI normalization
    double dpiNorm = params.dpiNormalization();
    
    // Build total multiplier for X and Y
    // Master Equation: C(v) · S · α · E_s · E_r · W_s · (m_raw / D_hw)
    
    double baseMultiplier = velocityCurveValue 
                          * params.pixelToAngular 
                          * params.emulatorSensitivity 
                          * params.resolutionScale 
                          * params.windowsPointerScale 
                          * dpiNorm;
    
    result.totalMultiplierX = baseMultiplier * params.sensitivityX;
    result.totalMultiplierY = baseMultiplier * params.sensitivityY;
    
    // Apply to input
    InputState finalState = input;
    finalState.deltaX *= result.totalMultiplierX;
    finalState.deltaY *= result.totalMultiplierY;
    finalState.velocity = finalState.magnitude();
    finalState.stage = InputState::Final;
    
    result.finalState = finalState;
    result.angularDeltaX = finalState.deltaX;
    result.angularDeltaY = finalState.deltaY;
    
    return result;
}

double SensitivityCalculator::effectiveSensitivity(const Parameters& params, double velocityCurve)
{
    // Returns the combined multiplier for display purposes
    return velocityCurve * params.totalMultiplier();
}

double SensitivityCalculator::calculate360Distance(const Parameters& params, double velocityCurve)
{
    // Calculate cm needed to rotate 360° in-game
    
    // Effective sensitivity (degrees per raw count)
    double degreesPerCount = effectiveSensitivity(params, velocityCurve);
    
    if (degreesPerCount <= 0.0) return 0.0;
    
    // Counts needed for 360°
    double countsFor360 = 360.0 / degreesPerCount;
    
    // Convert to inches (using mouse DPI)
    double inchesFor360 = countsFor360 / params.mouseDpi;
    
    // Convert to cm
    double cmFor360 = inchesFor360 * 2.54;
    
    return cmFor360;
}

} // namespace NeoZ
