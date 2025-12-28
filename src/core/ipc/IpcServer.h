#ifndef NEOZ_IPCSERVER_H
#define NEOZ_IPCSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <functional>

namespace NeoZ {

/**
 * @brief IPC Server using Qt Local Server for Core ↔ UI communication.
 * 
 * Features:
 * - Zero dependencies (uses Qt's QLocalServer)
 * - Secure (local socket, no network exposure)  
 * - Low latency (no TCP handshake overhead)
 * - Cross-platform (works on Windows, Linux, Mac)
 * 
 * Protocol:
 * - Messages are JSON objects with "type" field
 * - Each message has optional "id" for correlation
 * - Server broadcasts events to all connected clients
 */
class IpcServer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool listening READ isListening NOTIFY listeningChanged)
    Q_PROPERTY(int clientCount READ clientCount NOTIFY clientCountChanged)
    
public:
    explicit IpcServer(QObject* parent = nullptr);
    ~IpcServer();
    
    /**
     * @brief Start listening on the specified endpoint
     * @param endpoint Server name (default: "NeoZCore")
     * @return true if server started successfully
     */
    bool initialize(const QString& endpoint = "NeoZCore");
    
    /**
     * @brief Stop the server
     */
    void shutdown();
    
    /**
     * @brief Check if server is listening
     */
    bool isListening() const { return m_server && m_server->isListening(); }
    
    /**
     * @brief Get number of connected clients
     */
    int clientCount() const { return m_clients.size(); }
    
    /**
     * @brief Send message to all connected clients
     * @param msg JSON message to broadcast
     */
    void broadcast(const QJsonObject& msg);
    
    /**
     * @brief Send message to a specific client
     * @param clientId Client identifier
     * @param msg JSON message to send
     */
    void sendTo(qintptr clientId, const QJsonObject& msg);
    
    /**
     * @brief Register a message handler for a specific message type
     * @param type Message type (e.g., "GetCurrentParams")
     * @param handler Function to call when message is received
     */
    using MessageHandler = std::function<QJsonObject(const QJsonObject& request)>;
    void registerHandler(const QString& type, MessageHandler handler);
    
    /**
     * @brief Send a response to a request (includes correlation id)
     * @param request Original request message
     * @param response Response data
     */
    void respond(qintptr clientId, const QJsonObject& request, const QJsonObject& response);
    
signals:
    void listeningChanged();
    void clientCountChanged();
    void clientConnected(qintptr clientId);
    void clientDisconnected(qintptr clientId);
    void messageReceived(qintptr clientId, const QJsonObject& message);
    void error(const QString& errorMessage);
    
private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();
    
private:
    void processMessage(qintptr clientId, const QByteArray& data);
    
    QLocalServer* m_server = nullptr;
    QHash<qintptr, QLocalSocket*> m_clients;
    QHash<QString, MessageHandler> m_handlers;
    QString m_endpoint;
};

} // namespace NeoZ

#endif // NEOZ_IPCSERVER_H
