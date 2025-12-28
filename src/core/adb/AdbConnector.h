#ifndef ADBCONNECTOR_H
#define ADBCONNECTOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QTimer>

// Represents a detected emulator device
struct EmulatorDevice {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString status MEMBER status)
    Q_PROPERTY(QString port MEMBER port)
    Q_PROPERTY(bool isConnected MEMBER isConnected)
public:
    QString id;         // e.g., "127.0.0.1:5555"
    QString name;       // e.g., "BlueStacks", "HD-Player"
    QString status;     // e.g., "device", "offline", "connecting"
    QString port;       // e.g., "5555"
    bool isConnected = false;
};

class AdbConnector : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(QString selectedDevice READ selectedDevice WRITE setSelectedDevice NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QStringList deviceList READ deviceList NOTIFY deviceListChanged)

public:
    explicit AdbConnector(QObject *parent = nullptr);
    ~AdbConnector();

    bool isScanning() const { return m_isScanning; }
    QString selectedDevice() const { return m_selectedDevice; }
    QString connectionStatus() const { return m_connectionStatus; }
    QStringList deviceList() const { return m_deviceList; }

    void setSelectedDevice(const QString& device);

    // Public methods exposed to QML
    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();
    Q_INVOKABLE void connectToDevice(const QString& deviceId);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void killServer();
    Q_INVOKABLE void restartServer();
    Q_INVOKABLE QString getAdbPath();
    
    // Execute arbitrary ADB shell command on the connected device
    Q_INVOKABLE QString executeCommand(const QString& command, int timeoutMs = 3000);

signals:
    void scanningChanged();
    void selectedDeviceChanged();
    void connectionStatusChanged();
    void deviceListChanged();
    void deviceFound(const QString& deviceId, const QString& deviceName);
    void deviceConnected(const QString& deviceId);
    void deviceDisconnected();
    void scanComplete(int deviceCount);
    void error(const QString& message);

private slots:
    void onScanTimeout();
    void processScanResult();

private:
    void detectAdbPath();
    void tryConnectPort(const QString& port);
    void updateDeviceList();
    void parseDevices(const QString& output);

    bool m_isScanning = false;
    QString m_selectedDevice;
    QString m_connectionStatus = "Disconnected";
    QStringList m_deviceList;
    QString m_adbPath;
    
    QTimer* m_scanTimer;
    QProcess* m_adbProcess;
    QStringList m_commonPorts = {"5555", "5556", "5554", "62001", "21503"};
    int m_currentPortIndex = 0;
    
    QList<EmulatorDevice> m_devices;
};

#endif // ADBCONNECTOR_H
