#include "IpcServer.h"
#include <QDebug>
#include <QDataStream>

namespace NeoZ {

IpcServer::IpcServer(QObject* parent)
    : QObject(parent)
{
}

IpcServer::~IpcServer()
{
    shutdown();
}

bool IpcServer::initialize(const QString& endpoint)
{
    if (m_server) {
        qWarning() << "[IpcServer] Already initialized";
        return false;
    }
    
    m_endpoint = endpoint;
    m_server = new QLocalServer(this);
    
    // Remove any stale server from previous crash
    QLocalServer::removeServer(endpoint);
    
    if (!m_server->listen(endpoint)) {
        qCritical() << "[IpcServer] Failed to listen on" << endpoint 
                    << ":" << m_server->errorString();
        emit error(m_server->errorString());
        delete m_server;
        m_server = nullptr;
        return false;
    }
    
    connect(m_server, &QLocalServer::newConnection, this, &IpcServer::onNewConnection);
    
    qDebug() << "[IpcServer] Listening on:" << endpoint;
    emit listeningChanged();
    return true;
}

void IpcServer::shutdown()
{
    if (m_server) {
        // Disconnect all clients
        for (auto* client : m_clients) {
            client->disconnectFromServer();
            client->deleteLater();
        }
        m_clients.clear();
        
        m_server->close();
        delete m_server;
        m_server = nullptr;
        
        qDebug() << "[IpcServer] Shutdown complete";
        emit listeningChanged();
        emit clientCountChanged();
    }
}

void IpcServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket* client = m_server->nextPendingConnection();
        qintptr clientId = client->socketDescriptor();
        
        m_clients[clientId] = client;
        
        connect(client, &QLocalSocket::disconnected, this, &IpcServer::onClientDisconnected);
        connect(client, &QLocalSocket::readyRead, this, &IpcServer::onReadyRead);
        
        qDebug() << "[IpcServer] Client connected:" << clientId;
        emit clientConnected(clientId);
        emit clientCountChanged();
        
        // Send welcome message
        QJsonObject welcome;
        welcome["type"] = "Welcome";
        welcome["version"] = "1.0";
        sendTo(clientId, welcome);
    }
}

void IpcServer::onClientDisconnected()
{
    QLocalSocket* client = qobject_cast<QLocalSocket*>(sender());
    if (!client) return;
    
    qintptr clientId = 0;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value() == client) {
            clientId = it.key();
            m_clients.erase(it);
            break;
        }
    }
    
    client->deleteLater();
    
    qDebug() << "[IpcServer] Client disconnected:" << clientId;
    emit clientDisconnected(clientId);
    emit clientCountChanged();
}

void IpcServer::onReadyRead()
{
    QLocalSocket* client = qobject_cast<QLocalSocket*>(sender());
    if (!client) return;
    
    qintptr clientId = 0;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value() == client) {
            clientId = it.key();
            break;
        }
    }
    
    // Read all available data
    while (client->bytesAvailable() > 0) {
        QByteArray data = client->readLine();
        if (!data.isEmpty()) {
            processMessage(clientId, data);
        }
    }
}

void IpcServer::processMessage(qintptr clientId, const QByteArray& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[IpcServer] Invalid JSON from client" << clientId 
                   << ":" << error.errorString();
        return;
    }
    
    QJsonObject msg = doc.object();
    QString type = msg["type"].toString();
    
    qDebug() << "[IpcServer] Received from" << clientId << ":" << type;
    emit messageReceived(clientId, msg);
    
    // Check for registered handler
    if (m_handlers.contains(type)) {
        QJsonObject response = m_handlers[type](msg);
        if (!response.isEmpty()) {
            respond(clientId, msg, response);
        }
    }
}

void IpcServer::broadcast(const QJsonObject& msg)
{
    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n";
    
    for (auto* client : m_clients) {
        if (client->state() == QLocalSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}

void IpcServer::sendTo(qintptr clientId, const QJsonObject& msg)
{
    if (!m_clients.contains(clientId)) {
        qWarning() << "[IpcServer] Unknown client:" << clientId;
        return;
    }
    
    QLocalSocket* client = m_clients[clientId];
    if (client->state() == QLocalSocket::ConnectedState) {
        QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n";
        client->write(data);
        client->flush();
    }
}

void IpcServer::registerHandler(const QString& type, MessageHandler handler)
{
    m_handlers[type] = std::move(handler);
    qDebug() << "[IpcServer] Registered handler for:" << type;
}

void IpcServer::respond(qintptr clientId, const QJsonObject& request, const QJsonObject& response)
{
    QJsonObject reply = response;
    reply["type"] = request["type"].toString() + "Response";
    
    // Copy correlation ID if present
    if (request.contains("id")) {
        reply["id"] = request["id"];
    }
    
    sendTo(clientId, reply);
}

} // namespace NeoZ
