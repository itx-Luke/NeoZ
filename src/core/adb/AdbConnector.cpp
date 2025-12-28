#include "AdbConnector.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QRegularExpression>

AdbConnector::AdbConnector(QObject *parent)
    : QObject(parent),
      m_scanTimer(new QTimer(this)),
      m_adbProcess(new QProcess(this))
{
    detectAdbPath();
    
    connect(m_scanTimer, &QTimer::timeout, this, &AdbConnector::onScanTimeout);
    connect(m_adbProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AdbConnector::processScanResult);
}

AdbConnector::~AdbConnector()
{
    stopScan();
}

void AdbConnector::detectAdbPath()
{
    // Try common ADB locations
    QStringList possiblePaths = {
        "adb", // System PATH
        "C:/Program Files/BlueStacks_nxt/HD-Adb.exe",
        "C:/Program Files (x86)/BlueStacks_nxt/HD-Adb.exe",
        "C:/Program Files/Bluestacks/HD-Adb.exe",
        "C:/Program Files (x86)/Bluestacks/HD-Adb.exe",
        QDir::homePath() + "/AppData/Local/Android/Sdk/platform-tools/adb.exe"
    };
    
    for (const QString& path : possiblePaths) {
        QProcess testProcess;
        testProcess.start(path, QStringList() << "version");
        if (testProcess.waitForFinished(1000)) {
            qDebug() << "[AdbConnector] Found ADB at:" << path;
            m_adbPath = path;
            return;
        }
    }
    
    qDebug() << "[AdbConnector] Warning: ADB not found, using 'adb'";
    m_adbPath = "adb";
}

QString AdbConnector::getAdbPath()
{
    return m_adbPath;
}

void AdbConnector::killServer()
{
    QProcess killProcess;
    killProcess.start(m_adbPath, QStringList() << "kill-server");
    killProcess.waitForFinished(3000);
    qDebug() << "[AdbConnector] Killed ADB server";
    
    m_connectionStatus = "Disconnected";
    emit connectionStatusChanged();
}

void AdbConnector::restartServer()
{
    killServer();
    
    QProcess startProcess;
    startProcess.start(m_adbPath, QStringList() << "start-server");
    startProcess.waitForFinished(5000);
    qDebug() << "[AdbConnector] Started ADB server";
    
    m_connectionStatus = "Ready";
    emit connectionStatusChanged();
}

void AdbConnector::startScan()
{
    if (m_isScanning) return;
    
    qDebug() << "[AdbConnector] Starting device scan...";
    
    m_isScanning = true;
    emit scanningChanged();
    
    m_connectionStatus = "Scanning...";
    emit connectionStatusChanged();
    
    // Kill and restart server for fresh scan
    restartServer();
    
    // Clear previous devices
    m_devices.clear();
    m_deviceList.clear();
    m_currentPortIndex = 0;
    
    // Start trying ports
    tryConnectPort(m_commonPorts[m_currentPortIndex]);
    
    // Set overall timeout (10 seconds)
    m_scanTimer->start(10000);
}

void AdbConnector::stopScan()
{
    m_scanTimer->stop();
    m_isScanning = false;
    emit scanningChanged();
    
    if (m_adbProcess->state() != QProcess::NotRunning) {
        m_adbProcess->kill();
        m_adbProcess->waitForFinished(1000);
    }
}

void AdbConnector::tryConnectPort(const QString& port)
{
    QString address = QString("127.0.0.1:%1").arg(port);
    qDebug() << "[AdbConnector] Trying port:" << address;
    
    m_adbProcess->start(m_adbPath, QStringList() << "connect" << address);
}

void AdbConnector::processScanResult()
{
    QString output = m_adbProcess->readAllStandardOutput();
    QString errorOut = m_adbProcess->readAllStandardError();
    
    QString currentPort = m_commonPorts[m_currentPortIndex];
    QString address = QString("127.0.0.1:%1").arg(currentPort);
    
    if (output.contains("connected") || output.contains("already connected")) {
        qDebug() << "[AdbConnector] Connected to:" << address;
        
        EmulatorDevice device;
        device.id = address;
        device.port = currentPort;
        device.isConnected = true;
        device.status = "device";
        
        // Try to detect emulator type
        if (currentPort == "5555" || currentPort == "5556") {
            device.name = "BlueStacks";
        } else if (currentPort == "62001") {
            device.name = "NoxPlayer";
        } else if (currentPort == "21503") {
            device.name = "LDPlayer";
        } else {
            device.name = "Emulator";
        }
        
        m_devices.append(device);
        m_deviceList.append(QString("%1 (%2)").arg(device.name, address));
        emit deviceListChanged();
        emit deviceFound(device.id, device.name);
    }
    
    // Try next port
    m_currentPortIndex++;
    if (m_currentPortIndex < m_commonPorts.size()) {
        tryConnectPort(m_commonPorts[m_currentPortIndex]);
    } else {
        // All ports tried, update device list from 'adb devices'
        updateDeviceList();
    }
}

void AdbConnector::updateDeviceList()
{
    QProcess devicesProcess;
    devicesProcess.start(m_adbPath, QStringList() << "devices" << "-l");
    
    if (devicesProcess.waitForFinished(3000)) {
        parseDevices(devicesProcess.readAllStandardOutput());
    }
    
    // Scan complete
    stopScan();
    
    if (m_devices.isEmpty()) {
        m_connectionStatus = "No Devices Found";
    } else {
        m_connectionStatus = QString("%1 Device(s) Found").arg(m_devices.size());
    }
    emit connectionStatusChanged();
    emit scanComplete(m_devices.size());
    
    qDebug() << "[AdbConnector] Scan complete. Found:" << m_devices.size() << "devices";
}

void AdbConnector::parseDevices(const QString& output)
{
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        if (line.startsWith("List of devices") || line.trimmed().isEmpty()) continue;
        
        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            QString deviceId = parts[0];
            QString status = parts[1];
            
            // Check if already in list
            bool found = false;
            for (const EmulatorDevice& d : m_devices) {
                if (d.id == deviceId) {
                    found = true;
                    break;
                }
            }
            
            if (!found && status == "device") {
                EmulatorDevice device;
                device.id = deviceId;
                device.status = status;
                device.isConnected = true;
                
                // Try to detect name from model info
                if (line.contains("model:")) {
                    QRegularExpression re("model:(\\S+)");
                    QRegularExpressionMatch match = re.match(line);
                    if (match.hasMatch()) {
                        device.name = match.captured(1).replace("_", " ");
                    }
                } else {
                    device.name = "Android Device";
                }
                
                m_devices.append(device);
                m_deviceList.append(QString("%1 (%2)").arg(device.name, deviceId));
                emit deviceListChanged();
            }
        }
    }
}

void AdbConnector::onScanTimeout()
{
    qDebug() << "[AdbConnector] Scan timeout";
    updateDeviceList();
}

void AdbConnector::connectToDevice(const QString& deviceId)
{
    qDebug() << "[AdbConnector] Connecting to device:" << deviceId;
    
    m_connectionStatus = "Connecting...";
    emit connectionStatusChanged();
    
    QProcess connectProcess;
    connectProcess.start(m_adbPath, QStringList() << "connect" << deviceId);
    
    if (connectProcess.waitForFinished(5000)) {
        QString output = connectProcess.readAllStandardOutput();
        if (output.contains("connected") || output.contains("already connected")) {
            m_selectedDevice = deviceId;
            m_connectionStatus = "Connected";
            emit selectedDeviceChanged();
            emit connectionStatusChanged();
            emit deviceConnected(deviceId);
            qDebug() << "[AdbConnector] Successfully connected to:" << deviceId;
        } else {
            m_connectionStatus = "Connection Failed";
            emit connectionStatusChanged();
            emit error("Failed to connect to " + deviceId);
        }
    } else {
        m_connectionStatus = "Connection Timeout";
        emit connectionStatusChanged();
        emit error("Connection timeout for " + deviceId);
    }
}

void AdbConnector::disconnectDevice()
{
    if (m_selectedDevice.isEmpty()) return;
    
    qDebug() << "[AdbConnector] Disconnecting from:" << m_selectedDevice;
    
    QProcess disconnectProcess;
    disconnectProcess.start(m_adbPath, QStringList() << "disconnect" << m_selectedDevice);
    disconnectProcess.waitForFinished(3000);
    
    m_selectedDevice.clear();
    m_connectionStatus = "Disconnected";
    emit selectedDeviceChanged();
    emit connectionStatusChanged();
    emit deviceDisconnected();
}

void AdbConnector::setSelectedDevice(const QString& device)
{
    if (m_selectedDevice != device) {
        m_selectedDevice = device;
        emit selectedDeviceChanged();
    }
}

QString AdbConnector::executeCommand(const QString& command, int timeoutMs)
{
    if (m_selectedDevice.isEmpty()) {
        qDebug() << "[AdbConnector] executeCommand failed: no device selected";
        return QString();
    }
    
    QProcess adbProcess;
    QStringList args;
    args << "-s" << m_selectedDevice << "shell" << command;
    
    qDebug() << "[AdbConnector] Executing:" << m_adbPath << args.join(' ');
    
    adbProcess.start(m_adbPath, args);
    
    if (adbProcess.waitForFinished(timeoutMs)) {
        QString output = adbProcess.readAllStandardOutput().trimmed();
        QString errorOut = adbProcess.readAllStandardError().trimmed();
        
        if (!errorOut.isEmpty()) {
            qDebug() << "[AdbConnector] Command error:" << errorOut;
        }
        
        qDebug() << "[AdbConnector] Command output:" << output;
        return output;
    }
    
    qDebug() << "[AdbConnector] Command timed out after" << timeoutMs << "ms";
    return QString();
}
