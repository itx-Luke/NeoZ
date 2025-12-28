#include "DeviceManager.h"
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

namespace NeoZ {

DeviceManager::DeviceManager(QObject* parent)
    : QObject(parent)
{
    m_connection = new AdbConnection(this);
    
    // State polling timer - checks Free Fire status periodically
    m_statePollingTimer = new QTimer(this);
    m_statePollingTimer->setInterval(2000); // 2 seconds
    connect(m_statePollingTimer, &QTimer::timeout, this, &DeviceManager::checkFreeFireState);
}

DeviceManager::~DeviceManager()
{
    disconnect();
}

void DeviceManager::setSelectedDevice(const QString& device)
{
    if (m_selectedDevice == device) return;
    m_selectedDevice = device;
    emit devicesChanged();
    
    // Auto-connect when device selected
    if (!device.isEmpty()) {
        connectToDevice(device);
    }
}

void DeviceManager::setAdbPath(const QString& path)
{
    if (m_connection) {
        m_connection->setAdbPath(path);
    }
}

QString DeviceManager::adbPath() const
{
    return m_connection ? m_connection->adbPath() : "adb";
}

void DeviceManager::scanForDevices()
{
    if (m_scanning) return;
    
    m_scanning = true;
    emit scanningChanged();
    
    // Run adb devices
    QProcess* proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int exitCode, QProcess::ExitStatus) {
        
        m_availableDevices.clear();
        
        if (exitCode == 0) {
            QString output = QString::fromUtf8(proc->readAllStandardOutput());
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);
            
            for (int i = 1; i < lines.size(); ++i) {
                QString line = lines[i].trimmed();
                QStringList parts = line.split('\t');
                if (parts.size() >= 2 && parts[1] == "device") {
                    m_availableDevices << parts[0];
                }
            }
            
            m_adbStatus = m_availableDevices.isEmpty() ? "No devices" : "Found " + QString::number(m_availableDevices.size()) + " device(s)";
        } else {
            m_adbStatus = "ADB Error";
        }
        
        m_scanning = false;
        proc->deleteLater();
        
        emit statusChanged();
        emit devicesChanged();
        emit scanningChanged();
        onScanComplete();
    });
    
    proc->start(adbPath(), {"devices"});
}

void DeviceManager::connectToDevice(const QString& deviceId)
{
    if (deviceId.isEmpty()) {
        qWarning() << "[DeviceManager] Cannot connect: empty device ID";
        return;
    }
    
    if (m_connection->connect(deviceId)) {
        m_selectedDevice = deviceId;
        m_adbStatus = "Connected: " + deviceId;
        
        // Start state polling
        m_statePollingTimer->start();
        refreshEmulatorState();
        
        emit connectionChanged();
        emit statusChanged();
        emit devicesChanged();
        emit deviceConnected(deviceId);
        
        qDebug() << "[DeviceManager] Connected to" << deviceId;
    } else {
        m_adbStatus = "Connection failed";
        emit statusChanged();
    }
}

void DeviceManager::disconnect()
{
    m_statePollingTimer->stop();
    
    if (m_connection) {
        m_connection->disconnect();
    }
    
    m_selectedDevice.clear();
    m_adbStatus = "Disconnected";
    m_resolution.clear();
    m_mobileRes.clear();
    m_mobileDpi.clear();
    m_freeFireRunning = false;
    m_processId.clear();
    
    emit connectionChanged();
    emit statusChanged();
    emit devicesChanged();
    emit emulatorStateChanged();
    emit deviceDisconnected();
    
    qDebug() << "[DeviceManager] Disconnected";
}

void DeviceManager::refreshEmulatorState()
{
    if (!isConnected()) return;
    
    // Batch fetch emulator state
    auto result = m_connection->executeBatch({
        "wm size",
        "wm density",
        "pidof com.dts.freefireth"
    });
    
    if (result.success && result.results.size() >= 3) {
        // Parse screen size
        QString sizeStr = result.results[0];
        static QRegularExpression sizeRx("(\\d+)x(\\d+)");
        auto match = sizeRx.match(sizeStr);
        if (match.hasMatch()) {
            m_mobileRes = match.captured(1) + "x" + match.captured(2);
        }
        
        // Parse density
        QString densityStr = result.results[1];
        static QRegularExpression densityRx("(\\d+)");
        match = densityRx.match(densityStr);
        if (match.hasMatch()) {
            m_mobileDpi = match.captured(1);
        }
        
        // Free Fire PID
        m_processId = result.results[2].trimmed();
        bool wasRunning = m_freeFireRunning;
        m_freeFireRunning = !m_processId.isEmpty();
        
        if (m_freeFireRunning != wasRunning) {
            emit freeFireStateChanged(m_freeFireRunning);
        }
        
        emit emulatorStateChanged();
    }
}

void DeviceManager::onScanComplete()
{
    qDebug() << "[DeviceManager] Scan complete. Found:" << m_availableDevices.size() << "devices";
    
    // Auto-select first device if none selected
    if (m_selectedDevice.isEmpty() && !m_availableDevices.isEmpty()) {
        setSelectedDevice(m_availableDevices.first());
    }
}

void DeviceManager::checkFreeFireState()
{
    if (!isConnected()) return;
    
    // Quick cached check
    bool running = m_connection->isFreeFireRunning();
    
    if (running != m_freeFireRunning) {
        m_freeFireRunning = running;
        emit emulatorStateChanged();
        emit freeFireStateChanged(running);
    }
}

} // namespace NeoZ
