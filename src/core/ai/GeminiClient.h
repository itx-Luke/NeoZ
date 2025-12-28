#ifndef GEMINICLIENT_H
#define GEMINICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief HTTP client for Google Gemini API communication
 * 
 * Handles async requests to the Gemini generative AI API.
 * API key is loaded from environment variable GEMINI_API_KEY
 * or from config file ~/.neoz/config.json
 */
class GeminiClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isReady READ isReady NOTIFY readyChanged)
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY processingChanged)

public:
    explicit GeminiClient(QObject *parent = nullptr);
    ~GeminiClient();

    /**
     * @brief Check if API key is configured and client is ready
     */
    bool isReady() const { return !m_apiKey.isEmpty(); }
    
    /**
     * @brief Check if a request is currently in progress
     */
    bool isProcessing() const { return m_isProcessing; }

    /**
     * @brief Set API key manually (useful for settings UI)
     */
    void setApiKey(const QString& apiKey);

    /**
     * @brief Send a prompt to Gemini API
     * @param prompt The text prompt to send
     * @param systemInstruction Optional system instruction for context
     */
    Q_INVOKABLE void sendPrompt(const QString& prompt, const QString& systemInstruction = QString());

    /**
     * @brief Send structured data for sensitivity analysis
     * @param systemConfig JSON object with system configuration
     * @param sessionData JSON object with session analytics
     */
    void analyzeSensitivity(const QJsonObject& systemConfig, const QJsonObject& sessionData);

signals:
    void readyChanged();
    void processingChanged();
    void responseReceived(const QString& response);
    void errorOccurred(const QString& error);
    void sensitivityAnalysisComplete(const QJsonObject& recommendation);

private slots:
    void onNetworkReply(QNetworkReply* reply);

private:
    void loadApiKey();
    QString buildRequestBody(const QString& prompt, const QString& systemInstruction);
    void parseResponse(const QByteArray& data);
    void parseSensitivityResponse(const QByteArray& data);

    QNetworkAccessManager* m_networkManager;
    QString m_apiKey;
    bool m_isProcessing;
    bool m_isSensitivityRequest;
    
    // Gemini API endpoint
    static const QString API_BASE_URL;
    static const QString MODEL_NAME;
};

#endif // GEMINICLIENT_H
