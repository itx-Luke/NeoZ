#include "AiAdvisor.h"
#include "GeminiClient.h"
#include <QDebug>
#include <QUuid>
#include <cmath>

// ============================================
// SystemSnapshot implementation
// ============================================

QJsonObject SystemSnapshot::toJson() const
{
    QJsonObject obj;
    
    // Host display
    QJsonObject display;
    display["width"] = displayWidth;
    display["height"] = displayHeight;
    display["refreshHz"] = displayRefreshHz;
    obj["display"] = display;
    
    // Emulator state
    QJsonObject emulator;
    emulator["name"] = emulatorName;
    emulator["wmWidth"] = wmWidth;
    emulator["wmHeight"] = wmHeight;
    emulator["wmDensity"] = wmDensity;
    emulator["freeFireRunning"] = freeFireRunning;
    obj["emulator"] = emulator;
    
    // Current sensitivity
    QJsonObject sens;
    sens["xMultiplier"] = xMultiplier;
    sens["yMultiplier"] = yMultiplier;
    sens["curveId"] = curveId;
    sens["slowZone"] = slowZone;
    sens["smoothingMs"] = smoothingMs;
    sens["mouseDpi"] = mouseDpi;
    obj["sensitivity"] = sens;
    
    // Derived metrics
    QJsonObject metrics;
    metrics["pxPerCm"] = pxPerCm;
    metrics["cmPer360"] = cmPer360;
    obj["metrics"] = metrics;
    
    return obj;
}

// ============================================
// SessionSummary implementation
// ============================================

QJsonObject SessionSummary::toJson() const
{
    QJsonObject obj;
    obj["sessionId"] = sessionId;
    obj["totalEvents"] = totalEvents;
    obj["avgSpeedLow"] = avgSpeedLow;
    obj["avgSpeedHigh"] = avgSpeedHigh;
    obj["overshootRateX"] = overshootRateX;
    obj["overshootRateY"] = overshootRateY;
    obj["microAdjustErrorX"] = microAdjustErrorX;
    obj["microAdjustErrorY"] = microAdjustErrorY;
    obj["redzoneEntryCount"] = redzoneEntryCount;
    obj["avgTimeToStabilizeMs"] = avgTimeToStabilizeMs;
    obj["fpsMean"] = fpsMean;
    obj["fpsStdDev"] = fpsStdDev;
    return obj;
}

// ============================================
// AiAdvisor implementation
// ============================================

AiAdvisor::AiAdvisor(QObject *parent)
    : QObject(parent),
      m_geminiClient(new GeminiClient(this)),
      m_status("Initializing..."),
      m_useAi(true)
{
    connect(m_geminiClient, &GeminiClient::sensitivityAnalysisComplete,
            this, &AiAdvisor::onGeminiResponse);
    connect(m_geminiClient, &GeminiClient::errorOccurred,
            this, &AiAdvisor::onGeminiError);
    connect(m_geminiClient, &GeminiClient::readyChanged,
            this, &AiAdvisor::statusChanged);
    connect(m_geminiClient, &GeminiClient::processingChanged,
            this, &AiAdvisor::statusChanged);
    
    // Update status based on API key availability
    if (m_geminiClient->isReady()) {
        m_status = "Online";
    } else {
        m_status = "Offline (No API Key)";
    }
    emit statusChanged();
    
    qDebug() << "[AiAdvisor] Initialized, status:" << m_status;
}

AiAdvisor::~AiAdvisor()
{
}

bool AiAdvisor::isOnline() const
{
    return m_geminiClient->isReady();
}

bool AiAdvisor::isProcessing() const
{
    return m_geminiClient->isProcessing();
}

void AiAdvisor::setApiKey(const QString& apiKey)
{
    m_geminiClient->setApiKey(apiKey);
    if (m_geminiClient->isReady()) {
        m_status = "Online";
    } else {
        m_status = "Offline (No API Key)";
    }
    emit statusChanged();
}

void AiAdvisor::requestTuning(const SystemSnapshot& currentSnapshot,
                               const SystemSnapshot* previousSnapshot)
{
    m_pendingSnapshot = currentSnapshot;
    
    // If AI is not available, use heuristics
    if (!m_geminiClient->isReady() || !m_useAi) {
        qDebug() << "[AiAdvisor] Using heuristic recommendation (offline mode)";
        m_status = "Offline (Heuristic)";
        emit statusChanged();
        
        TuningRecommendation recommendation = computeHeuristicRecommendation(
            currentSnapshot, previousSnapshot);
        
        m_lastRecommendation = recommendation;
        emit recommendationReady(recommendation);
        
        m_status = isOnline() ? "Online" : "Offline";
        emit statusChanged();
        return;
    }
    
    // Build session data for AI
    QJsonObject systemConfig = currentSnapshot.toJson();
    
    QJsonObject sessionData;
    if (previousSnapshot) {
        sessionData["previousSnapshot"] = previousSnapshot->toJson();
        
        // Compute change metrics
        double cmPer360Before = computeCmPer360(*previousSnapshot);
        double cmPer360After = computeCmPer360(currentSnapshot);
        double driftPercent = ((cmPer360After - cmPer360Before) / cmPer360Before) * 100.0;
        
        sessionData["cmPer360DriftPercent"] = driftPercent;
        sessionData["resolutionChanged"] = (previousSnapshot->displayWidth != currentSnapshot.displayWidth ||
                                            previousSnapshot->displayHeight != currentSnapshot.displayHeight);
    }
    
    m_status = "Analyzing...";
    emit statusChanged();
    
    qDebug() << "[AiAdvisor] Requesting AI analysis...";
    m_geminiClient->analyzeSensitivity(systemConfig, sessionData);
}

void AiAdvisor::analyzeSession(const SystemSnapshot& snapshot,
                                const QList<SessionSummary>& sessions)
{
    m_pendingSnapshot = snapshot;
    
    QJsonObject systemConfig = snapshot.toJson();
    
    QJsonObject sessionData;
    QJsonArray sessionsArray;
    for (const auto& session : sessions) {
        sessionsArray.append(session.toJson());
    }
    sessionData["sessions"] = sessionsArray;
    
    if (!m_geminiClient->isReady()) {
        m_status = "Offline (No API Key)";
        emit statusChanged();
        emit analysisError("AI analysis requires API key. Using defaults.");
        return;
    }
    
    m_status = "Analyzing Session...";
    emit statusChanged();
    
    qDebug() << "[AiAdvisor] Requesting session analysis with" << sessions.size() << "sessions";
    m_geminiClient->analyzeSensitivity(systemConfig, sessionData);
}

void AiAdvisor::onGeminiResponse(const QJsonObject& recommendation)
{
    qDebug() << "[AiAdvisor] Received AI recommendation";
    
    TuningRecommendation rec;
    rec.isValid = true;
    rec.timestamp = QDateTime::currentDateTime();
    
    // Parse recommendation JSON
    if (recommendation.contains("error")) {
        // AI response parsing failed, use heuristics
        qWarning() << "[AiAdvisor] AI response error, falling back to heuristics";
        rec = computeHeuristicRecommendation(m_pendingSnapshot, nullptr);
        rec.reasoning.append("AI response could not be parsed, using heuristic calculation.");
    } else {
        rec.xMultiplier = recommendation.value("xMultiplier").toDouble(m_pendingSnapshot.xMultiplier);
        rec.yMultiplier = recommendation.value("yMultiplier").toDouble(m_pendingSnapshot.yMultiplier);
        rec.curveId = recommendation.value("curveId").toString(m_pendingSnapshot.curveId);
        rec.aimAssistSlowZone = recommendation.value("aimAssistSlowZone").toInt(m_pendingSnapshot.slowZone);
        rec.smoothingMs = recommendation.value("smoothingMs").toInt(m_pendingSnapshot.smoothingMs);
        rec.severity = recommendation.value("severity").toString("medium");
        rec.confidence = recommendation.value("confidence").toDouble(0.7);
        
        QJsonArray reasoningArray = recommendation.value("reasoning").toArray();
        for (const auto& reason : reasoningArray) {
            rec.reasoning.append(reason.toString());
        }
    }
    
    // Apply safety limits
    applySafetyLimits(rec);
    
    m_lastRecommendation = rec;
    m_status = "Online";
    emit statusChanged();
    emit recommendationReady(rec);
}

void AiAdvisor::onGeminiError(const QString& error)
{
    qWarning() << "[AiAdvisor] Gemini error:" << error;
    
    // Fall back to heuristics
    TuningRecommendation rec = computeHeuristicRecommendation(m_pendingSnapshot, nullptr);
    rec.reasoning.prepend(QString("AI unavailable (%1), using heuristics.").arg(error));
    
    m_lastRecommendation = rec;
    m_status = isOnline() ? "Online" : "Offline";
    emit statusChanged();
    emit recommendationReady(rec);
    emit analysisError(error);
}

TuningRecommendation AiAdvisor::computeHeuristicRecommendation(
    const SystemSnapshot& current,
    const SystemSnapshot* previous)
{
    TuningRecommendation rec;
    rec.isValid = true;
    rec.timestamp = QDateTime::currentDateTime();
    rec.confidence = 0.6;  // Lower confidence for heuristics
    
    // Start with current values
    rec.xMultiplier = current.xMultiplier;
    rec.yMultiplier = current.yMultiplier;
    rec.curveId = current.curveId;
    rec.aimAssistSlowZone = current.slowZone;
    rec.smoothingMs = current.smoothingMs;
    rec.severity = "low";
    
    // If we have a previous snapshot, compute compensation for resolution changes
    if (previous) {
        double cmPer360Before = computeCmPer360(*previous);
        double cmPer360After = computeCmPer360(current);
        
        if (cmPer360Before > 0 && cmPer360After > 0) {
            double scaleFactor = cmPer360Before / cmPer360After;
            
            // Only adjust if change is significant (> 5%)
            if (std::abs(scaleFactor - 1.0) > 0.05) {
                rec.xMultiplier = current.xMultiplier * scaleFactor;
                rec.yMultiplier = current.yMultiplier * scaleFactor;
                
                double driftPercent = (scaleFactor - 1.0) * 100.0;
                rec.severity = std::abs(driftPercent) > 15 ? "high" : "medium";
                
                rec.reasoning.append(QString("Resolution change caused %.1f%% sensitivity drift.")
                    .arg(driftPercent));
                rec.reasoning.append(QString("Scaling multipliers by %.2f to restore original feel.")
                    .arg(scaleFactor));
            }
        }
    }
    
    // Apply safety limits
    applySafetyLimits(rec);
    
    if (rec.reasoning.isEmpty()) {
        rec.reasoning.append("Current settings appear optimal. No changes recommended.");
    }
    
    return rec;
}

double AiAdvisor::computeCmPer360(const SystemSnapshot& snapshot)
{
    // Basic cm/360 calculation based on:
    // - Physical DPI of mouse
    // - Multipliers applied
    // - Display resolution
    
    if (snapshot.mouseDpi <= 0) {
        return 0.0;
    }
    
    // Effective DPI = mouse DPI * average multiplier
    double avgMultiplier = (snapshot.xMultiplier + snapshot.yMultiplier) / 2.0;
    double effectiveDpi = snapshot.mouseDpi * avgMultiplier;
    
    // Counts per 360 (assuming standard Free Fire sensitivity)
    // This is an approximation - actual value depends on in-game settings
    double countsFor360 = 16000;  // Typical value for mid-sensitivity
    
    // cm for 360 = counts / (DPI * multiplier) * 2.54
    double cmPer360 = (countsFor360 / effectiveDpi) * 2.54;
    
    return cmPer360;
}

void AiAdvisor::applySafetyLimits(TuningRecommendation& rec)
{
    // Clamp values to safe ranges
    rec.xMultiplier = std::clamp(rec.xMultiplier, 0.1, 4.0);
    rec.yMultiplier = std::clamp(rec.yMultiplier, 0.1, 4.0);
    rec.aimAssistSlowZone = std::clamp(rec.aimAssistSlowZone, 0, 50);
    rec.smoothingMs = std::clamp(rec.smoothingMs, 0, 200);
    
    // Check for extreme changes
    // (We don't have original values here, but we can flag extreme absolute values)
    if (rec.xMultiplier > 3.0 || rec.yMultiplier > 3.0) {
        rec.severity = "high";
        rec.reasoning.append("Warning: Very high multiplier values. Consider review.");
    }
    
    if (rec.xMultiplier < 0.3 || rec.yMultiplier < 0.3) {
        rec.severity = "high";
        rec.reasoning.append("Warning: Very low multiplier values. Consider review.");
    }
}
