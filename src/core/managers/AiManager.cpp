#include "AiManager.h"
#include <QDebug>

namespace NeoZ {

AiManager::AiManager(QObject* parent)
    : QObject(parent)
{
    m_advisor = std::make_unique<AiAdvisor>(this);
    m_realTimeAi = new RealTimeSensitivityAI(this);
    
    connect(m_advisor.get(), &AiAdvisor::recommendationReady, 
            this, &AiManager::onRecommendationReady);
    
    connect(m_realTimeAi, &RealTimeSensitivityAI::adjusted,
            this, &AiManager::onRealTimeAdjusted);
    
    connect(m_realTimeAi, &RealTimeSensitivityAI::metricsChanged,
            this, &AiManager::realTimeMetricsChanged);
}

AiManager::~AiManager()
{
}

void AiManager::setAiEnabled(bool enabled)
{
    if (m_aiEnabled == enabled) return;
    m_aiEnabled = enabled;
    emit aiEnabledChanged();
    qDebug() << "[AiManager] AI enabled:" << enabled;
}

void AiManager::setAiConfidenceThreshold(double threshold)
{
    threshold = qBound(0.0, threshold, 1.0);
    if (qFuzzyCompare(m_aiConfidenceThreshold, threshold)) return;
    m_aiConfidenceThreshold = threshold;
    emit aiEnabledChanged();
}

void AiManager::requestRecommendation(const SystemSnapshot& snapshot)
{
    if (!m_aiEnabled) {
        qDebug() << "[AiManager] AI not enabled, skipping recommendation";
        return;
    }
    
    m_advisor->requestTuning(snapshot);
}

void AiManager::setRealTimeAiEnabled(bool enabled)
{
    if (m_realTimeAi) {
        m_realTimeAi->setEnabled(enabled);
        emit realTimeAiChanged();
    }
}

void AiManager::registerShot(bool hit, bool headshot)
{
    if (m_realTimeAi) {
        m_realTimeAi->processShotResult(hit, headshot);
    }
}

void AiManager::onRecommendationReady(const TuningRecommendation& rec)
{
    if (!rec.isValid) {
        qDebug() << "[AiManager] Invalid recommendation received";
        return;
    }
    
    // Check confidence threshold
    if (rec.confidence < m_aiConfidenceThreshold) {
        qDebug() << "[AiManager] Recommendation confidence too low:" 
                 << rec.confidence << "<" << m_aiConfidenceThreshold;
        return;
    }
    
    m_recommendedX = rec.xMultiplier;
    m_recommendedY = rec.yMultiplier;
    m_recommendationConfidence = rec.confidence;
    
    // Build summary
    QStringList parts;
    parts << QString("X: %1, Y: %2").arg(rec.xMultiplier, 0, 'f', 2).arg(rec.yMultiplier, 0, 'f', 2);
    parts << QString("Confidence: %1%").arg(rec.confidence * 100, 0, 'f', 0);
    if (!rec.reasoning.isEmpty()) {
        parts << rec.reasoning.first();
    }
    m_lastRecommendationSummary = parts.join(" | ");
    
    emit recommendationChanged();
    emit recommendationReady(m_recommendedX, m_recommendedY, m_recommendationConfidence);
    
    qDebug() << "[AiManager] Recommendation ready:" << m_lastRecommendationSummary;
}

void AiManager::onRealTimeAdjusted(float xDelta, float yDelta, int slowZoneDelta)
{
    qDebug() << "[AiManager] Real-time adjustment: X" << xDelta 
             << "Y" << yDelta << "SlowZone" << slowZoneDelta;
    // Emit signal for NeoController to apply
}

} // namespace NeoZ
