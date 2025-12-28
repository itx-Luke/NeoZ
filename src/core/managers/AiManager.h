#ifndef NEOZ_AIMANAGER_H
#define NEOZ_AIMANAGER_H

#include <QObject>
#include "../ai/AiAdvisor.h"
#include "../perf/RealTimeSensitivityAI.hpp"

namespace NeoZ {

/**
 * @brief Manages AI-powered sensitivity recommendations.
 * 
 * Extracted from NeoController. Handles:
 * - AI Advisor integration (Gemini API)
 * - Real-time sensitivity adjustments
 * - Recommendation gating by confidence threshold
 */
class AiManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(bool aiEnabled READ isAiEnabled WRITE setAiEnabled NOTIFY aiEnabledChanged)
    Q_PROPERTY(double aiConfidenceThreshold READ aiConfidenceThreshold WRITE setAiConfidenceThreshold NOTIFY aiEnabledChanged)
    Q_PROPERTY(QString lastRecommendationSummary READ lastRecommendationSummary NOTIFY recommendationChanged)
    Q_PROPERTY(double recommendedX READ recommendedX NOTIFY recommendationChanged)
    Q_PROPERTY(double recommendedY READ recommendedY NOTIFY recommendationChanged)
    Q_PROPERTY(double recommendationConfidence READ recommendationConfidence NOTIFY recommendationChanged)
    Q_PROPERTY(bool realTimeAiEnabled READ isRealTimeAiEnabled WRITE setRealTimeAiEnabled NOTIFY realTimeAiChanged)
    Q_PROPERTY(float hitRate READ hitRate NOTIFY realTimeMetricsChanged)
    Q_PROPERTY(float headshotRate READ headshotRate NOTIFY realTimeMetricsChanged)
    
public:
    explicit AiManager(QObject* parent = nullptr);
    ~AiManager();
    
    // AI Advisor control
    bool isAiEnabled() const { return m_aiEnabled; }
    void setAiEnabled(bool enabled);
    double aiConfidenceThreshold() const { return m_aiConfidenceThreshold; }
    void setAiConfidenceThreshold(double threshold);
    
    // Last recommendation
    QString lastRecommendationSummary() const { return m_lastRecommendationSummary; }
    double recommendedX() const { return m_recommendedX; }
    double recommendedY() const { return m_recommendedY; }
    double recommendationConfidence() const { return m_recommendationConfidence; }
    
    // AI Advisor access
    AiAdvisor* advisor() const { return m_advisor; }
    
    // Request recommendation
    Q_INVOKABLE void requestRecommendation(const SystemSnapshot& snapshot);
    
    // Real-time AI
    bool isRealTimeAiEnabled() const { return m_realTimeAi ? m_realTimeAi->isEnabled() : false; }
    void setRealTimeAiEnabled(bool enabled);
    RealTimeSensitivityAI* realTimeAi() const { return m_realTimeAi; }
    
    // Real-time metrics
    float hitRate() const { return m_realTimeAi ? m_realTimeAi->hitRate() : 0.0f; }
    float headshotRate() const { return m_realTimeAi ? m_realTimeAi->headshotRate() : 0.0f; }
    
    // Register shot result for real-time AI
    Q_INVOKABLE void registerShot(bool hit, bool headshot = false);
    
signals:
    void aiEnabledChanged();
    void recommendationChanged();
    void realTimeAiChanged();
    void realTimeMetricsChanged();
    void recommendationReady(double x, double y, double confidence);
    
private slots:
    void onRecommendationReady(const TuningRecommendation& rec);
    void onRealTimeAdjusted(float xDelta, float yDelta, int slowZoneDelta);
    
private:
    bool m_aiEnabled = false;
    double m_aiConfidenceThreshold = 0.7;
    
    // Last recommendation
    QString m_lastRecommendationSummary;
    double m_recommendedX = 0.0;
    double m_recommendedY = 0.0;
    double m_recommendationConfidence = 0.0;
    
    AiAdvisor* m_advisor = nullptr;
    RealTimeSensitivityAI* m_realTimeAi = nullptr;
};

} // namespace NeoZ

#endif // NEOZ_AIMANAGER_H
