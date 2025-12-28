#include "SensitivityPipeline.h"
#include "../input/WindowsInputReader.h"
#include <QDebug>
#include <cmath>

namespace NeoZ {

SensitivityPipeline::SensitivityPipeline(QObject* parent)
    : QObject(parent)
    , m_velocityCurve(std::make_unique<VelocityCurve>(this))
    , m_hostNormalizer(std::make_unique<HostNormalizer>(this))
    , m_emulatorTranslator(std::make_unique<EmulatorTranslator>(this))
{
    // Connect sub-component signals
    connect(m_velocityCurve.get(), &VelocityCurve::curveChanged,
            this, &SensitivityPipeline::onSubComponentChanged);
    connect(m_hostNormalizer.get(), &HostNormalizer::parametersChanged,
            this, &SensitivityPipeline::onSubComponentChanged);
    connect(m_emulatorTranslator.get(), &EmulatorTranslator::parametersChanged,
            this, &SensitivityPipeline::onSubComponentChanged);
    
    // Forward preset confidence from HostNormalizer
    connect(m_hostNormalizer.get(), &HostNormalizer::presetConfidenceChanged,
            this, [this]() {
                m_presetConfidence = m_hostNormalizer->presetConfidence();
                emit presetConfidenceChanged();
            });
    
    // Start timers
    m_smoothingTimer.start();
    m_latencyTimer.start();
    
    qDebug() << "[SensitivityPipeline] Initialized with Input Authority OFF (safe mode)";
}

SensitivityPipeline::~SensitivityPipeline() = default;

InputState SensitivityPipeline::process(const InputState& rawInput)
{
    // Start latency measurement
    m_latencyTimer.restart();
    
    // ===== INPUT AUTHORITY GATE =====
    // When OFF, pass through raw input unmodified (safe mode)
    if (!m_inputAuthorityEnabled || m_simulateMode) {
        InputState passthrough = rawInput;
        passthrough.velocity = std::sqrt(rawInput.deltaX * rawInput.deltaX + rawInput.deltaY * rawInput.deltaY);
        emit inputProcessed(passthrough);
        return passthrough;
    }
    
    // ===== NEO-Z PRECISION AXIS CONTROL PIPELINE =====
    // Raw Δ → DPI norm → Win speed → Res norm → X/Y mult → Curve → Slow Zone → Smoothing → Drag Limit → Output
    
    double deltaX = rawInput.deltaX;
    double deltaY = rawInput.deltaY;
    
    // Step 1: DPI normalization (convert counts to inches)
    double dpiNormX = deltaX / static_cast<double>(m_mouseDpi);
    double dpiNormY = deltaY / static_cast<double>(m_mouseDpi);
    
    // Step 2: Windows cursor speed scaling
    double winSpeedScale = m_hostNormalizer->windowsPointerScale();
    double winScaledX = dpiNormX * winSpeedScale;
    double winScaledY = dpiNormY * winSpeedScale;
    
    // Step 3: Resolution normalization (Emulator Translation)
    // Desktop Mode: Assistive shaping (no emulator scaling)
    // ADB Mode: Full control (apply emulator resolution scaling)
    double resScale = 1.0;
    if (m_adbMode) {
        resScale = m_emulatorTranslator->resolutionScale();
    }
    
    double resNormX = winScaledX * resScale;
    double resNormY = winScaledY * resScale;
    
    // Step 4: Apply center-zero axis multipliers
    double axisX = resNormX * gainX();
    double axisY = resNormY * gainY();
    
    // Step 5: Calculate velocity and apply curve
    double velocity = std::sqrt(axisX * axisX + axisY * axisY);
    double curveValue = m_velocityCurve->apply(velocity);
    double curvedX = axisX * curveValue;
    double curvedY = axisY * curveValue;
    
    // Step 6: SLOW ZONE (AIM ASSIST FRIENDLY)
    // Free Fire aim assist engages when angular velocity is low.
    // We punish over-drag to prevent breaking aim assist.
    
    // 6a. Compute angular velocity: ω = |Δθ| / Δt
    qint64 elapsedMs = m_smoothingTimer.elapsed();
    m_smoothingTimer.restart();
    
    double dt = (elapsedMs > 0) ? (elapsedMs / 1000.0) : 0.001; // Avoid divide by zero
    double angularVelocity = velocity / dt; // velocity is |Δθ|
    
    // 6b. Define slow zone threshold
    // ω_threshold = ω_max * slowZone
    // ω_max ≈ 500 deg/s (typical fast flick)
    constexpr double OMEGA_MAX = 500.0; 
    double omegaThreshold = OMEGA_MAX * (m_slowZonePercent / 100.0);
    
    double slowZoneX = curvedX;
    double slowZoneY = curvedY;
    
    // 6c. Apply exponential damping
    // if ω < ω_threshold: scale = (ω / ω_threshold)^γ
    // γ = 1.6 - 2.2 (tunable, using 2.0 as sweet spot)
    constexpr double GAMMA = 2.0;
    
    if (angularVelocity < omegaThreshold && omegaThreshold > 0.0) {
        double ratio = angularVelocity / omegaThreshold;
        // Ensure ratio is not zero to avoid issues, though mathematically 0^2 is 0
        if (ratio < 0.001) ratio = 0.001;
        
        double scale = std::pow(ratio, GAMMA);
        
        // Apply scaling ("sticky but smooth")
        slowZoneX = curvedX * scale;
        slowZoneY = curvedY * scale;
    }
    
    // Step 7: Time-based smoothing with non-linear τ
    // τ = max(1, S^1.35) where S is smoothingMs
    // λ = e^(-Δt/τ)
    // Note: elapsedMs is already captured above
    
    double tau = smoothingTau();  // Uses non-linear mapping
    double lambda = 0.0;
    if (tau > 0.0 && elapsedMs > 0) {
        lambda = std::exp(-static_cast<double>(elapsedMs) / tau);
    }
    
    double smoothedX = lambda * m_prevDeltaX + (1.0 - lambda) * slowZoneX;
    double smoothedY = lambda * m_prevDeltaY + (1.0 - lambda) * slowZoneY;
    
    m_prevDeltaX = smoothedX;
    m_prevDeltaY = smoothedY;
    
    // Step 8: REPETITION DRAG LIMITER
    // Detect repetitive movement patterns and apply damping
    double dragX = smoothedX;
    double dragY = smoothedY;
    if (m_dragHistory.size() >= 2) {
        auto& last = m_dragHistory.back();
        double magnitude = std::sqrt(smoothedX * smoothedX + smoothedY * smoothedY);
        double lastMag = std::sqrt(last.first * last.first + last.second * last.second);
        if (magnitude > 0.001 && lastMag > 0.001) {
            // Cosine similarity
            double dot = smoothedX * last.first + smoothedY * last.second;
            double similarity = dot / (magnitude * lastMag);
            if (similarity > DRAG_SIMILARITY_THRESHOLD) {
                dragX *= DRAG_DAMPING;
                dragY *= DRAG_DAMPING;
            }
        }
    }
    m_dragHistory.push_back({smoothedX, smoothedY});
    while (m_dragHistory.size() > DRAG_HISTORY_SIZE) {
        m_dragHistory.pop_front();
    }
    
    // Step 9: Apply final sensitivity multipliers (clamped if safe zone enabled)
    double finalX = dragX * m_sensitivityX;
    double finalY = dragY * m_sensitivityY;
    
    if (m_safeZoneClampEnabled) {
        // Clamp to prevent overshoot
        finalX = qBound(-100.0, finalX, 100.0);
        finalY = qBound(-100.0, finalY, 100.0);
    }
    
    // Measure latency
    m_latencyMs = m_latencyTimer.nsecsElapsed() / 1000000.0;
    emit latencyChanged();
    
    // Build output state
    InputState result;
    result.deltaX = finalX;
    result.deltaY = finalY;
    result.velocity = velocity;
    result.timestamp = rawInput.timestamp;
    
    emit inputProcessed(result);
    
    return result;
}

SensitivityCalculator::Parameters SensitivityPipeline::buildParameters(double velocity) const
{
    SensitivityCalculator::Parameters params;
    
    // Direct parameters
    params.sensitivityX = m_sensitivityX * gainX();
    params.sensitivityY = m_sensitivityY * gainY();
    params.mouseDpi = m_mouseDpi;
    params.pixelToAngular = m_pixelToAngular;
    
    // From host normalizer
    params.windowsPointerScale = m_hostNormalizer->windowsPointerScale();
    
    // From emulator translator
    params.emulatorSensitivity = m_emulatorTranslator->sensitivityScalar();
    params.resolutionScale = m_emulatorTranslator->resolutionScale();
    
    // Velocity curve
    params.velocityCurve = m_velocityCurve->apply(velocity);
    
    return params;
}

double SensitivityPipeline::effectiveSensitivity() const
{
    auto params = buildParameters(1.0);
    return SensitivityCalculator::effectiveSensitivity(params, 1.0);
}

double SensitivityPipeline::cm360() const
{
    auto params = buildParameters(1.0);
    return SensitivityCalculator::calculate360Distance(params, 1.0);
}

double SensitivityPipeline::effectiveAngularSensitivity() const
{
    // Calculate degrees per cm of mouse movement
    // Formula: (360 / cm360) = degrees per cm
    double distance = cm360();
    if (distance <= 0.0) return 0.0;
    return 360.0 / distance;
}

void SensitivityPipeline::setInputAuthorityEnabled(bool enabled)
{
    if (m_inputAuthorityEnabled == enabled) return;
    m_inputAuthorityEnabled = enabled;
    qDebug() << "[SensitivityPipeline] Input Authority:" << (enabled ? "ENABLED" : "DISABLED (safe mode)");
    emit inputAuthorityChanged();
}

void SensitivityPipeline::setSafeZoneClampEnabled(bool enabled)
{
    if (m_safeZoneClampEnabled == enabled) return;
    m_safeZoneClampEnabled = enabled;
    qDebug() << "[SensitivityPipeline] Safe Zone Clamp:" << (enabled ? "ON" : "OFF");
    emit settingsChanged();
}

void SensitivityPipeline::setSensitivityX(double value)
{
    value = qBound(0.01, value, 10.0);
    if (qFuzzyCompare(m_sensitivityX, value)) return;
    m_sensitivityX = value;
    qDebug() << "[SensitivityPipeline] Sensitivity X:" << value;
    emit settingsChanged();
}

void SensitivityPipeline::setSensitivityY(double value)
{
    value = qBound(0.01, value, 10.0);
    if (qFuzzyCompare(m_sensitivityY, value)) return;
    m_sensitivityY = value;
    qDebug() << "[SensitivityPipeline] Sensitivity Y:" << value;
    emit settingsChanged();
}

void SensitivityPipeline::setMouseDpi(int dpi)
{
    dpi = qBound(100, dpi, 16000);
    if (m_mouseDpi == dpi) return;
    m_mouseDpi = dpi;
    m_hostNormalizer->setMouseDpi(dpi);
    qDebug() << "[SensitivityPipeline] Mouse DPI:" << dpi;
    emit settingsChanged();
}

void SensitivityPipeline::setAxisMultiplierX(double value)
{
    value = qBound(-1.0, value, 1.0);
    if (qFuzzyCompare(m_axisMultiplierX, value)) return;
    m_axisMultiplierX = value;
    qDebug() << "[SensitivityPipeline] Axis Multiplier X:" << value << "-> Gain:" << gainX();
    emit settingsChanged();
}

void SensitivityPipeline::setAxisMultiplierY(double value)
{
    value = qBound(-1.0, value, 1.0);
    if (qFuzzyCompare(m_axisMultiplierY, value)) return;
    m_axisMultiplierY = value;
    qDebug() << "[SensitivityPipeline] Axis Multiplier Y:" << value << "-> Gain:" << gainY();
    emit settingsChanged();
}

void SensitivityPipeline::setGainFactor(double value)
{
    value = qBound(0.1, value, 1.0);
    if (qFuzzyCompare(m_gainFactor, value)) return;
    m_gainFactor = value;
    qDebug() << "[SensitivityPipeline] Gain Factor (k):" << value;
    emit settingsChanged();
}

// Non-linear smoothing tau getter: τ = max(1, S^1.35)
double SensitivityPipeline::smoothingTau() const
{
    if (m_smoothingMs <= 0.0) return 0.0;
    return std::max(1.0, std::pow(m_smoothingMs, 1.35));
}

void SensitivityPipeline::setSmoothingMs(double value)
{
    value = qBound(0.0, value, 200.0);  // Extended range 0-200ms
    if (qFuzzyCompare(m_smoothingMs, value)) return;
    m_smoothingMs = value;
    
    // Determine label
    QString label;
    if (value <= 10) label = "Raw";
    else if (value <= 60) label = "Competitive";
    else if (value <= 120) label = "Assist";
    else label = "Training";
    
    qDebug() << "[SensitivityPipeline] Smoothing:" << value << "ms (τ=" << smoothingTau() << ") [" << label << "]";
    emit settingsChanged();
}

void SensitivityPipeline::setSlowZonePercent(double value)
{
    value = qBound(1.0, value, 100.0);  // Range 1-100%
    if (qFuzzyCompare(m_slowZonePercent, value)) return;
    m_slowZonePercent = value;
    
    // Determine label
    QString label;
    if (value <= 10) label = "Manual";
    else if (value <= 30) label = "Headshot";
    else if (value <= 60) label = "Body Lock";
    else label = "Sticky";
    
    qDebug() << "[SensitivityPipeline] Slow Zone:" << value << "% [" << label << "]";
    emit settingsChanged();
}

void SensitivityPipeline::linkWindowsInputReader(WindowsInputReader* reader)
{
    if (m_windowsReader) {
        disconnect(m_windowsReader, nullptr, this, nullptr);
    }
    
    m_windowsReader = reader;
    
    if (reader) {
        connect(reader, &WindowsInputReader::settingsChanged, this, [this]() {
            m_hostNormalizer->setWindowsPointerScale(m_windowsReader->pointerSpeedMultiplier());
            m_hostNormalizer->setAccelerationEnabled(m_windowsReader->enhancePrecisionEnabled());
            emit pipelineRecalculated();
        });
        
        m_hostNormalizer->setWindowsPointerScale(reader->pointerSpeedMultiplier());
        m_hostNormalizer->setAccelerationEnabled(reader->enhancePrecisionEnabled());
    }
}

void SensitivityPipeline::resetToDefaults()
{
    m_sensitivityX = 1.0;
    m_sensitivityY = 1.0;
    m_mouseDpi = 800;
    m_axisMultiplierX = 0.0;
    m_axisMultiplierY = 0.0;
    m_gainFactor = 0.6;
    m_smoothingMs = 16.0;
    m_slowZonePercent = 20.0;  // Headshot sweet spot
    m_prevDeltaX = 0.0;
    m_prevDeltaY = 0.0;
    
    m_velocityCurve->applyPreset(VelocityCurve::Linear);
    m_hostNormalizer->setMouseDpi(800);
    m_hostNormalizer->setWindowsPointerScale(1.0);
    m_hostNormalizer->setAccelerationEnabled(false);
    m_emulatorTranslator->applyPreset(EmulatorTranslator::Unknown);
    
    qDebug() << "[SensitivityPipeline] Reset to defaults (Precision Axis Control)";
    emit settingsChanged();
}

void SensitivityPipeline::onSubComponentChanged()
{
    emit settingsChanged();
    emit pipelineRecalculated();
}

void SensitivityPipeline::takeSnapshot()
{
    m_snapshot.sensitivityX = m_sensitivityX;
    m_snapshot.sensitivityY = m_sensitivityY;
    m_snapshot.axisMultiplierX = m_axisMultiplierX;
    m_snapshot.axisMultiplierY = m_axisMultiplierY;
    m_snapshot.gainFactor = m_gainFactor;
    m_snapshot.smoothingMs = m_smoothingMs;
    m_snapshot.slowZonePercent = m_slowZonePercent;
    m_snapshot.mouseDpi = m_mouseDpi;
    m_hasSnapshot = true;
    qDebug() << "[SensitivityPipeline] Snapshot taken";
}

void SensitivityPipeline::rollback()
{
    if (!m_hasSnapshot) {
        qDebug() << "[SensitivityPipeline] No snapshot to rollback to";
        return;
    }
    
    m_sensitivityX = m_snapshot.sensitivityX;
    m_sensitivityY = m_snapshot.sensitivityY;
    m_axisMultiplierX = m_snapshot.axisMultiplierX;
    m_axisMultiplierY = m_snapshot.axisMultiplierY;
    m_gainFactor = m_snapshot.gainFactor;
    m_smoothingMs = m_snapshot.smoothingMs;
    m_slowZonePercent = m_snapshot.slowZonePercent;
    m_mouseDpi = m_snapshot.mouseDpi;
    
    m_hostNormalizer->setMouseDpi(m_mouseDpi);
    
    qDebug() << "[SensitivityPipeline] Rolled back to snapshot";
    emit settingsChanged();
}

void SensitivityPipeline::enableSimulateMode(bool enable)
{
    m_simulateMode = enable;
    qDebug() << "[SensitivityPipeline] Simulate mode:" << (enable ? "ON" : "OFF");
}

void SensitivityPipeline::setAdbMode(bool enabled)
{
    if (m_adbMode == enabled) return;
    m_adbMode = enabled;
    qDebug() << "[SensitivityPipeline] ADB Mode:" << (enabled ? "ON (Full Control)" : "OFF (Assistive Shaping)");
    emit settingsChanged();
}

} // namespace NeoZ
