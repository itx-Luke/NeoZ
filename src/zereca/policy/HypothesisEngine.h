#ifndef ZERECA_HYPOTHESIS_ENGINE_H
#define ZERECA_HYPOTHESIS_ENGINE_H

#include "../types/ZerecaTypes.h"
#include "../arbiter/OptimizationArbiter.h"
#include "ObservationPhase.h"
#include <QObject>
#include <vector>
#include <random>

namespace Zereca {

/**
 * @brief Optimization hypothesis with proposed parameters.
 */
struct Hypothesis {
    OptimizationProposal proposal;
    float expectedGain = 0.0f;
    float confidence = 0.0f;
    int priority = 0;  ///< Higher = try first
};

/**
 * @brief Hypothesis Engine - Bayesian-style optimization (System B).
 * 
 * The engine treats optimizations as tunable parameters, not profiles.
 * 
 * Dimensions (≤5 active at once per spec):
 * - CPU affinity
 * - Priority class
 * - IO priority
 * - Timer resolution
 * - Standby purge behavior
 * 
 * Learning approach:
 * - Bayesian-style local optimization
 * - Gradient-guided parameter search
 * - Low-dimensional (≤5 params)
 * 
 * The engine generates proposals that are submitted to the Arbiter.
 * Only approved proposals are tested.
 */
class HypothesisEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hypothesisCount READ hypothesisCount NOTIFY hypothesesChanged)
    Q_PROPERTY(bool generating READ isGenerating NOTIFY generatingChanged)
    
public:
    /**
     * @brief Engine configuration.
     */
    struct Config {
        int maxActiveParameters = 5;       ///< Max dimensions to tune at once
        float explorationRate = 0.2f;      ///< Probability of exploring vs exploiting
        float minConfidence = 0.3f;        ///< Min confidence to generate proposal
        int maxHypotheses = 10;            ///< Max hypotheses to generate per cycle
    };
    
    /**
     * @brief Parameter range definition.
     */
    struct ParameterSpace {
        ChangeType type;
        QString processName;               ///< Empty for system-wide
        std::vector<uint64_t> values;      ///< Discrete values to try
        float priorGain = 0.0f;            ///< Expected gain from prior knowledge
        float priorConfidence = 0.5f;      ///< Confidence in prior
    };
    
    explicit HypothesisEngine(QObject* parent = nullptr);
    ~HypothesisEngine() override;
    
    /**
     * @brief Generate hypotheses based on baseline and context.
     * @param baseline Baseline metrics from observation phase
     * @param emulatorName Name of the detected emulator
     * @return List of generated hypotheses
     */
    std::vector<Hypothesis> generateHypotheses(const BaselineMetrics& baseline,
                                                 const QString& emulatorName);
    
    /**
     * @brief Update priors based on observed outcome.
     * Called after each trial to improve future predictions.
     */
    void updatePriors(const OptimizationProposal& proposal, 
                      Outcome outcome, 
                      float actualDelta);
    
    /**
     * @brief Get the next best hypothesis to try.
     * @return Highest priority untried hypothesis
     */
    Hypothesis nextHypothesis();
    
    /**
     * @brief Check if generating.
     */
    bool isGenerating() const { return m_generating; }
    
    /**
     * @brief Get number of pending hypotheses.
     */
    int hypothesisCount() const { return static_cast<int>(m_hypotheses.size()); }
    
    /**
     * @brief Get/set configuration.
     */
    const Config& config() const { return m_config; }
    void setConfig(const Config& config) { m_config = config; }
    
    /**
     * @brief Register a parameter space for exploration.
     */
    void registerParameter(const ParameterSpace& param);
    
    /**
     * @brief Clear all learned priors (reset).
     */
    void resetPriors();
    
signals:
    void hypothesesChanged(int count);
    void generatingChanged(bool generating);
    
    /**
     * @brief Emitted when a new hypothesis is generated.
     */
    void hypothesisGenerated(const Hypothesis& h);
    
    /**
     * @brief Emitted when priors are updated after an outcome.
     */
    void priorsUpdated();
    
private:
    void initDefaultParameters();
    float estimateGain(ChangeType type, const BaselineMetrics& baseline);
    float computeConfidence(ChangeType type, const QString& emulator);
    int computePriority(float gain, float confidence, ChangeType type);
    
    Config m_config;
    bool m_generating = false;
    
    std::vector<ParameterSpace> m_parameters;
    std::vector<Hypothesis> m_hypotheses;
    
    // Prior knowledge (updated via Bayesian updates)
    QHash<uint64_t, float> m_gainPriors;       // config hash → expected gain
    QHash<uint64_t, float> m_confidencePriors; // config hash → confidence
    QHash<uint64_t, int> m_trialCounts;        // config hash → # trials
    
    std::mt19937 m_rng;
};

} // namespace Zereca

#endif // ZERECA_HYPOTHESIS_ENGINE_H
