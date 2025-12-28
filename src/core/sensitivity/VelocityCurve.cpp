#include "VelocityCurve.h"
#include <cmath>
#include <QDebug>

namespace NeoZ {

VelocityCurve::VelocityCurve(QObject* parent)
    : QObject(parent)
{
    applyPreset(Linear);
}

double VelocityCurve::apply(double velocity) const
{
    if (m_preset == Linear) {
        return 1.0;
    }
    
    return interpolate(velocity);
}

double VelocityCurve::interpolate(double velocity) const
{
    // Below low threshold: use low multiplier
    if (velocity <= m_lowThreshold) {
        return m_lowMultiplier;
    }
    
    // Above high threshold: use high multiplier
    if (velocity >= m_highThreshold) {
        return m_highMultiplier;
    }
    
    // In between: smooth S-curve interpolation
    // Normalized position in the transition zone [0, 1]
    double t = (velocity - m_lowThreshold) / (m_highThreshold - m_lowThreshold);
    
    // Smoothstep function for S-curve: 3t² - 2t³
    double s = t * t * (3.0 - 2.0 * t);
    
    // Two-stage interpolation:
    // [low -> mid] for t in [0, 0.5]
    // [mid -> high] for t in [0.5, 1]
    if (t < 0.5) {
        double localT = t * 2.0; // Remap to [0, 1]
        double localS = localT * localT * (3.0 - 2.0 * localT);
        return m_lowMultiplier + (m_midMultiplier - m_lowMultiplier) * localS;
    } else {
        double localT = (t - 0.5) * 2.0; // Remap to [0, 1]
        double localS = localT * localT * (3.0 - 2.0 * localT);
        return m_midMultiplier + (m_highMultiplier - m_midMultiplier) * localS;
    }
}

void VelocityCurve::setLowThreshold(double v)
{
    if (qFuzzyCompare(m_lowThreshold, v)) return;
    m_lowThreshold = v;
    m_preset = Custom;
    emit curveChanged();
}

void VelocityCurve::setHighThreshold(double v)
{
    if (qFuzzyCompare(m_highThreshold, v)) return;
    m_highThreshold = v;
    m_preset = Custom;
    emit curveChanged();
}

void VelocityCurve::setLowMultiplier(double v)
{
    if (qFuzzyCompare(m_lowMultiplier, v)) return;
    m_lowMultiplier = v;
    m_preset = Custom;
    emit curveChanged();
}

void VelocityCurve::setHighMultiplier(double v)
{
    if (qFuzzyCompare(m_highMultiplier, v)) return;
    m_highMultiplier = v;
    m_preset = Custom;
    emit curveChanged();
}

void VelocityCurve::setPreset(CurvePreset preset)
{
    if (m_preset == preset) return;
    applyPreset(preset);
}

void VelocityCurve::applyPreset(CurvePreset preset)
{
    m_preset = preset;
    
    switch (preset) {
    case Linear:
        // No velocity adjustment
        m_lowThreshold = 0.5;
        m_highThreshold = 5.0;
        m_lowMultiplier = 1.0;
        m_midMultiplier = 1.0;
        m_highMultiplier = 1.0;
        break;
        
    case SCurve:
        // Smooth transition, balanced
        m_lowThreshold = 0.3;
        m_highThreshold = 4.0;
        m_lowMultiplier = 0.85;
        m_midMultiplier = 1.0;
        m_highMultiplier = 1.15;
        break;
        
    case OneTap:
        // Precise at low velocity for headshots
        // Aim assist optimization: engage at low speeds
        m_lowThreshold = 0.2;
        m_highThreshold = 3.0;
        m_lowMultiplier = 0.7;   // Very precise at low speed
        m_midMultiplier = 0.95;
        m_highMultiplier = 1.1;  // Slightly faster for escapes
        break;
        
    case RedZone:
        // Higher sensitivity for spray control and flicks
        m_lowThreshold = 0.5;
        m_highThreshold = 6.0;
        m_lowMultiplier = 0.9;
        m_midMultiplier = 1.0;
        m_highMultiplier = 1.3;  // Fast flicks enhanced
        break;
        
    case Custom:
        // Don't change values for custom
        break;
    }
    
    qDebug() << "[VelocityCurve] Applied preset:" << preset
             << "| Low:" << m_lowMultiplier << "@" << m_lowThreshold
             << "| High:" << m_highMultiplier << "@" << m_highThreshold;
    
    emit curveChanged();
}

} // namespace NeoZ
