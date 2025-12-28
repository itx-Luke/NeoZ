#include "SensitivityManager.h"
#include "../input/InputHook.h"
#include "../config/FastConfig.h"
#include <QDebug>

namespace NeoZ {

SensitivityManager::SensitivityManager(QObject* parent)
    : QObject(parent)
{
    m_velocityCurve = new VelocityCurve(this);
    m_drcs = new DRCS(this);
    
    loadFromConfig();
}

SensitivityManager::~SensitivityManager()
{
    saveToConfig();
}

void SensitivityManager::setXMultiplier(double value)
{
    value = qBound(-1.0, value, 1.0);
    if (qFuzzyCompare(m_xMultiplier, value)) return;
    
    m_xMultiplier = value;
    syncToPipeline();
    emit sensitivityChanged();
}

void SensitivityManager::setYMultiplier(double value)
{
    value = qBound(-1.0, value, 1.0);
    if (qFuzzyCompare(m_yMultiplier, value)) return;
    
    m_yMultiplier = value;
    syncToPipeline();
    emit sensitivityChanged();
}

void SensitivityManager::setSlowZone(int value)
{
    if (m_slowZone == value) return;
    m_slowZone = value;
    emit sensitivityChanged();
}

void SensitivityManager::setSmoothing(int value)
{
    if (m_smoothing == value) return;
    m_smoothing = value;
    
    if (auto* pipe = InputHookManager::instance().pipeline()) {
        pipe->setSmoothingMs(static_cast<double>(value));
    }
    
    emit sensitivityChanged();
}

void SensitivityManager::setMouseDpi(int dpi)
{
    if (m_mouseDpi == dpi) return;
    m_mouseDpi = dpi;
    
    if (auto* pipe = InputHookManager::instance().pipeline()) {
        pipe->setMouseDpi(dpi);
    }
    
    emit sensitivityChanged();
}

void SensitivityManager::setCurve(const QString& curve)
{
    if (m_curve == curve) return;
    m_curve = curve;
    emit curveChanged();
}

int SensitivityManager::velocityCurvePreset() const
{
    return m_velocityCurve ? static_cast<int>(m_velocityCurve->preset()) : 0;
}

void SensitivityManager::setVelocityCurvePreset(int preset)
{
    if (m_velocityCurve) {
        m_velocityCurve->setPreset(static_cast<VelocityCurve::CurvePreset>(preset));
        emit curveChanged();
    }
}

double SensitivityManager::velocityLowThreshold() const
{
    return m_velocityCurve ? m_velocityCurve->lowThreshold() : 0.0;
}

void SensitivityManager::setVelocityLowThreshold(double v)
{
    if (m_velocityCurve) {
        m_velocityCurve->setLowThreshold(v);
        emit curveChanged();
    }
}

double SensitivityManager::velocityHighThreshold() const
{
    return m_velocityCurve ? m_velocityCurve->highThreshold() : 0.0;
}

void SensitivityManager::setVelocityHighThreshold(double v)
{
    if (m_velocityCurve) {
        m_velocityCurve->setHighThreshold(v);
        emit curveChanged();
    }
}

double SensitivityManager::velocityLowMultiplier() const
{
    return m_velocityCurve ? m_velocityCurve->lowMultiplier() : 1.0;
}

void SensitivityManager::setVelocityLowMultiplier(double v)
{
    if (m_velocityCurve) {
        m_velocityCurve->setLowMultiplier(v);
        emit curveChanged();
    }
}

double SensitivityManager::velocityHighMultiplier() const
{
    return m_velocityCurve ? m_velocityCurve->highMultiplier() : 1.0;
}

void SensitivityManager::setVelocityHighMultiplier(double v)
{
    if (m_velocityCurve) {
        m_velocityCurve->setHighMultiplier(v);
        emit curveChanged();
    }
}

bool SensitivityManager::drcsEnabled() const
{
    return m_drcs ? m_drcs->isEnabled() : false;
}

void SensitivityManager::setDrcsEnabled(bool enabled)
{
    if (m_drcs) {
        m_drcs->setEnabled(enabled);
        emit drcsChanged();
    }
}

void SensitivityManager::takeSnapshot()
{
    m_snapshot.xMultiplier = m_xMultiplier;
    m_snapshot.yMultiplier = m_yMultiplier;
    m_snapshot.slowZone = m_slowZone;
    m_snapshot.smoothing = m_smoothing;
    m_snapshot.mouseDpi = m_mouseDpi;
    m_hasSnapshot = true;
    emit snapshotChanged();
    qDebug() << "[SensitivityManager] Snapshot taken";
}

void SensitivityManager::rollback()
{
    if (!m_hasSnapshot) return;
    
    m_xMultiplier = m_snapshot.xMultiplier;
    m_yMultiplier = m_snapshot.yMultiplier;
    m_slowZone = m_snapshot.slowZone;
    m_smoothing = m_snapshot.smoothing;
    m_mouseDpi = m_snapshot.mouseDpi;
    
    syncToPipeline();
    emit sensitivityChanged();
    qDebug() << "[SensitivityManager] Rollback complete";
}

void SensitivityManager::setSensitivity(double x, double y, const QString& curve, 
                                        int slowZone, int smoothing)
{
    m_xMultiplier = x;
    m_yMultiplier = y;
    m_curve = curve;
    m_slowZone = slowZone;
    m_smoothing = smoothing;
    
    syncToPipeline();
    emit sensitivityChanged();
    emit curveChanged();
}

void SensitivityManager::syncToPipeline()
{
    if (auto* pipe = InputHookManager::instance().pipeline()) {
        pipe->setAxisMultiplierX(m_xMultiplier);
        pipe->setAxisMultiplierY(m_yMultiplier);
        pipe->setSmoothingMs(static_cast<double>(m_smoothing));
        pipe->setMouseDpi(m_mouseDpi);
    }
}

void SensitivityManager::loadFromConfig()
{
    if (auto* config = globalConfig()) {
        m_xMultiplier = config->get<double>("sensitivity/x", 0.0);
        m_yMultiplier = config->get<double>("sensitivity/y", 0.0);
        m_slowZone = config->get<int>("sensitivity/slowZone", 35);
        m_smoothing = config->get<int>("sensitivity/smoothing", 20);
        m_mouseDpi = config->get<int>("sensitivity/dpi", 800);
        m_curve = config->get<QString>("sensitivity/curve", "FF_OneTap_v2");
    }
}

void SensitivityManager::saveToConfig()
{
    if (auto* config = globalConfig()) {
        config->set("sensitivity/x", m_xMultiplier);
        config->set("sensitivity/y", m_yMultiplier);
        config->set("sensitivity/slowZone", m_slowZone);
        config->set("sensitivity/smoothing", m_smoothing);
        config->set("sensitivity/dpi", m_mouseDpi);
        config->set("sensitivity/curve", m_curve);
    }
}

} // namespace NeoZ
