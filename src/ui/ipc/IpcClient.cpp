#include "IpcClient.h"
#include <QDebug>
#include <QDateTime>

namespace NeoZ {

IpcClient::IpcClient(QObject* parent)
    : QObject(parent)
{
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    QObject::connect(m_reconnectTimer, &QTimer::timeout, this, &IpcClient::attemptReconnect);
}

IpcClient::~IpcClient()
{
    disconnect();
}

bool IpcClient::connect(const QString& endpoint)
{
    if (m_socket) {
        if (m_socket->state() == QLocalSocket::ConnectedState) {
            qDebug() << "[IpcClient] Already connected";
            return true;
        }
        m_socket->deleteLater();
    }
    
    m_endpoint = endpoint;
    m_socket = new QLocalSocket(this);
    
    QObject::connect(m_socket, &QLocalSocket::connected, this, &IpcClient::onConnected);
    QObject::connect(m_socket, &QLocalSocket::disconnected, this, &IpcClient::onDisconnected);
    QObject::connect(m_socket, &QLocalSocket::readyRead, this, &IpcClient::onReadyRead);
    QObject::connect(m_socket, &QLocalSocket::errorOccurred, this, &IpcClient::onError);
    
    m_socket->connectToServer(endpoint);
    
    if (!m_socket->waitForConnected(3000)) {
        qWarning() << "[IpcClient] Connection timeout to" << endpoint;
        if (m_autoReconnect) {
            m_reconnectTimer->start(m_reconnectInterval);
        }
        return false;
    }
    
    return true;
}

void IpcClient::disconnect()
{
    m_reconnectTimer->stop();
    
    if (m_socket) {
        m_socket->disconnectFromServer();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    // Cancel all pending requests
    for (auto& pending : m_pendingRequests) {
        if (pending.timer) {
            pending.timer->stop();
            pending.timer->deleteLater();
        }
    }
    m_pendingRequests.clear();
}

void IpcClient::onConnected()
{
    qDebug() << "[IpcClient] Connected to" << m_endpoint;
    m_reconnectTimer->stop();
    emit connected();
    emit connectionChanged();
}

void IpcClient::onDisconnected()
{
    qDebug() << "[IpcClient] Disconnected from" << m_endpoint;
    emit disconnected();
    emit connectionChanged();
    
    if (m_autoReconnect) {
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void IpcClient::onReadyRead()
{
    while (m_socket && m_socket->bytesAvailable() > 0) {
        QByteArray data = m_socket->readLine();
        if (!data.isEmpty()) {
            processMessage(data);
        }
    }
}

void IpcClient::onError(QLocalSocket::LocalSocketError socketError)
{
    Q_UNUSED(socketError)
    QString errorMsg = m_socket ? m_socket->errorString() : "Unknown error";
    qWarning() << "[IpcClient] Error:" << errorMsg;
    emit error(errorMsg);
}

void IpcClient::attemptReconnect()
{
    if (!isConnected()) {
        qDebug() << "[IpcClient] Attempting reconnect to" << m_endpoint;
        connect(m_endpoint);
    }
}

void IpcClient::processMessage(const QByteArray& data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[IpcClient] Invalid JSON:" << error.errorString();
        return;
    }
    
    QJsonObject msg = doc.object();
    QString type = msg["type"].toString();
    QString id = msg["id"].toString();
    
    // Check for pending request response
    if (!id.isEmpty() && m_pendingRequests.contains(id)) {
        PendingRequest& pending = m_pendingRequests[id];
        
        // Calculate latency
        m_latencyMs = QDateTime::currentMSecsSinceEpoch() - pending.startTime;
        emit latencyChanged();
        
        // Stop timeout timer
        if (pending.timer) {
            pending.timer->stop();
            pending.timer->deleteLater();
        }
        
        // Call callback
        if (pending.callback) {
            pending.callback(msg);
        }
        
        m_pendingRequests.remove(id);
        return;
    }
    
    // Check for type handler
    if (m_handlers.contains(type)) {
        m_handlers[type](msg);
        return;
    }
    
    // Emit generic signal
    emit messageReceived(msg);
}

void IpcClient::send(const QJsonObject& msg)
{
    if (!isConnected()) {
        qWarning() << "[IpcClient] Not connected, cannot send";
        return;
    }
    
    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n";
    m_socket->write(data);
    m_socket->flush();
}

void IpcClient::request(const QJsonObject& request, ResponseCallback callback, int timeoutMs)
{
    if (!isConnected()) {
        qWarning() << "[IpcClient] Not connected, cannot send request";
        if (callback) {
            QJsonObject errorResponse;
            errorResponse["error"] = "Not connected";
            callback(errorResponse);
        }
        return;
    }
    
    // Generate correlation ID
    QString id = generateId();
    
    // Add ID to request
    QJsonObject requestWithId = request;
    requestWithId["id"] = id;
    
    // Setup timeout timer
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, this, [this, id, callback]() {
        if (m_pendingRequests.contains(id)) {
            qWarning() << "[IpcClient] Request timeout:" << id;
            m_pendingRequests.remove(id);
            if (callback) {
                QJsonObject errorResponse;
                errorResponse["error"] = "Request timeout";
                callback(errorResponse);
            }
        }
    });
    
    // Store pending request
    PendingRequest pending;
    pending.callback = callback;
    pending.timer = timer;
    pending.startTime = QDateTime::currentMSecsSinceEpoch();
    m_pendingRequests[id] = pending;
    
    // Start timeout
    timer->start(timeoutMs);
    
    // Send request
    send(requestWithId);
}

void IpcClient::on(const QString& type, MessageHandler handler)
{
    m_handlers[type] = std::move(handler);
}

void IpcClient::setAutoReconnect(bool enabled, int intervalMs)
{
    m_autoReconnect = enabled;
    m_reconnectInterval = intervalMs;
    
    if (!enabled) {
        m_reconnectTimer->stop();
    }
}

QString IpcClient::generateId()
{
    return QString::number(++m_idCounter);
}

} // namespace NeoZ
