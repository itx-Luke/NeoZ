#ifndef NEOZ_DEVICEMANAGER_H
#define NEOZ_DEVICEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include "../adb/AdbConnection.h"

namespace NeoZ {

/**
 * @brief Manages ADB devices and emulator connections.
 * 
 * Extracted from NeoController. Handles:
 * - Device discovery and scanning
 * - Emulator state monitoring
 * - Free Fire detection
 * - ADB connection management
 */
class DeviceManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString adbStatus READ adbStatus NOTIFY statusChanged)
    Q_PROPERTY(QString selectedDevice READ selectedDevice WRITE setSelectedDevice NOTIFY devicesChanged)
    Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY devicesChanged)
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    
    // Emulator state
    Q_PROPERTY(QString resolution READ resolution NOTIFY emulatorStateChanged)
    Q_PROPERTY(QString mobileRes READ mobileRes NOTIFY emulatorStateChanged)
    Q_PROPERTY(QString mobileDpi READ mobileDpi NOTIFY emulatorStateChanged)
    Q_PROPERTY(bool freeFireRunning READ freeFireRunning NOTIFY emulatorStateChanged)
    Q_PROPERTY(QString processId READ processId NOTIFY emulatorStateChanged)
    
public:
    explicit DeviceManager(QObject* parent = nullptr);
    ~DeviceManager();
    
    // Status
    QString adbStatus() const { return m_adbStatus; }
    bool isScanning() const { return m_scanning; }
    bool isConnected() const { return m_connection && m_connection->isConnected(); }
    
    // Device selection
    QString selectedDevice() const { return m_selectedDevice; }
    void setSelectedDevice(const QString& device);
    QStringList availableDevices() const { return m_availableDevices; }
    
    // Connection
    AdbConnection* connection() const { return m_connection; }
    
    // Commands
    Q_INVOKABLE void scanForDevices();
    Q_INVOKABLE void connectToDevice(const QString& deviceId);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void refreshEmulatorState();
    
    // Emulator state
    QString resolution() const { return m_resolution; }
    QString mobileRes() const { return m_mobileRes; }
    QString mobileDpi() const { return m_mobileDpi; }
    bool freeFireRunning() const { return m_freeFireRunning; }
    QString processId() const { return m_processId; }
    
    // ADB path
    void setAdbPath(const QString& path);
    QString adbPath() const;
    
signals:
    void statusChanged();
    void devicesChanged();
    void scanningChanged();
    void connectionChanged();
    void emulatorStateChanged();
    void deviceConnected(const QString& deviceId);
    void deviceDisconnected();
    void freeFireStateChanged(bool running);
    
private slots:
    void onScanComplete();
    void checkFreeFireState();
    
private:
    QString m_adbStatus = "Not Connected";
    QString m_selectedDevice;
    QStringList m_availableDevices;
    bool m_scanning = false;
    
    // Emulator state
    QString m_resolution;
    QString m_mobileRes;
    QString m_mobileDpi;
    bool m_freeFireRunning = false;
    QString m_processId;
    
    AdbConnection* m_connection = nullptr;
    QTimer* m_statePollingTimer = nullptr;
};

} // namespace NeoZ

#endif // NEOZ_DEVICEMANAGER_H
