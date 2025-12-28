#ifndef NEOZ_VELOCITYCURVE_H
#define NEOZ_VELOCITYCURVE_H

#include <QObject>
#include <vector>

namespace NeoZ {

/**
 * @brief Implements C(v) velocity-based sensitivity curve.
 * 
 * The velocity curve modifies sensitivity based on mouse speed.
 * This is critical for aim assist optimization in Free Fire:
 * - Low velocity = precise aiming (possibly lower sens)
 * - Medium velocity = normal tracking
 * - High velocity = fast flicks (possibly higher sens)
 * 
 * From the Neo-Z Master Equation: C(v) is the velocity curve function.
 */
class VelocityCurve : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(double lowThreshold READ lowThreshold WRITE setLowThreshold NOTIFY curveChanged)
    Q_PROPERTY(double highThreshold READ highThreshold WRITE setHighThreshold NOTIFY curveChanged)
    Q_PROPERTY(double lowMultiplier READ lowMultiplier WRITE setLowMultiplier NOTIFY curveChanged)
    Q_PROPERTY(double highMultiplier READ highMultiplier WRITE setHighMultiplier NOTIFY curveChanged)
    Q_PROPERTY(CurvePreset preset READ preset WRITE setPreset NOTIFY curveChanged)
    
public:
    enum CurvePreset {
        Linear,      // No velocity adjustment (C(v) = 1.0)
        SCurve,      // Smooth S-curve transition
        OneTap,      // Optimized for one-tap headshots (low sens at low velocity)
        RedZone,     // Optimized for spray control (higher sens at high velocity)
        Custom       // User-defined curve
    };
    Q_ENUM(CurvePreset)
    
    explicit VelocityCurve(QObject* parent = nullptr);
    ~VelocityCurve() override = default;
    
    // Apply the curve to a velocity value, returns multiplier C(v)
    Q_INVOKABLE double apply(double velocity) const;
    
    // Getters
    double lowThreshold() const { return m_lowThreshold; }
    double highThreshold() const { return m_highThreshold; }
    double lowMultiplier() const { return m_lowMultiplier; }
    double highMultiplier() const { return m_highMultiplier; }
    CurvePreset preset() const { return m_preset; }
    
    // Setters
    void setLowThreshold(double v);
    void setHighThreshold(double v);
    void setLowMultiplier(double v);
    void setHighMultiplier(double v);
    void setPreset(CurvePreset preset);
    
    // Apply preset values
    Q_INVOKABLE void applyPreset(CurvePreset preset);
    
signals:
    void curveChanged();
    
private:
    // Smooth interpolation between low and high multipliers
    double interpolate(double velocity) const;
    
    // Curve parameters
    double m_lowThreshold = 0.5;    // Velocity below this uses lowMultiplier
    double m_highThreshold = 5.0;   // Velocity above this uses highMultiplier
    double m_lowMultiplier = 0.8;   // Multiplier at low velocity
    double m_highMultiplier = 1.2;  // Multiplier at high velocity
    double m_midMultiplier = 1.0;   // Multiplier at mid velocity (baseline)
    
    CurvePreset m_preset = Linear;
};

} // namespace NeoZ

#endif // NEOZ_VELOCITYCURVE_H
