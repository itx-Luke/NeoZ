/**
 * LogitechHID.h - Logitech HID++ Protocol Controller
 * 
 * Implements real hardware DPI control for Logitech mice
 * using the HID++ 2.0 protocol with feature 0x2201 (AdjustableDPI)
 */

#ifndef LOGITECHHID_H
#define LOGITECHHID_H

#include <QObject>
#include <QString>
#include <QList>
#include <hidapi.h>

// Logitech Vendor ID
#define LOGITECH_VENDOR_ID 0x046D

// HID++ Constants
#define HIDPP_SHORT_MESSAGE_LENGTH 7
#define HIDPP_LONG_MESSAGE_LENGTH 20

// HID++ Feature IDs
#define HIDPP_FEATURE_ROOT 0x0000
#define HIDPP_FEATURE_FEATURE_SET 0x0001
#define HIDPP_FEATURE_ADJUSTABLE_DPI 0x2201

// Report IDs
#define HIDPP_REPORT_ID_SHORT 0x10
#define HIDPP_REPORT_ID_LONG 0x11

struct LogitechMouseInfo {
    QString name;
    QString path;
    unsigned short productId;
    int currentDpi;
    int minDpi;
    int maxDpi;
    int dpiStep;
    bool connected;
};

class LogitechHIDController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(int currentDpi READ currentDpi NOTIFY dpiChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY connectionChanged)

public:
    explicit LogitechHIDController(QObject *parent = nullptr);
    ~LogitechHIDController();

    // Device management
    Q_INVOKABLE bool scanForDevices();
    Q_INVOKABLE bool connectToDevice(const QString& path = "");
    Q_INVOKABLE void disconnect();
    
    // DPI control
    Q_INVOKABLE bool setDpi(int dpi);
    Q_INVOKABLE int getDpi();
    Q_INVOKABLE int getMinDpi() const { return m_mouseInfo.minDpi; }
    Q_INVOKABLE int getMaxDpi() const { return m_mouseInfo.maxDpi; }
    Q_INVOKABLE int getDpiStep() const { return m_mouseInfo.dpiStep; }
    
    // Properties
    bool isConnected() const { return m_device != nullptr; }
    int currentDpi() const { return m_mouseInfo.currentDpi; }
    QString deviceName() const { return m_mouseInfo.name; }
    
    // Get available devices
    QList<LogitechMouseInfo> availableDevices() const { return m_availableDevices; }

signals:
    void connectionChanged();
    void dpiChanged(int newDpi);
    void deviceFound(QString name, QString path);
    void error(QString message);

private:
    // HID++ Protocol
    bool sendHidppMessage(uint8_t* message, int length);
    bool receiveHidppMessage(uint8_t* buffer, int length, int timeout = 1000);
    
    // Feature management
    uint8_t getFeatureIndex(uint16_t featureId);
    
    // DPI feature (0x2201)
    bool readDpiInfo();
    bool readCurrentDpi();
    bool writeDpi(int dpi);
    
    // Device handle
    hid_device* m_device = nullptr;
    LogitechMouseInfo m_mouseInfo;
    QList<LogitechMouseInfo> m_availableDevices;
    
    // Feature indices cache
    uint8_t m_dpiFeatureIndex = 0;
    bool m_hidInitialized = false;
};

#endif // LOGITECHHID_H
