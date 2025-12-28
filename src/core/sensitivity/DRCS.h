/**
 * DRCS.h - Directional Repetition Constraint System
 * 
 * Intelligent over-drag prevention that discriminates between
 * legitimate repeated micro-drags and harmful repetitive patterns
 * that break aim assist engagement.
 */

#ifndef DRCS_H
#define DRCS_H

#include <QObject>
#include <vector>
#include <cmath>
#include <chrono>

struct MotionVector {
    double dx = 0.0;
    double dy = 0.0;
    double magnitude = 0.0;
    double dirX = 0.0;  // Normalized direction X
    double dirY = 0.0;  // Normalized direction Y
    std::chrono::steady_clock::time_point timestamp;
    
    void normalize() {
        magnitude = std::sqrt(dx * dx + dy * dy);
        if (magnitude > 0.0001) {
            dirX = dx / magnitude;
            dirY = dy / magnitude;
        } else {
            dirX = 0.0;
            dirY = 0.0;
        }
    }
};

class DRCS : public QObject
{
    Q_OBJECT
    
    // Configurable parameters exposed to QML
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(double repetitionTolerance READ repetitionTolerance WRITE setRepetitionTolerance NOTIFY parametersChanged)
    Q_PROPERTY(double directionThreshold READ directionThreshold WRITE setDirectionThreshold NOTIFY parametersChanged)
    Q_PROPERTY(double suppressionSteepness READ suppressionSteepness WRITE setSuppressionSteepness NOTIFY parametersChanged)
    Q_PROPERTY(double resetSensitivity READ resetSensitivity WRITE setResetSensitivity NOTIFY parametersChanged)
    Q_PROPERTY(double currentSuppression READ currentSuppression NOTIFY suppressionChanged)
    Q_PROPERTY(double repetitionScore READ repetitionScore NOTIFY suppressionChanged)

public:
    explicit DRCS(QObject *parent = nullptr);
    
    // Main processing function - call this for each input frame
    // Returns the suppression factor [0.0 - 1.0] to multiply with input
    double processInput(double dx, double dy);
    
    // Apply DRCS to input and return modified values
    void applyToInput(double& dx, double& dy);
    
    // Reset the system (e.g., on direction change or timeout)
    Q_INVOKABLE void reset();
    
    // Getters
    bool isEnabled() const { return m_enabled; }
    double repetitionTolerance() const { return m_repetitionTolerance; }
    double directionThreshold() const { return m_directionThreshold; }
    double suppressionSteepness() const { return m_suppressionSteepness; }
    double resetSensitivity() const { return m_resetSensitivity; }
    double currentSuppression() const { return m_currentSuppression; }
    double repetitionScore() const { return m_repetitionScore; }
    
    // Setters
    void setEnabled(bool enabled);
    void setRepetitionTolerance(double value);
    void setDirectionThreshold(double value);
    void setSuppressionSteepness(double value);
    void setResetSensitivity(double value);

signals:
    void enabledChanged();
    void parametersChanged();
    void suppressionChanged();

private:
    // Core algorithm functions
    double calculateCosineSimilarity(const MotionVector& a, const MotionVector& b);
    double calculateRepetitionScore();
    double calculateSuppressionFactor(double repetitionScore);
    bool hasMicroVariance();
    bool hasAngularJitter();
    
    // Motion buffer
    std::vector<MotionVector> m_motionBuffer;
    static constexpr size_t BUFFER_SIZE = 20;
    
    // Parameters
    bool m_enabled = false;
    double m_repetitionTolerance = 4.0;      // R₀: drags before suppression
    double m_directionThreshold = 0.95;       // θ_d: cosine similarity threshold
    double m_suppressionSteepness = 2.0;      // a: sigmoid steepness
    double m_resetSensitivity = 0.8;          // How fast direction change resets
    double m_varianceThreshold = 0.05;        // ε_m: micro-variance threshold
    
    // Current state
    double m_currentSuppression = 1.0;
    double m_repetitionScore = 0.0;
    
    // Time decay weights (more recent = higher weight)
    std::vector<double> m_timeDecayWeights;
    void initTimeDecayWeights();
};

#endif // DRCS_H
