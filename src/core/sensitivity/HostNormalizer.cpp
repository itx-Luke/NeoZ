#include "HostNormalizer.h"
#include <cmath>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

namespace NeoZ {

HostNormalizer::HostNormalizer(QObject* parent)
    : QObject(parent)
{
    // Auto-detect screen resolution and refresh rate
    if (QGuiApplication::primaryScreen()) {
        QScreen* screen = QGuiApplication::primaryScreen();
        m_screenWidth = screen->size().width();
        m_screenHeight = screen->size().height();
        m_refreshRate = qRound(screen->refreshRate());
        qDebug() << "[HostNormalizer] Detected:" << m_screenWidth << "x" << m_screenHeight 
                 << "@" << m_refreshRate << "Hz";
    }
    updatePresetConfidence();
}

InputState HostNormalizer::normalize(const InputState& input) const
{
    InputState result = input;
    
    // Step 1: Apply acceleration correction if enabled
    if (m_accelerationEnabled) {
        double velocity = input.velocity;
        double correctionFactor = applyAccelerationCurve(velocity);
        result.deltaX /= correctionFactor;
        result.deltaY /= correctionFactor;
    }
    
    // Step 2: DPI normalization
    double dpiNorm = dpiNormalizationFactor();
    result.deltaX *= dpiNorm;
    result.deltaY *= dpiNorm;
    
    // Step 3: Apply Windows pointer scale
    result.deltaX *= m_windowsPointerScale;
    result.deltaY *= m_windowsPointerScale;
    
    // Step 4: Apply refresh rate compensation (higher Hz = faster perceived motion)
    double hzFactor = refreshRateFactor();
    result.deltaX *= hzFactor;
    result.deltaY *= hzFactor;
    
    // Update velocity
    result.velocity = result.magnitude();
    result.stage = InputState::HostNormalized;
    
    return result;
}

double HostNormalizer::dpiNormalizationFactor() const
{
    return static_cast<double>(REFERENCE_DPI) / static_cast<double>(m_mouseDpi);
}

// ========== NEO-Z PRECISION ENGINE - ANGULAR DELTA ==========
// FOV-based formula: Δθ = (Δcounts / DPI) × (FOVx / screenWidth)
// This makes sensitivity resolution-independent (angular, not pixel-based)
HostNormalizer::AngularDelta HostNormalizer::calculateAngularDelta(double deltaX, double deltaY) const
{
    AngularDelta result;
    
    // Convert mouse counts to inches of physical movement
    double inchesX = deltaX / static_cast<double>(m_mouseDpi);
    double inchesY = deltaY / static_cast<double>(m_mouseDpi);
    
    // Convert to angular displacement (degrees) using FOV-based formula
    // Δθ = (Δcounts / DPI) × (FOVx / screenWidth)
    // This is resolution-independent: same physical mouse movement = same angular rotation
    double degreesPerInch = m_fovX / static_cast<double>(m_screenWidth) * m_mouseDpi;
    
    result.x = inchesX * degreesPerInch;
    result.y = inchesY * degreesPerInch;  // Use same ratio for Y to maintain 1:1 aspect
    
    return result;
}

double HostNormalizer::angularSensitivity() const
{
    // Calculate degrees per cm of mouse movement using FOV-based formula
    // Formula: degreesPerCm = (FOVx / screenWidth) × DPI / 2.54
    // 1 inch = 2.54 cm
    double degreesPerInch = m_fovX / static_cast<double>(m_screenWidth) * m_mouseDpi;
    return degreesPerInch / 2.54;  // Convert to degrees per cm
}

// Refresh rate compensation: HzFactor = ReferenceHz / CurrentHz
// Clamped to prevent extreme adjustments: 0.75 ≤ HzFactor ≤ 1.25
double HostNormalizer::refreshRateFactor() const
{
    if (m_refreshRate <= 0) return 1.0;
    double rawFactor = static_cast<double>(REFERENCE_HZ) / static_cast<double>(m_refreshRate);
    // Clamp to [0.75, 1.25] per Neo-Z Precision Engine spec
    return qBound(0.75, rawFactor, 1.25);
}

double HostNormalizer::applyAccelerationCurve(double velocity) const
{
    // Approximate Windows EPP curve
    if (velocity < 3.5) {
        return 0.3 + (velocity / 3.5) * 0.2;
    } else if (velocity < 7.0) {
        double t = (velocity - 3.5) / 3.5;
        return 0.5 + t * 0.5;
    } else {
        double excess = velocity - 7.0;
        return 1.0 + excess * 0.075;
    }
}

void HostNormalizer::updatePresetConfidence()
{
    // Determine preset confidence based on resolution matching
    // Native: matches common gaming resolutions exactly
    // Scaled: non-standard but compensatable
    // Mismatch: dangerous combination
    
    bool isNativeRes = (m_screenWidth == 1920 && m_screenHeight == 1080) ||
                       (m_screenWidth == 2560 && m_screenHeight == 1440) ||
                       (m_screenWidth == 3840 && m_screenHeight == 2160) ||
                       (m_screenWidth == 1280 && m_screenHeight == 720);
    
    bool isGoodHz = (m_refreshRate >= 60 && m_refreshRate <= 360);
    
    int newConfidence;
    if (isNativeRes && isGoodHz) {
        newConfidence = Native;
    } else if (!isNativeRes && isGoodHz) {
        newConfidence = Scaled;
    } else {
        newConfidence = Mismatch;
    }
    
    if (m_presetConfidence != newConfidence) {
        m_presetConfidence = newConfidence;
        emit presetConfidenceChanged();
    }
}

void HostNormalizer::setMouseDpi(int dpi)
{
    dpi = qBound(100, dpi, 16000);
    if (m_mouseDpi == dpi) return;
    m_mouseDpi = dpi;
    
    qDebug() << "[HostNormalizer] Mouse DPI:" << dpi 
             << "| Norm factor:" << dpiNormalizationFactor()
             << "| Angular sens:" << angularSensitivity() << "°/cm";
    
    emit parametersChanged();
}

void HostNormalizer::setWindowsPointerScale(double scale)
{
    if (qFuzzyCompare(m_windowsPointerScale, scale)) return;
    m_windowsPointerScale = scale;
    qDebug() << "[HostNormalizer] Windows pointer scale:" << scale;
    emit parametersChanged();
}

void HostNormalizer::setAccelerationEnabled(bool enabled)
{
    if (m_accelerationEnabled == enabled) return;
    m_accelerationEnabled = enabled;
    qDebug() << "[HostNormalizer] Acceleration compensation:" 
             << (enabled ? "ENABLED" : "DISABLED");
    emit parametersChanged();
}

void HostNormalizer::setScreenWidth(int width)
{
    width = qBound(640, width, 7680);
    if (m_screenWidth == width) return;
    m_screenWidth = width;
    qDebug() << "[HostNormalizer] Screen width:" << width;
    updatePresetConfidence();
    emit parametersChanged();
}

void HostNormalizer::setScreenHeight(int height)
{
    height = qBound(480, height, 4320);
    if (m_screenHeight == height) return;
    m_screenHeight = height;
    qDebug() << "[HostNormalizer] Screen height:" << height;
    updatePresetConfidence();
    emit parametersChanged();
}

void HostNormalizer::setRefreshRate(int hz)
{
    hz = qBound(30, hz, 500);
    if (m_refreshRate == hz) return;
    m_refreshRate = hz;
    qDebug() << "[HostNormalizer] Refresh rate:" << hz << "Hz | Factor:" << refreshRateFactor();
    updatePresetConfidence();
    emit parametersChanged();
}

void HostNormalizer::setFovX(double fov)
{
    fov = qBound(30.0, fov, 180.0);  // Reasonable FOV range
    if (qFuzzyCompare(m_fovX, fov)) return;
    m_fovX = fov;
    qDebug() << "[HostNormalizer] FOVx:" << fov << "° | Angular sens:" << angularSensitivity() << "°/cm";
    emit parametersChanged();
}

} // namespace NeoZ
