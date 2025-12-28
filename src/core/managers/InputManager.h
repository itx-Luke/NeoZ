#ifndef NEOZ_INPUTMANAGER_H
#define NEOZ_INPUTMANAGER_H

#include <QObject>
#include "../input/InputHook.h"
#include "../sensitivity/SensitivityPipeline.h"
#include "../perf/FastConf.hpp"

namespace NeoZ {

/**
 * @brief Manages input processing and mouse hook.
 * 
 * Extracted from NeoController. Handles:
 * - Input hook lifecycle (start/stop)
 * - Input pipeline configuration
 * - Telemetry tracking (velocity, angle, latency)
 */
class InputManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(bool hookActive READ isHookActive NOTIFY hookStateChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(double mouseVelocity READ mouseVelocity NOTIFY telemetryChanged)
    Q_PROPERTY(double mouseAngleDegrees READ mouseAngleDegrees NOTIFY telemetryChanged)
    Q_PROPERTY(double latencyMs READ latencyMs NOTIFY telemetryChanged)
    
public:
    explicit InputManager(QObject* parent = nullptr);
    ~InputManager();
    
    // Hook control
    Q_INVOKABLE void startHook();
    Q_INVOKABLE void stopHook();
    Q_INVOKABLE void toggleHook();
    bool isHookActive() const;
    QString status() const { return m_status; }
    
    // Pipeline access
    SensitivityPipeline* pipeline() const;
    
    // Telemetry
    double mouseVelocity() const { return m_mouseVelocity; }
    double mouseAngleDegrees() const { return m_mouseAngleDegrees; }
    double latencyMs() const { return m_latencyMs; }
    
    // Configuration
    void setAxisMultiplierX(double value);
    void setAxisMultiplierY(double value);
    void setSmoothingMs(double ms);
    void setMouseDpi(int dpi);
    
signals:
    void hookStateChanged();
    void statusChanged();
    void telemetryChanged();
    void inputProcessed(double deltaX, double deltaY, double velocity);
    
private slots:
    void onInputProcessed(const InputState& input);
    
private:
    QString m_status = "Idle";
    double m_mouseVelocity = 0.0;
    double m_mouseAngleDegrees = 0.0;
    double m_latencyMs = 0.0;
    
    // Performance tracking
    FastConf<float, 32> m_latencyTracker;
};

} // namespace NeoZ

#endif // NEOZ_INPUTMANAGER_H
