#ifndef NEOZ_ADBSERVICE_H
#define NEOZ_ADBSERVICE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include "../core/adb/AdbConnection.h"

namespace NeoZ {

/**
 * @brief ADB Service - TCP server for ADB operations.
 * 
 * Runs as a separate executable (NeoZ_AdbService.exe) and handles
 * all ADB communication for the Core process.
 * 
 * Protocol:
 * - JSON messages over TCP (port 5557)
 * - Request/response pattern
 * - Supports batch commands
 * 
 * Message Types:
 * - GetDevices: List connected ADB devices
 * - GetEmulatorState: Get emulator screen size, density, etc.
 * - Execute: Run arbitrary ADB shell command
 * - ExecuteBatch: Run multiple commands in single connection
 * - IsFreeFireRunning: Check if Free Fire is running
 */
class AdbService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool listening READ isListening NOTIFY listeningChanged)
    Q_PROPERTY(int port READ port NOTIFY listeningChanged)
    
public:
    explicit AdbService(QObject* parent = nullptr);
    ~AdbService();
    
    /**
     * @brief Start the TCP server
     * @param port Port to listen on (default: 5557)
     * @return true if started successfully
     */
    bool start(quint16 port = 5557);
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Check if server is listening
     */
    bool isListening() const { return m_server && m_server->isListening(); }
    
    /**
     * @brief Get the listening port
     */
    quint16 port() const { return m_port; }
    
    /**
     * @brief Set ADB path
     */
    void setAdbPath(const QString& path);
    
signals:
    void listeningChanged();
    void error(const QString& errorMessage);
    void requestReceived(const QString& type, const QString& deviceId);
    
private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();
    
private:
    QJsonObject handleRequest(const QJsonObject& request);
    
    // Request handlers
    QJsonObject handleGetDevices(const QJsonObject& request);
    QJsonObject handleGetEmulatorState(const QJsonObject& request);
    QJsonObject handleExecute(const QJsonObject& request);
    QJsonObject handleExecuteBatch(const QJsonObject& request);
    QJsonObject handleIsFreeFireRunning(const QJsonObject& request);
    QJsonObject handlePing(const QJsonObject& request);
    
    QTcpServer* m_server = nullptr;
    quint16 m_port = 5557;
    QHash<qintptr, QTcpSocket*> m_clients;
    
    // Each device gets its own AdbConnection for efficiency
    QHash<QString, AdbConnection*> m_deviceConnections;
    QString m_adbPath;
    
    AdbConnection* getConnection(const QString& deviceId);
};

} // namespace NeoZ

#endif // NEOZ_ADBSERVICE_H
