/**
 * LogitechHID.cpp - Logitech HID++ Protocol Implementation
 * 
 * Implements real hardware DPI control for Logitech mice
 * using the HID++ 2.0 protocol with feature 0x2201 (AdjustableDPI)
 */

#include "LogitechHID.h"
#include <QDebug>
#include <cstring>

// Device index: 0x01 for wired mice, 0xFF for wireless receiver
static const uint8_t DEVICE_INDEX_WIRED = 0x01;
static const uint8_t DEVICE_INDEX_RECEIVER = 0xFF;

// Known G203 product IDs
static const uint16_t G203_PRODIGY_PID = 0xC084;
static const uint16_t G203_LIGHTSYNC_PID = 0xC092;

LogitechHIDController::LogitechHIDController(QObject *parent)
    : QObject(parent)
{
    // Initialize hidapi with safety check
    int initResult = hid_init();
    if (initResult != 0) {
        qWarning() << "[LogitechHID] Failed to initialize hidapi - HID features disabled";
        m_hidInitialized = false;
    } else {
        qDebug() << "[LogitechHID] hidapi initialized successfully";
        m_hidInitialized = true;
    }
    
    // Only scan if hidapi initialized successfully
    if (m_hidInitialized) {
        try {
            scanForDevices();
        } catch (...) {
            qWarning() << "[LogitechHID] Exception during device scan - continuing without HID";
        }
    }
}

LogitechHIDController::~LogitechHIDController()
{
    disconnect();
    hid_exit();
}

bool LogitechHIDController::scanForDevices()
{
    m_availableDevices.clear();
    
    // Enumerate all Logitech HID devices
    struct hid_device_info* devs = hid_enumerate(LOGITECH_VENDOR_ID, 0);
    struct hid_device_info* cur = devs;
    
    while (cur) {
        // Look for HID++ devices (usage page 0xFF00 for vendor-defined)
        // We MUST use the vendor page to avoid "Access is denied" on Windows
        if (cur->usage_page == 0xFF00) {
            LogitechMouseInfo info;
            info.productId = cur->product_id;
            info.path = QString::fromUtf8(cur->path);
            info.name = cur->product_string ? 
                QString::fromWCharArray(cur->product_string) : 
                QString("Logitech Device (0x%1)").arg(cur->product_id, 4, 16, QChar('0'));
            info.connected = false;
            info.currentDpi = 0;
            info.minDpi = 200;
            info.maxDpi = 16000;
            info.dpiStep = 50;
            
            // Avoid duplicates
            bool exists = false;
            for (const auto& existing : m_availableDevices) {
                if (existing.productId == info.productId && existing.path == info.path) {
                    exists = true;
                    break;
                }
            }
            
            if (!exists) {
                m_availableDevices.append(info);
                emit deviceFound(info.name, info.path);
                qDebug() << "[LogitechHID] Found device:" << info.name 
                         << "PID:" << Qt::hex << info.productId;
            }
        }
        cur = cur->next;
    }
    
    hid_free_enumeration(devs);
    
    qDebug() << "[LogitechHID] Scan complete. Found" << m_availableDevices.size() << "devices";
    return !m_availableDevices.isEmpty();
}

bool LogitechHIDController::connectToDevice(const QString& path)
{
    disconnect();
    
    QString targetPath = path;
    
    // If no path specified, try first available device
    if (targetPath.isEmpty() && !m_availableDevices.isEmpty()) {
        targetPath = m_availableDevices.first().path;
    }
    
    if (targetPath.isEmpty()) {
        emit error("No Logitech device found");
        return false;
    }
    
    // Open the device
    m_device = hid_open_path(targetPath.toUtf8().constData());
    
    if (!m_device) {
        emit error(QString("Failed to open device: %1").arg(
            QString::fromWCharArray(hid_error(nullptr))));
        return false;
    }
    
    // Set non-blocking mode
    hid_set_nonblocking(m_device, 1);
    
    // Find device info
    for (auto& dev : m_availableDevices) {
        if (dev.path == targetPath) {
            m_mouseInfo = dev;
            m_mouseInfo.connected = true;
            break;
        }
    }
    
    qDebug() << "[LogitechHID] Connected to:" << m_mouseInfo.name;
    
    // Try to get the DPI feature index
    m_dpiFeatureIndex = getFeatureIndex(HIDPP_FEATURE_ADJUSTABLE_DPI);
    
    if (m_dpiFeatureIndex == 0) {
        qWarning() << "[LogitechHID] Device does not support AdjustableDPI feature";
        // Still connected, but DPI control may not work
    } else {
        qDebug() << "[LogitechHID] DPI feature index:" << m_dpiFeatureIndex;
        
        // Read DPI info (min, max, step)
        readDpiInfo();
        
        // Read current DPI
        readCurrentDpi();
    }
    
    emit connectionChanged();
    return true;
}

void LogitechHIDController::disconnect()
{
    if (m_device) {
        hid_close(m_device);
        m_device = nullptr;
        m_mouseInfo.connected = false;
        m_dpiFeatureIndex = 0;
        emit connectionChanged();
        qDebug() << "[LogitechHID] Disconnected";
    }
}

uint8_t LogitechHIDController::getFeatureIndex(uint16_t featureId)
{
    if (!m_device) return 0;
    
    // HID++ 2.0: Use Feature 0x0000 (IRoot) function 0 to get feature index
    // Use device index 0x01 for wired mice (like G203)
    uint8_t request[HIDPP_LONG_MESSAGE_LENGTH] = {0};
    request[0] = HIDPP_REPORT_ID_LONG;  // Long report
    request[1] = DEVICE_INDEX_WIRED;     // Device index (0x01 for wired mice)
    request[2] = 0x00;                   // Feature index 0 (IRoot)
    request[3] = 0x00;                   // Function 0 (getFeatureIndex) with SW ID
    request[4] = (featureId >> 8) & 0xFF; // Feature ID high byte
    request[5] = featureId & 0xFF;        // Feature ID low byte
    
    if (!sendHidppMessage(request, HIDPP_LONG_MESSAGE_LENGTH)) {
        return 0;
    }
    
    uint8_t response[HIDPP_LONG_MESSAGE_LENGTH] = {0};
    if (!receiveHidppMessage(response, HIDPP_LONG_MESSAGE_LENGTH)) {
        return 0;
    }
    
    // Response format: [report_id][device_index][feature_index][function][feature_index_result]...
    return response[4];  // Feature index is in byte 4
}

bool LogitechHIDController::readDpiInfo()
{
    if (!m_device || m_dpiFeatureIndex == 0) return false;
    
    // Function 1: getSensorDpiList - get DPI capabilities
    uint8_t request[HIDPP_LONG_MESSAGE_LENGTH] = {0};
    request[0] = HIDPP_REPORT_ID_LONG;
    request[1] = DEVICE_INDEX_WIRED;
    request[2] = m_dpiFeatureIndex;
    request[3] = 0x10;  // Function 1 (getSensorDpiList) << 4
    
    if (!sendHidppMessage(request, HIDPP_LONG_MESSAGE_LENGTH)) {
        return false;
    }
    
    uint8_t response[HIDPP_LONG_MESSAGE_LENGTH] = {0};
    if (!receiveHidppMessage(response, HIDPP_LONG_MESSAGE_LENGTH)) {
        return false;
    }
    
    // Parse DPI list - format depends on device
    // Typical format: min_dpi (2 bytes), max_dpi (2 bytes), step (2 bytes)
    if (response[4] != 0 || response[5] != 0) {
        m_mouseInfo.minDpi = (response[4] << 8) | response[5];
        m_mouseInfo.maxDpi = (response[6] << 8) | response[7];
        m_mouseInfo.dpiStep = (response[8] << 8) | response[9];
        
        if (m_mouseInfo.dpiStep == 0) m_mouseInfo.dpiStep = 50;
        
        qDebug() << "[LogitechHID] DPI range:" << m_mouseInfo.minDpi 
                 << "-" << m_mouseInfo.maxDpi 
                 << "step:" << m_mouseInfo.dpiStep;
    }
    
    return true;
}

bool LogitechHIDController::readCurrentDpi()
{
    if (!m_device || m_dpiFeatureIndex == 0) return false;
    
    // Function 2: getSensorDpi - get current DPI
    uint8_t request[HIDPP_LONG_MESSAGE_LENGTH] = {0};
    request[0] = HIDPP_REPORT_ID_LONG;
    request[1] = DEVICE_INDEX_WIRED;
    request[2] = m_dpiFeatureIndex;
    request[3] = 0x20;  // Function 2 (getSensorDpi) << 4
    request[4] = 0x00;  // Sensor index 0
    
    if (!sendHidppMessage(request, HIDPP_LONG_MESSAGE_LENGTH)) {
        return false;
    }
    
    uint8_t response[HIDPP_LONG_MESSAGE_LENGTH] = {0};
    if (!receiveHidppMessage(response, HIDPP_LONG_MESSAGE_LENGTH)) {
        return false;
    }
    
    // DPI is in bytes 4-5 (big-endian)
    int dpi = (response[4] << 8) | response[5];
    if (dpi > 0 && dpi <= 32000) {
        m_mouseInfo.currentDpi = dpi;
        qDebug() << "[LogitechHID] Current DPI:" << dpi;
        emit dpiChanged(dpi);
        return true;
    }
    
    return false;
}

bool LogitechHIDController::writeDpi(int dpi)
{
    if (!m_device || m_dpiFeatureIndex == 0) return false;
    
    // Clamp to valid range
    if (dpi < m_mouseInfo.minDpi) dpi = m_mouseInfo.minDpi;
    if (dpi > m_mouseInfo.maxDpi) dpi = m_mouseInfo.maxDpi;
    
    // Round to step
    dpi = (dpi / m_mouseInfo.dpiStep) * m_mouseInfo.dpiStep;
    
    // Function 3: setSensorDpi - set DPI
    uint8_t request[HIDPP_LONG_MESSAGE_LENGTH] = {0};
    request[0] = HIDPP_REPORT_ID_LONG;
    request[1] = DEVICE_INDEX_WIRED;
    request[2] = m_dpiFeatureIndex;
    request[3] = 0x30;  // Function 3 (setSensorDpi) << 4
    request[4] = 0x00;  // Sensor index 0
    request[5] = (dpi >> 8) & 0xFF;  // DPI high byte
    request[6] = dpi & 0xFF;          // DPI low byte
    
    if (!sendHidppMessage(request, HIDPP_LONG_MESSAGE_LENGTH)) {
        return false;
    }
    
    uint8_t response[HIDPP_LONG_MESSAGE_LENGTH] = {0};
    if (!receiveHidppMessage(response, HIDPP_LONG_MESSAGE_LENGTH)) {
        // Some mice don't respond to set commands, try reading back
        readCurrentDpi();
        return m_mouseInfo.currentDpi == dpi;
    }
    
    m_mouseInfo.currentDpi = dpi;
    emit dpiChanged(dpi);
    qDebug() << "[LogitechHID] DPI set to:" << dpi;
    
    return true;
}

bool LogitechHIDController::setDpi(int dpi)
{
    if (!isConnected()) {
        // Try to auto-connect
        if (!connectToDevice()) {
            emit error("No Logitech device connected");
            return false;
        }
    }
    
    return writeDpi(dpi);
}

int LogitechHIDController::getDpi()
{
    if (!isConnected()) {
        return m_mouseInfo.currentDpi;
    }
    
    readCurrentDpi();
    return m_mouseInfo.currentDpi;
}

bool LogitechHIDController::sendHidppMessage(uint8_t* message, int length)
{
    if (!m_device) return false;
    
    int result = hid_write(m_device, message, length);
    if (result < 0) {
        qWarning() << "[LogitechHID] Write failed:" 
                   << QString::fromWCharArray(hid_error(m_device));
        return false;
    }
    
    return true;
}

bool LogitechHIDController::receiveHidppMessage(uint8_t* buffer, int length, int timeout)
{
    if (!m_device) return false;
    
    memset(buffer, 0, length);
    
    // Try multiple times with short timeouts
    for (int attempt = 0; attempt < 10; ++attempt) {
        int result = hid_read_timeout(m_device, buffer, length, timeout / 10);
        
        if (result > 0) {
            // Check for error response
            if (buffer[2] == 0xFF) {
                uint8_t errorCode = buffer[4];
                qWarning() << "[LogitechHID] HID++ error response, code:" << errorCode;
                return false;
            }
            return true;
        } else if (result < 0) {
            qWarning() << "[LogitechHID] Read failed:" 
                       << QString::fromWCharArray(hid_error(m_device));
            return false;
        }
        // result == 0 means timeout, try again
    }
    
    return false;
}
