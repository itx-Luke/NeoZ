#pragma once
/**
 * @file RealTimeSensitivityAI.hpp
 * @brief Real-time sensitivity adjustment based on shot results
 * 
 * Uses FastConf for ultra-low-latency hit/headshot tracking.
 * Adjusts sensitivity parameters on-the-fly based on player performance.
 */

#include "FastConf.hpp"
#include <QObject>
#include <functional>

namespace NeoZ {

/**
 * @brief Real-time sensitivity adjustment based on shot accuracy
 * 
 * This class monitors shot results and adjusts sensitivity parameters
 * in real-time to optimize player performance.
 */
class RealTimeSensitivityAI : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(float hitRate READ hitRate NOTIFY metricsChanged)
    Q_PROPERTY(float headshotRate READ headshotRate NOTIFY metricsChanged)
    Q_PROPERTY(float confidenceScore READ confidenceScore NOTIFY metricsChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit RealTimeSensitivityAI(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
    
    // ========== SHOT REGISTRATION ==========
    
    /**
     * @brief Register a shot result
     * @param hit Whether the shot hit
     * @param headshot Whether it was a headshot
     * @param damage Damage dealt (for weighting)
     */
    void processShotResult(bool hit, bool headshot = false, float damage = 0.0f)
    {
        Q_UNUSED(damage) // Can be used for weighted tracking later
        
        m_hitTracker.add(hit);
        if (hit) {
            m_headshotTracker.add(headshot);
        }
        
        ++m_totalShots;
        
        // Check if we should adjust
        if (m_enabled && m_hitTracker.hasMinSamples() && m_adjustCallback) {
            evaluateAndAdjust();
        }
        
        emit metricsChanged();
    }
    
    // ========== METRICS ==========
    
    float hitRate() const { return m_hitTracker.confidence(); }
    float headshotRate() const { return m_headshotTracker.confidence(); }
    float confidenceScore() const { return m_hitTracker.hasMinSamples() ? hitRate() : 0.0f; }
    
    size_t totalShots() const { return m_totalShots; }
    
    // ========== CONTROL ==========
    
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { 
        if (m_enabled != enabled) {
            m_enabled = enabled; 
            emit enabledChanged();
        }
    }
    
    void reset()
    {
        m_hitTracker.reset();
        m_headshotTracker.reset();
        m_totalShots = 0;
        m_lastAdjustShot = 0;
        emit metricsChanged();
    }
    
    // ========== CALLBACKS ==========
    
    using AdjustCallback = std::function<void(float xDelta, float yDelta, int slowZoneDelta)>;
    
    void setAdjustCallback(AdjustCallback callback)
    {
        m_adjustCallback = std::move(callback);
    }
    
    // ========== CONFIGURATION ==========
    
    struct Config {
        float minHitRateForAdjust = 0.6f;      // Minimum hit rate before adjusting
        float targetHeadshotRate = 0.3f;       // Target headshot percentage
        float adjustmentStep = 0.02f;          // How much to adjust per cycle
        int adjustIntervalShots = 20;          // Shots between adjustments
        int slowZoneStep = 3;                  // Slow zone adjustment increment
    };
    
    void setConfig(const Config& config) { m_config = config; }
    const Config& config() const { return m_config; }
    
signals:
    void metricsChanged();
    void enabledChanged();
    void adjusted(float xDelta, float yDelta, int slowZoneDelta);

private:
    void evaluateAndAdjust()
    {
        // Only adjust every N shots
        if (m_totalShots - m_lastAdjustShot < static_cast<size_t>(m_config.adjustIntervalShots)) {
            return;
        }
        
        m_lastAdjustShot = m_totalShots;
        
        float xDelta = 0.0f;
        float yDelta = 0.0f;
        int slowZoneDelta = 0;
        
        const float hr = hitRate();
        const float hsr = headshotRate();
        
        // Logic: 
        // - Low hit rate -> increase slow zone (more aim assist friendly)
        // - High hit rate but low headshot -> reduce Y sensitivity
        // - Very high hit rate -> can reduce slow zone for faster flicks
        
        if (hr < m_config.minHitRateForAdjust) {
            // Struggling to hit - increase slow zone
            slowZoneDelta = m_config.slowZoneStep;
        } else if (hr > 0.8f && hsr < m_config.targetHeadshotRate) {
            // Hitting body but not head - reduce Y for precision
            yDelta = -m_config.adjustmentStep;
        } else if (hr > 0.85f && hsr > m_config.targetHeadshotRate) {
            // Performing well - can loosen slow zone
            slowZoneDelta = -m_config.slowZoneStep / 2;
        }
        
        // Apply adjustment via callback
        if (xDelta != 0.0f || yDelta != 0.0f || slowZoneDelta != 0) {
            m_adjustCallback(xDelta, yDelta, slowZoneDelta);
            emit adjusted(xDelta, yDelta, slowZoneDelta);
        }
    }
    
    FastConf<float, 64> m_hitTracker;       // 64-shot hit rate
    FastConf<float, 64> m_headshotTracker;  // 64-shot headshot rate
    
    size_t m_totalShots = 0;
    size_t m_lastAdjustShot = 0;
    bool m_enabled = false;
    
    Config m_config;
    AdjustCallback m_adjustCallback;
};

} // namespace NeoZ
