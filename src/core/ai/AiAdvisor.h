#ifndef AIADVISOR_H
#define AIADVISOR_H

#include <QObject>
#include <QJsonObject>
#include <QDateTime>

// Forward declaration
class GeminiClient;

/**
 * @brief Sensitivity tuning recommendation from AI Advisor
 */
struct TuningRecommendation {
    double xMultiplier = 1.0;
    double yMultiplier = 1.0;
    QString curveId = "FF_OneTap_v2";
    int aimAssistSlowZone = 35;
    int smoothingMs = 20;
    QString severity = "low";      // "low", "medium", "high"
    double confidence = 0.0;       // 0.0 - 1.0
    QStringList reasoning;
    QDateTime timestamp;
    bool isValid = false;
};

/**
 * @brief System snapshot for analysis
 */
struct SystemSnapshot {
    // Host display
    int displayWidth = 0;
    int displayHeight = 0;
    int displayRefreshHz = 0;
    
    // Emulator state
    QString emulatorName;
    int wmWidth = 0;
    int wmHeight = 0;
    int wmDensity = 0;
    bool freeFireRunning = false;
    
    // Current sensitivity
    double xMultiplier = 1.0;
    double yMultiplier = 1.0;
    QString curveId;
    int slowZone = 0;
    int smoothingMs = 0;
    int mouseDpi = 800;
    
    // Computed metrics
    double pxPerCm = 0.0;
    double cmPer360 = 0.0;
    
    QJsonObject toJson() const;
};

/**
 * @brief Session analytics data
 */
struct SessionSummary {
    QString sessionId;
    int totalEvents = 0;
    double avgSpeedLow = 0.0;
    double avgSpeedHigh = 0.0;
    double overshootRateX = 0.0;
    double overshootRateY = 0.0;
    double microAdjustErrorX = 0.0;
    double microAdjustErrorY = 0.0;
    int redzoneEntryCount = 0;
    double avgTimeToStabilizeMs = 0.0;
    double fpsMean = 0.0;
    double fpsStdDev = 0.0;
    
    QJsonObject toJson() const;
};

/**
 * @brief AI Advisor for sensitivity recommendations
 * 
 * Uses GeminiClient for AI-powered recommendations with
 * fallback to heuristic-based calculations when offline.
 */
class AiAdvisor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY statusChanged)
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY statusChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit AiAdvisor(QObject *parent = nullptr);
    ~AiAdvisor();

    bool isOnline() const;
    bool isProcessing() const;
    QString status() const { return m_status; }

    /**
     * @brief Request sensitivity tuning recommendation
     * @param currentSnapshot Current system state
     * @param previousSnapshot Previous system state (for change detection)
     */
    void requestTuning(const SystemSnapshot& currentSnapshot, 
                       const SystemSnapshot* previousSnapshot = nullptr);

    /**
     * @brief Request full session analysis
     * @param snapshot Current system state
     * @param sessions List of recent session summaries
     */
    void analyzeSession(const SystemSnapshot& snapshot,
                        const QList<SessionSummary>& sessions);

    /**
     * @brief Get the last recommendation
     */
    TuningRecommendation lastRecommendation() const { return m_lastRecommendation; }

    /**
     * @brief Set API key for Gemini
     */
    void setApiKey(const QString& apiKey);

signals:
    void statusChanged();
    void recommendationReady(const TuningRecommendation& recommendation);
    void analysisError(const QString& error);

private slots:
    void onGeminiResponse(const QJsonObject& recommendation);
    void onGeminiError(const QString& error);

private:
    /**
     * @brief Heuristic-based recommendation (offline fallback)
     */
    TuningRecommendation computeHeuristicRecommendation(
        const SystemSnapshot& current,
        const SystemSnapshot* previous);

    /**
     * @brief Compute cm/360 based on system config
     */
    double computeCmPer360(const SystemSnapshot& snapshot);

    /**
     * @brief Apply safety limits to recommendation
     */
    void applySafetyLimits(TuningRecommendation& recommendation);

    GeminiClient* m_geminiClient;
    QString m_status;
    TuningRecommendation m_lastRecommendation;
    SystemSnapshot m_pendingSnapshot;
    bool m_useAi;
};

#endif // AIADVISOR_H
