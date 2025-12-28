#include "EmulatorTranslator.h"
#include "../adb/AdbConnector.h"
#include <QDebug>
#include <QRegularExpression>

namespace NeoZ {

EmulatorTranslator::EmulatorTranslator(QObject* parent)
    : QObject(parent)
{
}

InputState EmulatorTranslator::translate(const InputState& input) const
{
    InputState result = input;
    
    // Apply emulator sensitivity scalar (E_s)
    result.deltaX *= m_sensitivityScalar;
    result.deltaY *= m_sensitivityScalar;
    
    // Apply resolution scale (E_r)
    result.deltaX *= m_resolutionScale;
    result.deltaY *= m_resolutionScale;
    
    // Update velocity
    result.velocity = result.magnitude();
    result.stage = InputState::EmulatorMapped;
    
    return result;
}

double EmulatorTranslator::calculateResolutionScale(const QSize& resolution) const
{
    // Scale based on width ratio to reference resolution
    double widthScale = static_cast<double>(resolution.width()) / REFERENCE_WIDTH;
    double heightScale = static_cast<double>(resolution.height()) / REFERENCE_HEIGHT;
    
    // Use average of width and height scales
    return (widthScale + heightScale) / 2.0;
}

void EmulatorTranslator::setPreset(EmulatorPreset preset)
{
    if (m_preset == preset) return;
    applyPreset(preset);
}

void EmulatorTranslator::applyPreset(EmulatorPreset preset)
{
    m_preset = preset;
    
    switch (preset) {
    case BlueStacks:
        // BlueStacks has its own sensitivity multiplier
        m_sensitivityScalar = 1.0;
        // BlueStacks typically runs at native resolution
        break;
        
    case MSIAppPlayer:
        // MSI App Player (BlueStacks variant)
        m_sensitivityScalar = 1.0;
        break;
        
    case LDPlayer:
        // LDPlayer has slightly different input handling
        m_sensitivityScalar = 0.95;
        break;
        
    case NoxPlayer:
        // Nox has a different input curve
        m_sensitivityScalar = 1.05;
        break;
        
    case MEmu:
        m_sensitivityScalar = 1.0;
        break;
        
    case HDPlayer:
        // HDPlayer - user's primary emulator target
        m_sensitivityScalar = 1.0;
        break;
        
    case Unknown:
    case Custom:
        // Keep current values for unknown/custom
        break;
    }
    
    // Recalculate resolution scale
    m_resolutionScale = calculateResolutionScale(m_emulatorResolution);
    
    qDebug() << "[EmulatorTranslator] Applied preset:" << presetName(preset)
             << "| E_s:" << m_sensitivityScalar
             << "| E_r:" << m_resolutionScale;
    
    emit parametersChanged();
}

void EmulatorTranslator::setSensitivityScalar(double scalar)
{
    if (scalar < 0.1) scalar = 0.1;
    if (scalar > 10.0) scalar = 10.0;
    
    if (qFuzzyCompare(m_sensitivityScalar, scalar)) return;
    m_sensitivityScalar = scalar;
    m_preset = Custom;
    
    emit parametersChanged();
}

void EmulatorTranslator::setResolutionScale(double scale)
{
    if (scale < 0.1) scale = 0.1;
    if (scale > 4.0) scale = 4.0;
    
    if (qFuzzyCompare(m_resolutionScale, scale)) return;
    m_resolutionScale = scale;
    m_preset = Custom;
    
    emit parametersChanged();
}

void EmulatorTranslator::setEmulatorResolution(const QSize& resolution)
{
    if (m_emulatorResolution == resolution) return;
    m_emulatorResolution = resolution;
    
    // Auto-calculate resolution scale
    m_resolutionScale = calculateResolutionScale(resolution);
    
    qDebug() << "[EmulatorTranslator] Resolution:" << resolution.width() 
             << "x" << resolution.height()
             << "| Scale:" << m_resolutionScale;
    
    emit parametersChanged();
}

void EmulatorTranslator::setConnected(bool connected)
{
    if (m_connected == connected) return;
    m_connected = connected;
    emit connectionChanged();
}

QString EmulatorTranslator::presetName(EmulatorPreset preset)
{
    switch (preset) {
    case Unknown:     return QStringLiteral("Unknown");
    case BlueStacks:  return QStringLiteral("BlueStacks");
    case MSIAppPlayer: return QStringLiteral("MSI App Player");
    case LDPlayer:    return QStringLiteral("LDPlayer");
    case NoxPlayer:   return QStringLiteral("Nox Player");
    case MEmu:        return QStringLiteral("MEmu");
    case HDPlayer:    return QStringLiteral("HDPlayer");
    case Custom:      return QStringLiteral("Custom");
    }
    return QStringLiteral("Unknown");
}

// ========== ADB DPI SYNC ==========
bool EmulatorTranslator::syncEmulatorDpi(int densityDpi)
{
    if (!m_adbConnector || !m_connected) {
        qDebug() << "[EmulatorTranslator] Cannot sync DPI: no ADB connection";
        return false;
    }
    
    // Clamp to valid Android DPI range
    densityDpi = qBound(120, densityDpi, 640);
    
    // Execute: adb shell wm density <value>
    QString command = QString("wm density %1").arg(densityDpi);
    QString result = m_adbConnector->executeCommand(command);
    
    // Verify by reading back
    int currentDpi = readEmulatorDpi();
    if (currentDpi == densityDpi) {
        m_emulatorDpi = densityDpi;
        qDebug() << "[EmulatorTranslator] ✓ Synced emulator DPI to:" << densityDpi;
        emit parametersChanged();
        return true;
    }
    
    qDebug() << "[EmulatorTranslator] ✗ DPI sync failed. Expected:" << densityDpi << "Got:" << currentDpi;
    return false;
}

int EmulatorTranslator::readEmulatorDpi()
{
    if (!m_adbConnector || !m_connected) {
        qDebug() << "[EmulatorTranslator] Cannot read DPI: no ADB connection";
        return m_emulatorDpi;
    }
    
    // Execute: adb shell wm density
    QString result = m_adbConnector->executeCommand("wm density");
    
    // Parse output: "Physical density: 320" or "Override density: 480"
    QRegularExpression re("(?:Physical|Override) density:\\s*(\\d+)");
    QRegularExpressionMatch match = re.match(result);
    
    if (match.hasMatch()) {
        int dpi = match.captured(1).toInt();
        qDebug() << "[EmulatorTranslator] Current emulator DPI:" << dpi;
        m_emulatorDpi = dpi;
        return dpi;
    }
    
    qDebug() << "[EmulatorTranslator] Could not parse DPI from:" << result;
    return m_emulatorDpi;
}

} // namespace NeoZ

