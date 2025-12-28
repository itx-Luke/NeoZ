/**
 * DRCS.cpp - Directional Repetition Constraint System Implementation
 * 
 * Core algorithm for intelligent over-drag prevention.
 */

#include "DRCS.h"
#include <QDebug>
#include <numeric>
#include <algorithm>

DRCS::DRCS(QObject *parent)
    : QObject(parent)
{
    m_motionBuffer.reserve(BUFFER_SIZE);
    initTimeDecayWeights();
    qDebug() << "[DRCS] Initialized - Directional Repetition Constraint System";
}

void DRCS::initTimeDecayWeights()
{
    m_timeDecayWeights.resize(BUFFER_SIZE);
    // Exponential decay: recent inputs have higher weight
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        // Weight decreases as we go back in time
        // w[0] = 1.0 (most recent), w[n] decays exponentially
        m_timeDecayWeights[i] = std::exp(-0.3 * static_cast<double>(i));
    }
}

double DRCS::processInput(double dx, double dy)
{
    if (!m_enabled) {
        m_currentSuppression = 1.0;
        m_repetitionScore = 0.0;
        return 1.0;
    }
    
    // Skip tiny movements
    double magnitude = std::sqrt(dx * dx + dy * dy);
    if (magnitude < 0.5) {
        return m_currentSuppression;
    }
    
    // Create motion vector
    MotionVector current;
    current.dx = dx;
    current.dy = dy;
    current.timestamp = std::chrono::steady_clock::now();
    current.normalize();
    
    // Check for direction change that should reset
    if (!m_motionBuffer.empty()) {
        double similarity = calculateCosineSimilarity(current, m_motionBuffer.back());
        if (similarity < m_resetSensitivity) {
            // Significant direction change - partial reset
            double resetFactor = (m_resetSensitivity - similarity) / m_resetSensitivity;
            m_repetitionScore *= (1.0 - resetFactor * 0.5);
        }
    }
    
    // Add to buffer
    m_motionBuffer.push_back(current);
    if (m_motionBuffer.size() > BUFFER_SIZE) {
        m_motionBuffer.erase(m_motionBuffer.begin());
    }
    
    // Calculate repetition score
    m_repetitionScore = calculateRepetitionScore();
    
    // Check for micro-variance bypass
    if (hasMicroVariance() || hasAngularJitter()) {
        // Human-like variance detected - reduce suppression
        m_repetitionScore *= 0.5;
    }
    
    // Calculate suppression factor
    m_currentSuppression = calculateSuppressionFactor(m_repetitionScore);
    
    emit suppressionChanged();
    
    return m_currentSuppression;
}

void DRCS::applyToInput(double& dx, double& dy)
{
    double suppression = processInput(dx, dy);
    dx *= suppression;
    dy *= suppression;
}

void DRCS::reset()
{
    m_motionBuffer.clear();
    m_repetitionScore = 0.0;
    m_currentSuppression = 1.0;
    emit suppressionChanged();
    qDebug() << "[DRCS] Reset";
}

double DRCS::calculateCosineSimilarity(const MotionVector& a, const MotionVector& b)
{
    // Dot product of normalized direction vectors
    // Result is in range [-1, 1], where 1 = same direction
    return a.dirX * b.dirX + a.dirY * b.dirY;
}

double DRCS::calculateRepetitionScore()
{
    if (m_motionBuffer.size() < 2) {
        return 0.0;
    }
    
    const MotionVector& current = m_motionBuffer.back();
    double score = 0.0;
    
    // Compare current drag to all previous drags in buffer
    for (size_t i = 0; i < m_motionBuffer.size() - 1; ++i) {
        size_t bufferIndex = m_motionBuffer.size() - 2 - i;  // Start from most recent
        const MotionVector& previous = m_motionBuffer[bufferIndex];
        
        double similarity = calculateCosineSimilarity(current, previous);
        
        // If similarity exceeds threshold, count as repetition
        if (similarity >= m_directionThreshold) {
            // Apply time-decay weight
            double weight = (i < m_timeDecayWeights.size()) ? m_timeDecayWeights[i] : 0.1;
            score += weight;
        }
    }
    
    return score;
}

double DRCS::calculateSuppressionFactor(double repetitionScore)
{
    // Sigmoid suppression: λ(R) = 1 / (1 + e^(a*(R - R₀)))
    // When R < R₀: factor ~1.0 (no suppression)
    // When R > R₀: factor decreases smoothly toward 0
    
    double exponent = m_suppressionSteepness * (repetitionScore - m_repetitionTolerance);
    double factor = 1.0 / (1.0 + std::exp(exponent));
    
    // Clamp to reasonable range - never fully suppress
    return std::max(0.15, std::min(1.0, factor));
}

bool DRCS::hasMicroVariance()
{
    if (m_motionBuffer.size() < 3) {
        return false;
    }
    
    // Calculate variance in magnitude over recent inputs
    double sum = 0.0;
    double sumSq = 0.0;
    size_t count = std::min(m_motionBuffer.size(), static_cast<size_t>(5));
    
    for (size_t i = m_motionBuffer.size() - count; i < m_motionBuffer.size(); ++i) {
        double mag = m_motionBuffer[i].magnitude;
        sum += mag;
        sumSq += mag * mag;
    }
    
    double mean = sum / count;
    double variance = (sumSq / count) - (mean * mean);
    
    // Normalize variance by mean to get coefficient of variation
    double cv = (mean > 0.01) ? std::sqrt(variance) / mean : 0.0;
    
    return cv >= m_varianceThreshold;
}

bool DRCS::hasAngularJitter()
{
    if (m_motionBuffer.size() < 3) {
        return false;
    }
    
    // Check if there's small but consistent angular variance
    // (similarity < 1.0 but > threshold)
    size_t count = std::min(m_motionBuffer.size() - 1, static_cast<size_t>(5));
    double avgSimilarity = 0.0;
    
    for (size_t i = 1; i <= count; ++i) {
        size_t idx = m_motionBuffer.size() - 1 - i;
        double sim = calculateCosineSimilarity(m_motionBuffer.back(), m_motionBuffer[idx]);
        avgSimilarity += sim;
    }
    avgSimilarity /= count;
    
    // Jitter: similar enough to be "same direction" but not perfectly identical
    return avgSimilarity >= m_directionThreshold && avgSimilarity < 0.99;
}

// Setters
void DRCS::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        if (!enabled) {
            reset();
        }
        emit enabledChanged();
        qDebug() << "[DRCS] Enabled:" << enabled;
    }
}

void DRCS::setRepetitionTolerance(double value)
{
    m_repetitionTolerance = std::max(1.0, std::min(10.0, value));
    emit parametersChanged();
}

void DRCS::setDirectionThreshold(double value)
{
    m_directionThreshold = std::max(0.8, std::min(0.99, value));
    emit parametersChanged();
}

void DRCS::setSuppressionSteepness(double value)
{
    m_suppressionSteepness = std::max(0.5, std::min(5.0, value));
    emit parametersChanged();
}

void DRCS::setResetSensitivity(double value)
{
    m_resetSensitivity = std::max(0.5, std::min(0.95, value));
    emit parametersChanged();
}
