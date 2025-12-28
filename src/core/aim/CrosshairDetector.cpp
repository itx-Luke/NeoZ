#include "CrosshairDetector.h"
#include <QDebug>
#include <QBuffer>
#include <QColor>

namespace NeoZ {

CrosshairDetector::CrosshairDetector(QObject* parent)
    : QObject(parent)
{
    m_samplingTimer = new QTimer(this);
    connect(m_samplingTimer, &QTimer::timeout, this, &CrosshairDetector::performSample);
    
    m_screencapProcess = new QProcess(this);
    connect(m_screencapProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CrosshairDetector::onScreencapFinished);
    
    qDebug() << "[CrosshairDetector] Initialized - sampling at" << m_samplingIntervalMs << "ms";
}

CrosshairDetector::~CrosshairDetector()
{
    stopDetection();
}

void CrosshairDetector::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    
    if (enabled) {
        startDetection();
    } else {
        stopDetection();
    }
    
    emit enabledChanged();
}

void CrosshairDetector::setSamplingIntervalMs(int ms)
{
    ms = qBound(30, ms, 200);  // 30-200ms range
    if (m_samplingIntervalMs == ms) return;
    m_samplingIntervalMs = ms;
    
    if (m_samplingTimer->isActive()) {
        m_samplingTimer->setInterval(ms);
    }
    
    qDebug() << "[CrosshairDetector] Sampling interval:" << ms << "ms";
    emit settingsChanged();
}

void CrosshairDetector::setYReductionAlpha(double alpha)
{
    alpha = qBound(0.05, alpha, 0.5);  // 5-50% reduction
    if (qFuzzyCompare(m_yReductionAlpha, alpha)) return;
    m_yReductionAlpha = alpha;
    
    qDebug() << "[CrosshairDetector] Y reduction alpha:" << alpha * 100 << "%";
    emit settingsChanged();
}

void CrosshairDetector::setAdbPath(const QString& path)
{
    m_adbPath = path;
}

void CrosshairDetector::setDeviceId(const QString& deviceId)
{
    m_deviceId = deviceId;
    qDebug() << "[CrosshairDetector] Device set:" << deviceId;
}

void CrosshairDetector::startDetection()
{
    if (m_adbPath.isEmpty() || m_deviceId.isEmpty()) {
        qWarning() << "[CrosshairDetector] Cannot start - ADB path or device not set";
        emit detectionError("ADB not configured");
        return;
    }
    
    m_samplingTimer->start(m_samplingIntervalMs);
    qDebug() << "[CrosshairDetector] Detection STARTED";
}

void CrosshairDetector::stopDetection()
{
    m_samplingTimer->stop();
    
    if (m_aimAssistActive) {
        m_aimAssistActive = false;
        emit aimAssistStateChanged(false);
    }
    
    qDebug() << "[CrosshairDetector] Detection STOPPED";
}

void CrosshairDetector::performSample()
{
    if (m_sampleInProgress) return;  // Skip if previous sample still running
    if (m_adbPath.isEmpty() || m_deviceId.isEmpty()) return;
    
    m_sampleInProgress = true;
    
    // Run ADB screencap and pipe to stdout as PNG
    // adb -s <device> exec-out screencap -p
    m_screencapProcess->start(m_adbPath, {"-s", m_deviceId, "exec-out", "screencap", "-p"});
}

void CrosshairDetector::onScreencapFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_sampleInProgress = false;
    
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        return;  // Silent fail, will retry next sample
    }
    
    QByteArray imageData = m_screencapProcess->readAllStandardOutput();
    if (imageData.isEmpty()) return;
    
    QImage screenshot;
    if (!screenshot.loadFromData(imageData, "PNG")) {
        return;  // Failed to parse image
    }
    
    bool wasActive = m_aimAssistActive;
    m_aimAssistActive = analyzeImage(screenshot);
    
    if (m_aimAssistActive != wasActive) {
        qDebug() << "[CrosshairDetector] Aim assist state:" << (m_aimAssistActive ? "ACTIVE (RED)" : "INACTIVE (WHITE)");
        emit aimAssistStateChanged(m_aimAssistActive);
    }
}

bool CrosshairDetector::analyzeImage(const QImage& image)
{
    // Sample 5x5 pixels at the exact center of the screen (crosshair location)
    int centerX = image.width() / 2;
    int centerY = image.height() / 2;
    
    int redCount = 0;
    int totalSamples = 0;
    
    // Sample a 5x5 area around center
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            int x = centerX + dx;
            int y = centerY + dy;
            
            if (x >= 0 && x < image.width() && y >= 0 && y < image.height()) {
                QColor pixel = image.pixelColor(x, y);
                if (isRedColor(pixel)) {
                    redCount++;
                }
                totalSamples++;
            }
        }
    }
    
    // Consider aim assist active if > 40% of center pixels are red
    return (totalSamples > 0) && (static_cast<double>(redCount) / totalSamples > 0.4);
}

bool CrosshairDetector::isRedColor(const QColor& color) const
{
    // Free Fire crosshair red detection
    // Red hue is around 0-30 or 330-360 degrees
    // Must have decent saturation and not be too dark
    
    int h = color.hue();
    int s = color.saturation();
    int v = color.value();
    
    bool isRedHue = (h >= 0 && h <= 30) || (h >= 330 && h <= 360) || h == -1;
    bool hasSaturation = s > 100;  // Not grayish
    bool notTooDark = v > 80;      // Not too dark
    bool hasRedChannel = color.red() > 150 && color.red() > color.green() + 50;
    
    return isRedHue && hasSaturation && notTooDark && hasRedChannel;
}

} // namespace NeoZ
