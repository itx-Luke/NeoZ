#ifndef NEOZ_SENSITIVITYMANAGER_H
#define NEOZ_SENSITIVITYMANAGER_H

#include <QObject>
#include "../sensitivity/VelocityCurve.h"
#include "../sensitivity/DRCS.h"

namespace NeoZ {

/**
 * @brief Manages sensitivity settings and configurations.
 * 
 * Extracted from NeoController. Handles:
 * - X/Y axis multipliers
 * - Velocity curve configuration
 * - DRCS (Directional Repetition Constraint System)
 * - Slow zone / smoothing settings
 * - Snapshot/rollback for A/B testing
 */
class SensitivityManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(double xMultiplier READ xMultiplier WRITE setXMultiplier NOTIFY sensitivityChanged)
    Q_PROPERTY(double yMultiplier READ yMultiplier WRITE setYMultiplier NOTIFY sensitivityChanged)
    Q_PROPERTY(int slowZone READ slowZone WRITE setSlowZone NOTIFY sensitivityChanged)
    Q_PROPERTY(int smoothing READ smoothing WRITE setSmoothing NOTIFY sensitivityChanged)
    Q_PROPERTY(int mouseDpi READ mouseDpi WRITE setMouseDpi NOTIFY sensitivityChanged)
    Q_PROPERTY(QString curve READ curve WRITE setCurve NOTIFY curveChanged)
    Q_PROPERTY(bool hasSnapshot READ hasSnapshot NOTIFY snapshotChanged)
    
    // Velocity curve
    Q_PROPERTY(int velocityCurvePreset READ velocityCurvePreset WRITE setVelocityCurvePreset NOTIFY curveChanged)
    Q_PROPERTY(double velocityLowThreshold READ velocityLowThreshold WRITE setVelocityLowThreshold NOTIFY curveChanged)
    Q_PROPERTY(double velocityHighThreshold READ velocityHighThreshold WRITE setVelocityHighThreshold NOTIFY curveChanged)
    Q_PROPERTY(double velocityLowMultiplier READ velocityLowMultiplier WRITE setVelocityLowMultiplier NOTIFY curveChanged)
    Q_PROPERTY(double velocityHighMultiplier READ velocityHighMultiplier WRITE setVelocityHighMultiplier NOTIFY curveChanged)
    
    // DRCS
    Q_PROPERTY(bool drcsEnabled READ drcsEnabled WRITE setDrcsEnabled NOTIFY drcsChanged)
    
public:
    explicit SensitivityManager(QObject* parent = nullptr);
    ~SensitivityManager();
    
    // Basic multipliers
    double xMultiplier() const { return m_xMultiplier; }
    void setXMultiplier(double value);
    double yMultiplier() const { return m_yMultiplier; }
    void setYMultiplier(double value);
    
    // Slow zone / smoothing
    int slowZone() const { return m_slowZone; }
    void setSlowZone(int value);
    int smoothing() const { return m_smoothing; }
    void setSmoothing(int value);
    
    // Mouse DPI
    int mouseDpi() const { return m_mouseDpi; }
    void setMouseDpi(int dpi);
    
    // Curve preset
    QString curve() const { return m_curve; }
    void setCurve(const QString& curve);
    
    // Velocity curve
    VelocityCurve* velocityCurve() const { return m_velocityCurve; }
    int velocityCurvePreset() const;
    void setVelocityCurvePreset(int preset);
    double velocityLowThreshold() const;
    void setVelocityLowThreshold(double v);
    double velocityHighThreshold() const;
    void setVelocityHighThreshold(double v);
    double velocityLowMultiplier() const;
    void setVelocityLowMultiplier(double v);
    double velocityHighMultiplier() const;
    void setVelocityHighMultiplier(double v);
    
    // DRCS
    DRCS* drcs() const { return m_drcs; }
    bool drcsEnabled() const;
    void setDrcsEnabled(bool enabled);
    
    // Snapshot system for A/B testing
    Q_INVOKABLE void takeSnapshot();
    Q_INVOKABLE void rollback();
    bool hasSnapshot() const { return m_hasSnapshot; }
    
    // Batch update
    Q_INVOKABLE void setSensitivity(double x, double y, const QString& curve, 
                                    int slowZone, int smoothing);
    
    // Persistence
    void loadFromConfig();
    void saveToConfig();
    
signals:
    void sensitivityChanged();
    void curveChanged();
    void drcsChanged();
    void snapshotChanged();
    
private:
    void syncToPipeline();
    
    double m_xMultiplier = 0.0;
    double m_yMultiplier = 0.0;
    int m_slowZone = 35;
    int m_smoothing = 20;
    int m_mouseDpi = 800;
    QString m_curve = "FF_OneTap_v2";
    
    VelocityCurve* m_velocityCurve = nullptr;
    DRCS* m_drcs = nullptr;
    
    // Snapshot data
    struct Snapshot {
        double xMultiplier;
        double yMultiplier;
        int slowZone;
        int smoothing;
        int mouseDpi;
    };
    Snapshot m_snapshot;
    bool m_hasSnapshot = false;
};

} // namespace NeoZ

#endif // NEOZ_SENSITIVITYMANAGER_H
