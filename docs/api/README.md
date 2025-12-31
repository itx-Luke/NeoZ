# Neo-Z API Reference

Complete API documentation for Neo-Z core modules.

---

## Table of Contents

- [NeoController](#neocontroller)
- [Sensitivity System](#sensitivity-system)
- [ADB Integration](#adb-integration)
- [AI Services](#ai-services)
- [Input System](#input-system)

---

## NeoController

**File:** `src/backend/NeoController.h`

The main controller exposed to QML, orchestrating all subsystems.

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `emulatorStatus` | QString | Read | Current emulator connection status |
| `adbStatus` | QString | Read | ADB daemon status |
| `resolution` | QString | Read | Detected screen resolution |
| `xMultiplier` | double | R/W | Horizontal sensitivity multiplier |
| `yMultiplier` | double | R/W | Vertical sensitivity multiplier |
| `smoothing` | int | R/W | Input smoothing level (0-100) |
| `mouseDpi` | int | R/W | Mouse DPI setting |
| `drcsEnabled` | bool | R/W | DRCS enable/disable |
| `aiEnabled` | bool | R/W | AI recommendations enable |
| `selectedDevice` | QString | R/W | Selected ADB device ID |

### Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| `statusChanged` | — | Emitted when any status property changes |
| `sensitivityChanged` | — | Sensitivity settings modified |
| `devicesChanged` | — | ADB device list updated |
| `aiEnabledChanged` | — | AI toggle state changed |
| `drcsChanged` | — | DRCS settings modified |

### Slots

| Method | Parameters | Returns | Description |
|--------|------------|---------|-------------|
| `scanEmulators()` | — | void | Scan for running emulators |
| `connectAdb()` | — | void | Connect to selected ADB device |
| `disconnectAdb()` | — | void | Disconnect ADB |
| `refreshDevices()` | — | void | Refresh device list |
| `applySettings()` | — | void | Push settings to device |

---

## Sensitivity System

### DRCS (Dynamic Resolution Calibration System)

**File:** `src/core/sensitivity/DRCS.h`

Automatically adjusts sensitivity based on resolution changes.

```cpp
class DRCS : public QObject {
    // Core methods
    void enable();
    void disable();
    double calculateMultiplier(int width, int height);
    
    // Configuration
    void setRepetitionTolerance(double value);
    void setDirectionThreshold(double value);
    double suppressionLevel() const;
};
```

### VelocityCurve

**File:** `src/core/sensitivity/VelocityCurve.h`

Maps input velocity to output sensitivity.

```cpp
class VelocityCurve {
    // Curve types
    enum CurveType { Linear, Exponential, SCurve, Custom };
    
    // Apply curve to input
    double apply(double inputVelocity);
    void setCurveType(CurveType type);
};
```

---

## ADB Integration

### AdbConnector

**File:** `src/core/adb/AdbConnector.h`

Manages ADB device discovery and connection.

```cpp
class AdbConnector : public QObject {
signals:
    void deviceDiscovered(QString deviceId, QString model);
    void connectionStateChanged(bool connected);
    void errorOccurred(QString message);

public slots:
    void startDiscovery();
    void connect(QString deviceId);
    void disconnect();
    QStringList getDevices();
};
```

### AdbConnection

**File:** `src/core/adb/AdbConnection.h`

High-performance ADB command execution.

```cpp
class AdbConnection {
    bool execute(const QString& command);
    QString executeWithOutput(const QString& command);
    bool pushFile(const QString& local, const QString& remote);
    bool isConnected() const;
};
```

---

## AI Services

### GeminiClient

**File:** `src/core/ai/GeminiClient.h`

REST client for Google Gemini API.

```cpp
class GeminiClient : public QObject {
signals:
    void responseReceived(QString response);
    void errorOccurred(QString error);

public:
    void sendPrompt(const QString& prompt);
    void setApiKey(const QString& key);
    bool isConfigured() const;
};
```

### AiAdvisor

**File:** `src/core/ai/AiAdvisor.h`

Generates sensitivity recommendations.

```cpp
class AiAdvisor : public QObject {
signals:
    void recommendationReady(QVariantMap settings);
    void confidenceUpdated(double confidence);

public slots:
    void requestRecommendation(QVariantMap currentSettings);
    void setConfidenceThreshold(double threshold);
};
```

---

## Input System

### InputHook

**File:** `src/core/input/InputHook.h`

Low-level input capture using Windows Raw Input.

```cpp
class InputHook : public QObject {
signals:
    void mouseMove(int deltaX, int deltaY);
    void mouseButton(int button, bool pressed);
    
public:
    bool install();
    void uninstall();
    bool isActive() const;
};
```

### LogitechHID

**File:** `src/core/input/LogitechHID.h`

Direct HID communication with Logitech devices.

```cpp
class LogitechHID {
    bool connect();
    void disconnect();
    int getCurrentDpi();
    bool setDpi(int dpi);
    QString getDeviceName();
};
```

---

## Usage Examples

### Basic Sensitivity Adjustment

```cpp
// Get controller instance
NeoController* controller = qobject_cast<NeoController*>(
    engine.rootContext()->contextProperty("neoController")
);

// Adjust sensitivity
controller->setXMultiplier(1.5);
controller->setYMultiplier(1.2);
controller->setSmoothing(30);
controller->applySettings();
```

### QML Integration

```qml
import NeoZ 1.0

Slider {
    value: neoController.xMultiplier
    onValueChanged: neoController.xMultiplier = value
}

Button {
    text: "Apply"
    onClicked: neoController.applySettings()
}
```

---

*Generated: 2025-12-31*
