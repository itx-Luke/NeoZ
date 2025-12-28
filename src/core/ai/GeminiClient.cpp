#include "GeminiClient.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QNetworkRequest>
#include <QUrl>

// Gemini API configuration
const QString GeminiClient::API_BASE_URL = "https://generativelanguage.googleapis.com/v1beta/models/";
const QString GeminiClient::MODEL_NAME = "gemini-1.5-flash";

GeminiClient::GeminiClient(QObject *parent)
    : QObject(parent),
      m_networkManager(new QNetworkAccessManager(this)),
      m_isProcessing(false),
      m_isSensitivityRequest(false)
{
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &GeminiClient::onNetworkReply);
    
    loadApiKey();
    
    if (m_apiKey.isEmpty()) {
        qWarning() << "[GeminiClient] No API key found. Set GEMINI_API_KEY environment variable.";
    } else {
        qDebug() << "[GeminiClient] API key loaded successfully";
    }
}

GeminiClient::~GeminiClient()
{
}

void GeminiClient::loadApiKey()
{
    // Try environment variable first
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    m_apiKey = env.value("GEMINI_API_KEY", "");
    
    if (!m_apiKey.isEmpty()) {
        qDebug() << "[GeminiClient] API key loaded from environment variable";
        emit readyChanged();
        return;
    }
    
    // Try config file as fallback
    QString configPath = QDir::homePath() + "/.neoz/config.json";
    QFile configFile(configPath);
    
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
        configFile.close();
        
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            m_apiKey = obj.value("gemini_api_key").toString();
            if (!m_apiKey.isEmpty()) {
                qDebug() << "[GeminiClient] API key loaded from config file";
                emit readyChanged();
            }
        }
    }
}

void GeminiClient::setApiKey(const QString& apiKey)
{
    if (m_apiKey != apiKey) {
        m_apiKey = apiKey;
        emit readyChanged();
        
        // Save to config file for persistence
        QString configDir = QDir::homePath() + "/.neoz";
        QDir().mkpath(configDir);
        
        QString configPath = configDir + "/config.json";
        QFile configFile(configPath);
        
        QJsonObject config;
        
        // Load existing config if present
        if (configFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
            if (doc.isObject()) {
                config = doc.object();
            }
            configFile.close();
        }
        
        // Update API key
        config["gemini_api_key"] = apiKey;
        
        // Save config
        if (configFile.open(QIODevice::WriteOnly)) {
            configFile.write(QJsonDocument(config).toJson());
            configFile.close();
            qDebug() << "[GeminiClient] API key saved to config file";
        }
    }
}

void GeminiClient::sendPrompt(const QString& prompt, const QString& systemInstruction)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API key not configured. Please set GEMINI_API_KEY environment variable.");
        return;
    }
    
    if (m_isProcessing) {
        emit errorOccurred("Request already in progress. Please wait.");
        return;
    }
    
    m_isProcessing = true;
    m_isSensitivityRequest = false;
    emit processingChanged();
    
    QString url = API_BASE_URL + MODEL_NAME + ":generateContent?key=" + m_apiKey;
    
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString body = buildRequestBody(prompt, systemInstruction);
    
    qDebug() << "[GeminiClient] Sending request to Gemini API...";
    m_networkManager->post(request, body.toUtf8());
}

void GeminiClient::analyzeSensitivity(const QJsonObject& systemConfig, const QJsonObject& sessionData)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API key not configured.");
        return;
    }
    
    if (m_isProcessing) {
        emit errorOccurred("Request already in progress.");
        return;
    }
    
    m_isProcessing = true;
    m_isSensitivityRequest = true;
    emit processingChanged();
    
    // Build prompt for sensitivity analysis
    QString systemInstruction = R"(
You are Neo-Z AI Advisor, an expert in mouse sensitivity optimization for Free Fire on Android emulators.
Your task is to analyze system configuration and gameplay data to recommend optimal sensitivity settings.

ALWAYS respond with a valid JSON object containing:
{
    "xMultiplier": <number between 0.5 and 3.0>,
    "yMultiplier": <number between 0.5 and 3.0>,
    "curveId": "<string: one of FF_Linear, FF_OneTap_v1, FF_OneTap_v2, FF_Precision, FF_Aggressive>",
    "aimAssistSlowZone": <integer 0-50>,
    "smoothingMs": <integer 0-200>,
    "severity": "<string: low, medium, or high>",
    "confidence": <number 0.0 to 1.0>,
    "reasoning": ["<string explaining reason 1>", "<string explaining reason 2>", ...]
}

Consider these factors:
- Resolution affects px/cm (higher res = more precision needed)
- FPS affects smoothing (lower FPS = more smoothing helps)
- Emulator DPI affects in-game sensitivity mapping
- Overshoot rate indicates sens too high, undershoot indicates too low
- Red-zone stabilization time should be 60-100ms optimal
)";

    QJsonObject combinedData;
    combinedData["systemConfig"] = systemConfig;
    combinedData["sessionData"] = sessionData;
    
    QString prompt = "Analyze this system configuration and session data, then provide sensitivity recommendations:\n\n" 
                   + QString::fromUtf8(QJsonDocument(combinedData).toJson(QJsonDocument::Indented));
    
    QString url = API_BASE_URL + MODEL_NAME + ":generateContent?key=" + m_apiKey;
    
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString body = buildRequestBody(prompt, systemInstruction);
    
    qDebug() << "[GeminiClient] Sending sensitivity analysis request...";
    m_networkManager->post(request, body.toUtf8());
}

QString GeminiClient::buildRequestBody(const QString& prompt, const QString& systemInstruction)
{
    QJsonObject requestBody;
    
    // Add system instruction if provided
    if (!systemInstruction.isEmpty()) {
        QJsonObject systemInstructionObj;
        QJsonArray systemParts;
        QJsonObject systemTextPart;
        systemTextPart["text"] = systemInstruction;
        systemParts.append(systemTextPart);
        systemInstructionObj["parts"] = systemParts;
        requestBody["system_instruction"] = systemInstructionObj;
    }
    
    // Add user content
    QJsonArray contents;
    QJsonObject content;
    QJsonArray parts;
    QJsonObject textPart;
    textPart["text"] = prompt;
    parts.append(textPart);
    content["parts"] = parts;
    contents.append(content);
    requestBody["contents"] = contents;
    
    // Generation config for better JSON output
    QJsonObject generationConfig;
    generationConfig["temperature"] = 0.7;
    generationConfig["maxOutputTokens"] = 1024;
    requestBody["generationConfig"] = generationConfig;
    
    return QString::fromUtf8(QJsonDocument(requestBody).toJson());
}

void GeminiClient::onNetworkReply(QNetworkReply* reply)
{
    m_isProcessing = false;
    emit processingChanged();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("Network error: %1").arg(reply->errorString());
        qWarning() << "[GeminiClient]" << errorMsg;
        emit errorOccurred(errorMsg);
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    
    if (m_isSensitivityRequest) {
        parseSensitivityResponse(data);
    } else {
        parseResponse(data);
    }
    
    reply->deleteLater();
}

void GeminiClient::parseResponse(const QByteArray& data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        emit errorOccurred("Invalid JSON response from API");
        return;
    }
    
    QJsonObject root = doc.object();
    
    // Check for API error
    if (root.contains("error")) {
        QJsonObject error = root["error"].toObject();
        QString errorMsg = error["message"].toString();
        qWarning() << "[GeminiClient] API Error:" << errorMsg;
        emit errorOccurred(errorMsg);
        return;
    }
    
    // Extract text from response
    QJsonArray candidates = root["candidates"].toArray();
    if (candidates.isEmpty()) {
        emit errorOccurred("No response candidates from API");
        return;
    }
    
    QJsonObject firstCandidate = candidates[0].toObject();
    QJsonObject content = firstCandidate["content"].toObject();
    QJsonArray parts = content["parts"].toArray();
    
    if (parts.isEmpty()) {
        emit errorOccurred("Empty response from API");
        return;
    }
    
    QString responseText = parts[0].toObject()["text"].toString();
    qDebug() << "[GeminiClient] Response received:" << responseText.left(100) << "...";
    
    emit responseReceived(responseText);
}

void GeminiClient::parseSensitivityResponse(const QByteArray& data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        emit errorOccurred("Invalid JSON response from API");
        return;
    }
    
    QJsonObject root = doc.object();
    
    // Check for API error
    if (root.contains("error")) {
        QJsonObject error = root["error"].toObject();
        emit errorOccurred(error["message"].toString());
        return;
    }
    
    // Extract text from response
    QJsonArray candidates = root["candidates"].toArray();
    if (candidates.isEmpty()) {
        emit errorOccurred("No response candidates from API");
        return;
    }
    
    QJsonObject firstCandidate = candidates[0].toObject();
    QJsonObject content = firstCandidate["content"].toObject();
    QJsonArray parts = content["parts"].toArray();
    
    if (parts.isEmpty()) {
        emit errorOccurred("Empty response from API");
        return;
    }
    
    QString responseText = parts[0].toObject()["text"].toString();
    
    // Try to parse the JSON from the response
    // Handle case where response might have markdown code blocks
    QString jsonText = responseText;
    if (jsonText.contains("```json")) {
        int start = jsonText.indexOf("```json") + 7;
        int end = jsonText.indexOf("```", start);
        if (end > start) {
            jsonText = jsonText.mid(start, end - start).trimmed();
        }
    } else if (jsonText.contains("```")) {
        int start = jsonText.indexOf("```") + 3;
        int end = jsonText.indexOf("```", start);
        if (end > start) {
            jsonText = jsonText.mid(start, end - start).trimmed();
        }
    }
    
    QJsonDocument recommendationDoc = QJsonDocument::fromJson(jsonText.toUtf8());
    
    if (!recommendationDoc.isObject()) {
        qWarning() << "[GeminiClient] Failed to parse recommendation JSON, using raw text";
        // Create a minimal response
        QJsonObject fallback;
        fallback["error"] = "Failed to parse AI response";
        fallback["rawResponse"] = responseText;
        emit sensitivityAnalysisComplete(fallback);
        return;
    }
    
    QJsonObject recommendation = recommendationDoc.object();
    qDebug() << "[GeminiClient] Sensitivity analysis complete";
    emit sensitivityAnalysisComplete(recommendation);
}
