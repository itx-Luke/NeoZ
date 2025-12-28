#ifndef NEOZ_CROSSHAIRDETECTOR_H
#define NEOZ_CROSSHAIRDETECTOR_H

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QImage>

namespace NeoZ {

/**
 * @brief Crosshair color detection for Free Fire aim assist state.
 * 
 * Uses ADB screencap to sample center screen pixels and detect:
 * - WHITE crosshair = Normal, no enemy in range
 * - RED crosshair = Free Fire aim assist active (enemy in range box)
 * 
 * When aim assist is active, emits signal to reduce Y sensitivity
 * to prevent body lock and help headshots.
 */
class CrosshairDetector : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool aimAssistActive READ aimAssistActive NOTIFY aimAssistStateChanged)
    Q_PROPERTY(int samplingIntervalMs READ samplingIntervalMs WRITE setSamplingIntervalMs NOTIFY settingsChanged)
    Q_PROPERTY(double yReductionAlpha READ yReductionAlpha WRITE setYReductionAlpha NOTIFY settingsChanged)
    
public:
    explicit CrosshairDetector(QObject* parent = nullptr);
    ~CrosshairDetector() override;
    
    // Properties
    bool enabled() const { return m_enabled; }
    bool aimAssistActive() const { return m_aimAssistActive; }
    int samplingIntervalMs() const { return m_samplingIntervalMs; }
    double yReductionAlpha() const { return m_yReductionAlpha; }
    
    // Setters
    void setEnabled(bool enabled);
    void setSamplingIntervalMs(int ms);
    void setYReductionAlpha(double alpha);
    
    // ADB device management
    void setAdbPath(const QString& path);
    void setDeviceId(const QString& deviceId);
    
    // Start/stop detection
    Q_INVOKABLE void startDetection();
    Q_INVOKABLE void stopDetection();
    
signals:
    void enabledChanged();
    void aimAssistStateChanged(bool active);
    void settingsChanged();
    void detectionError(const QString& error);
    
private slots:
    void performSample();
    void onScreencapFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
private:
    bool analyzeImage(const QImage& image);
    bool isRedColor(const QColor& color) const;
    
    // State
    bool m_enabled = false;
    bool m_aimAssistActive = false;
    
    // Settings
    int m_samplingIntervalMs = 50;   // 50ms = 20 samples/sec
    double m_yReductionAlpha = 0.2;  // 20% Y reduction when assist active
    
    // ADB
    QString m_adbPath;
    QString m_deviceId;
    QProcess* m_screencapProcess = nullptr;
    
    // Polling
    QTimer* m_samplingTimer = nullptr;
    bool m_sampleInProgress = false;
};

} // namespace NeoZ

#endif // NEOZ_CROSSHAIRDETECTOR_H
