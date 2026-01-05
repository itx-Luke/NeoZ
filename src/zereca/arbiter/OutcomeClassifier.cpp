#include "OutcomeClassifier.h"
#include <QDebug>
#include <cmath>

namespace Zereca {

OutcomeClassifier::OutcomeClassifier(QObject* parent)
    : QObject(parent)
{
}

OutcomeClassifier::Result OutcomeClassifier::classify(
    const BaselineMetrics& baseline,
    const BaselineMetrics& current,
    bool hadCrash,
    bool hadThermalEvent)
{
    Result result;
    
    // ===== RULE 1: Safety events override everything =====
    if (hadThermalEvent) {
        result.outcome = Outcome::NEGATIVE_SAFETY;
        result.delta = 0.0f;
        result.confidence = 1.0f;
        result.reason = "Thermal throttling event detected";
        result.shouldRollback = true;
        result.probationSeverity = Severity::CRITICAL;
        
        qWarning() << "[Zereca] NEGATIVE_SAFETY: Thermal event";
        emit classified(result.outcome, result.delta);
        return result;
    }
    
    // ===== RULE 2: Stability events (crash/stutter) =====
    if (hadCrash) {
        result.outcome = Outcome::NEGATIVE_STABILITY;
        result.delta = 0.0f;
        result.confidence = 1.0f;
        result.reason = "Application crash detected";
        result.shouldRollback = true;
        result.probationSeverity = Severity::MEDIUM;
        
        qWarning() << "[Zereca] NEGATIVE_STABILITY: App crash";
        emit classified(result.outcome, result.delta);
        return result;
    }
    
    // ===== RULE 3: Calculate performance delta =====
    result.delta = calculatePerformanceDelta(baseline, current);
    
    // Estimate confidence based on observation duration
    float durationFactor = std::min(1.0f, 
        static_cast<float>(current.observationDurationMs) / m_thresholds.positiveSustainedMs);
    result.confidence = durationFactor * 0.9f;  // Max 90% confidence from duration alone
    
    // ===== RULE 4: Classify based on delta =====
    if (result.delta >= m_thresholds.positiveMinDelta && 
        result.confidence >= m_thresholds.confidenceRequired) {
        // 🟢 POSITIVE: Measurable improvement
        result.outcome = Outcome::POSITIVE;
        result.reason = QString("Performance improved by %1%")
                        .arg(result.delta * 100, 0, 'f', 1);
        result.shouldCommit = true;
        
        qDebug() << "[Zereca] POSITIVE: delta=" << result.delta;
    }
    else if (result.delta <= m_thresholds.negativeMaxRegression) {
        // 🟠 NEGATIVE_STABILITY: Significant regression
        result.outcome = Outcome::NEGATIVE_STABILITY;
        result.reason = QString("Performance regressed by %1%")
                        .arg(std::abs(result.delta) * 100, 0, 'f', 1);
        result.shouldRollback = true;
        result.probationSeverity = Severity::LOW;  // FPS regression only
        
        qDebug() << "[Zereca] NEGATIVE_STABILITY: regression=" << result.delta;
    }
    else {
        // 🟡 NEUTRAL: No significant change (placebo)
        result.outcome = Outcome::NEUTRAL;
        result.reason = QString("Delta %1% below threshold (±%2%)")
                        .arg(result.delta * 100, 0, 'f', 1)
                        .arg(m_thresholds.positiveMinDelta * 100, 0, 'f', 0);
        result.shouldRevert = true;
        // No probation for neutral - just STATE_INEFFECTIVE
        
        qDebug() << "[Zereca] NEUTRAL: delta=" << result.delta << "(ineffective)";
    }
    
    emit classified(result.outcome, result.delta);
    return result;
}

float OutcomeClassifier::calculatePerformanceDelta(
    const BaselineMetrics& baseline,
    const BaselineMetrics& current)
{
    // Primary metric: FPS improvement
    if (baseline.fps > 0 && current.fps > 0) {
        float fpsDelta = (current.fps - baseline.fps) / baseline.fps;
        
        // Secondary: Frame time reduction (weighted 30%)
        float frameTimeDelta = 0.0f;
        if (baseline.avgFrameTime > 0 && current.avgFrameTime > 0) {
            // Lower frame time is better, so invert
            frameTimeDelta = (baseline.avgFrameTime - current.avgFrameTime) / baseline.avgFrameTime;
        }
        
        // Tertiary: Variance reduction (weighted 20%)
        float varianceDelta = 0.0f;
        if (baseline.fpsVariance > 0 && current.fpsVariance > 0) {
            // Lower variance is better
            varianceDelta = (baseline.fpsVariance - current.fpsVariance) / baseline.fpsVariance;
        }
        
        // Weighted composite score
        return (fpsDelta * 0.5f) + (frameTimeDelta * 0.3f) + (varianceDelta * 0.2f);
    }
    
    // Fallback: Use frame time only
    if (baseline.avgFrameTime > 0 && current.avgFrameTime > 0) {
        return (baseline.avgFrameTime - current.avgFrameTime) / baseline.avgFrameTime;
    }
    
    return 0.0f;
}

} // namespace Zereca
