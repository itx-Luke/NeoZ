#include "AdbService.h"
#include <QDebug>
#include <QJsonArray>
#include <QProcess>

namespace NeoZ {

AdbService::AdbService(QObject* parent)
    : QObject(parent)
{
    m_adbPath = "adb"; // Default to PATH
}

AdbService::~AdbService()
{
    stop();
    
    // Cleanup device connections
    qDeleteAll(m_deviceConnections);
    m_deviceConnections.clear();
}

bool AdbService::start(quint16 port)
{
    if (m_server) {
        qWarning() << "[AdbService] Already started";
        return false;
    }
    
    m_port = port;
    m_server = new QTcpServer(this);
    
    if (!m_server->listen(QHostAddress::LocalHost, port)) {
        qCritical() << "[AdbService] Failed to listen on port" << port 
                    << ":" << m_server->errorString();
        emit error(m_server->errorString());
        delete m_server;
        m_server = nullptr;
        return false;
    }
    
    connect(m_server, &QTcpServer::newConnection, this, &AdbService::onNewConnection);
    
    qDebug() << "[AdbService] Listening on port" << port;
    emit listeningChanged();
    return true;
}

void AdbService::stop()
{
    if (m_server) {
        for (auto* client : m_clients) {
            client->disconnectFromHost();
            client->deleteLater();
        }
        m_clients.clear();
        
        m_server->close();
        delete m_server;
        m_server = nullptr;
        
        qDebug() << "[AdbService] Stopped";
        emit listeningChanged();
    }
}

void AdbService::setAdbPath(const QString& path)
{
    m_adbPath = path;
    for (auto* conn : m_deviceConnections) {
        conn->setAdbPath(path);
    }
}

void AdbService::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* client = m_server->nextPendingConnection();
        qintptr clientId = client->socketDescriptor();
        
        m_clients[clientId] = client;
        
        connect(client, &QTcpSocket::disconnected, this, &AdbService::onClientDisconnected);
        connect(client, &QTcpSocket::readyRead, this, &AdbService::onReadyRead);
        
        qDebug() << "[AdbService] Client connected:" << clientId;
    }
}

void AdbService::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
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
    qDebug() << "[AdbService] Client disconnected:" << clientId;
}

void AdbService::onReadyRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    while (client->bytesAvailable() > 0) {
        QByteArray data = client->readLine();
        if (data.isEmpty()) continue;
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            QJsonObject errorResponse;
            errorResponse["success"] = false;
            errorResponse["error"] = error.errorString();
            client->write(QJsonDocument(errorResponse).toJson(QJsonDocument::Compact) + "\n");
            continue;
        }
        
        QJsonObject request = doc.object();
        QJsonObject response = handleRequest(request);
        
        // Copy correlation ID
        if (request.contains("id")) {
            response["id"] = request["id"];
        }
        
        client->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
        client->flush();
    }
}

QJsonObject AdbService::handleRequest(const QJsonObject& request)
{
    QString type = request["type"].toString();
    QString deviceId = request["deviceId"].toString();
    
    emit requestReceived(type, deviceId);
    
    if (type == "Ping") {
        return handlePing(request);
    } else if (type == "GetDevices") {
        return handleGetDevices(request);
    } else if (type == "GetEmulatorState") {
        return handleGetEmulatorState(request);
    } else if (type == "Execute") {
        return handleExecute(request);
    } else if (type == "ExecuteBatch") {
        return handleExecuteBatch(request);
    } else if (type == "IsFreeFireRunning") {
        return handleIsFreeFireRunning(request);
    }
    
    QJsonObject error;
    error["success"] = false;
    error["error"] = "Unknown request type: " + type;
    return error;
}

QJsonObject AdbService::handlePing(const QJsonObject& request)
{
    Q_UNUSED(request)
    QJsonObject response;
    response["success"] = true;
    response["type"] = "Pong";
    return response;
}

QJsonObject AdbService::handleGetDevices(const QJsonObject& request)
{
    Q_UNUSED(request)
    
    QProcess proc;
    proc.start(m_adbPath, {"devices"});
    proc.waitForFinished(5000);
    
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    QJsonArray devices;
    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.isEmpty()) continue;
        
        QStringList parts = line.split('\t');
        if (parts.size() >= 2) {
            QJsonObject device;
            device["id"] = parts[0];
            device["state"] = parts[1];
            devices.append(device);
        }
    }
    
    QJsonObject response;
    response["success"] = true;
    response["devices"] = devices;
    return response;
}

QJsonObject AdbService::handleGetEmulatorState(const QJsonObject& request)
{
    QString deviceId = request["deviceId"].toString();
    if (deviceId.isEmpty()) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "deviceId required";
        return error;
    }
    
    AdbConnection* conn = getConnection(deviceId);
    if (!conn) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "Failed to connect to device";
        return error;
    }
    
    // Batch fetch all emulator state
    auto result = conn->executeBatch({
        "wm size",
        "wm density",
        "pidof com.dts.freefireth",
        "dumpsys window displays | grep mCurrentFocus"
    });
    
    QJsonObject state;
    state["success"] = result.success;
    
    if (result.success && result.results.size() >= 4) {
        // Parse screen size
        QString sizeStr = result.results[0];
        QRegularExpression sizeRx("(\\d+)x(\\d+)");
        auto match = sizeRx.match(sizeStr);
        if (match.hasMatch()) {
            state["screenWidth"] = match.captured(1).toInt();
            state["screenHeight"] = match.captured(2).toInt();
        }
        
        // Parse density
        QString densityStr = result.results[1];
        QRegularExpression densityRx("(\\d+)");
        match = densityRx.match(densityStr);
        if (match.hasMatch()) {
            state["density"] = match.captured(1).toInt();
        }
        
        // Free Fire running
        state["freeFireRunning"] = !result.results[2].isEmpty();
        
        // Current focus
        state["currentFocus"] = result.results[3];
    }
    
    return state;
}

QJsonObject AdbService::handleExecute(const QJsonObject& request)
{
    QString deviceId = request["deviceId"].toString();
    QString command = request["command"].toString();
    
    if (deviceId.isEmpty() || command.isEmpty()) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "deviceId and command required";
        return error;
    }
    
    AdbConnection* conn = getConnection(deviceId);
    if (!conn) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "Failed to connect to device";
        return error;
    }
    
    QString result = conn->execute(command);
    
    QJsonObject response;
    response["success"] = true;
    response["result"] = result;
    return response;
}

QJsonObject AdbService::handleExecuteBatch(const QJsonObject& request)
{
    QString deviceId = request["deviceId"].toString();
    QJsonArray commands = request["commands"].toArray();
    
    if (deviceId.isEmpty() || commands.isEmpty()) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "deviceId and commands required";
        return error;
    }
    
    AdbConnection* conn = getConnection(deviceId);
    if (!conn) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "Failed to connect to device";
        return error;
    }
    
    QStringList cmdList;
    for (const auto& cmd : commands) {
        cmdList << cmd.toString();
    }
    
    auto result = conn->executeBatch(cmdList);
    
    QJsonObject response;
    response["success"] = result.success;
    response["totalTimeMs"] = result.totalTimeMs;
    
    QJsonArray results;
    for (const QString& r : result.results) {
        results.append(r);
    }
    response["results"] = results;
    
    return response;
}

QJsonObject AdbService::handleIsFreeFireRunning(const QJsonObject& request)
{
    QString deviceId = request["deviceId"].toString();
    
    if (deviceId.isEmpty()) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "deviceId required";
        return error;
    }
    
    AdbConnection* conn = getConnection(deviceId);
    if (!conn) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "Failed to connect to device";
        return error;
    }
    
    bool running = conn->isFreeFireRunning();
    
    QJsonObject response;
    response["success"] = true;
    response["running"] = running;
    return response;
}

AdbConnection* AdbService::getConnection(const QString& deviceId)
{
    if (m_deviceConnections.contains(deviceId)) {
        AdbConnection* conn = m_deviceConnections[deviceId];
        if (conn->isConnected()) {
            return conn;
        }
        // Connection lost, try to reconnect
        if (conn->connect(deviceId)) {
            return conn;
        }
        // Failed, remove stale connection
        delete conn;
        m_deviceConnections.remove(deviceId);
    }
    
    // Create new connection
    AdbConnection* conn = new AdbConnection(this);
    conn->setAdbPath(m_adbPath);
    
    if (!conn->connect(deviceId)) {
        delete conn;
        return nullptr;
    }
    
    m_deviceConnections[deviceId] = conn;
    return conn;
}

} // namespace NeoZ
