#ifndef NEOZ_IPCCLIENT_H
#define NEOZ_IPCCLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QTimer>
#include <functional>

namespace NeoZ {

/**
 * @brief IPC Client using Qt Local Socket for UI → Core communication.
 * 
 * Features:
 * - Auto-reconnection on disconnect
 * - Request/response correlation via message ID
 * - Async and sync message patterns
 * - Connection state management
 */
class IpcClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(int latencyMs READ latencyMs NOTIFY latencyChanged)
    
public:
    explicit IpcClient(QObject* parent = nullptr);
    ~IpcClient();
    
    /**
     * @brief Connect to the Core server
     * @param endpoint Server name (default: "NeoZCore")
     * @return true if connection initiated
     */
    bool connect(const QString& endpoint = "NeoZCore");
    
    /**
     * @brief Disconnect from server
     */
    void disconnect();
    
    /**
     * @brief Check if connected
     */
    bool isConnected() const { return m_socket && m_socket->state() == QLocalSocket::ConnectedState; }
    
    /**
     * @brief Get last measured round-trip latency
     */
    int latencyMs() const { return m_latencyMs; }
    
    /**
     * @brief Send a message (fire and forget)
     * @param msg JSON message to send
     */
    void send(const QJsonObject& msg);
    
    /**
     * @brief Send a request and call callback with response
     * @param request Request message (must have "type")
     * @param callback Function called with response
     * @param timeoutMs Timeout in milliseconds
     */
    using ResponseCallback = std::function<void(const QJsonObject& response)>;
    void request(const QJsonObject& request, ResponseCallback callback, int timeoutMs = 5000);
    
    /**
     * @brief Register a handler for incoming messages of a type
     * @param type Message type to handle
     * @param handler Handler function
     */
    using MessageHandler = std::function<void(const QJsonObject& msg)>;
    void on(const QString& type, MessageHandler handler);
    
    /**
     * @brief Enable auto-reconnection
     * @param enabled Whether to auto-reconnect
     * @param intervalMs Reconnection interval
     */
    void setAutoReconnect(bool enabled, int intervalMs = 2000);
    
signals:
    void connectionChanged();
    void latencyChanged();
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject& message);
    void error(const QString& errorMessage);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QLocalSocket::LocalSocketError socketError);
    void attemptReconnect();
    
private:
    void processMessage(const QByteArray& data);
    QString generateId();
    
    QLocalSocket* m_socket = nullptr;
    QString m_endpoint;
    int m_latencyMs = 0;
    
    // Pending requests awaiting response
    struct PendingRequest {
        ResponseCallback callback;
        QTimer* timer;
        qint64 startTime;
    };
    QHash<QString, PendingRequest> m_pendingRequests;
    
    // Message type handlers
    QHash<QString, MessageHandler> m_handlers;
    
    // Auto-reconnect
    QTimer* m_reconnectTimer = nullptr;
    bool m_autoReconnect = true;
    int m_reconnectInterval = 2000;
    
    // ID generation
    quint64 m_idCounter = 0;
};

} // namespace NeoZ

#endif // NEOZ_IPCCLIENT_H
