#ifndef ZERECA_OUTCOME_CLASSIFIER_H
#define ZERECA_OUTCOME_CLASSIFIER_H

#include "../types/ZerecaTypes.h"
#include <QObject>

namespace Zereca {

/**
 * @brief Outcome Classifier - Classifies trial results.
 * 
 * Every optimization trial ends in exactly one state:
 * 
 * 🟢 POSITIVE: Improvement above threshold → COMMIT
 * 🟡 NEUTRAL: Delta below threshold → REVERT + STATE_INEFFECTIVE
 * 🟠 NEGATIVE_STABILITY: App crash/stutter → ROLLBACK + Severity 2
 * 🔴 NEGATIVE_SAFETY: BSOD/thermal → HARD STOP + Severity 3
 * 
 * The classifier uses hysteresis - a change must show sustained
 * improvement above threshold with sufficient confidence.
 */
class OutcomeClassifier : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief Classification thresholds.
     */
    struct Thresholds {
        float positiveMinDelta = 0.05f;      ///< 5% improvement required
        float positiveSustainedMs = 10000;   ///< 10 seconds sustained
        float confidenceRequired = 0.7f;      ///< 70% confidence required
        float negativeMaxRegression = -0.10f; ///< -10% triggers negative
    };
    
    /**
     * @brief Classification result.
     */
    struct Result {
        Outcome outcome;
        float delta;              ///< Actual performance delta
        float confidence;         ///< Confidence in the classification
        QString reason;           ///< Human-readable explanation
        bool shouldCommit = false;
        bool shouldRevert = false;
        bool shouldRollback = false;
        Severity probationSeverity = Severity::NONE;
    };
    
    explicit OutcomeClassifier(QObject* parent = nullptr);
    ~OutcomeClassifier() override = default;
    
    /**
     * @brief Classify the outcome of an optimization trial.
     * @param baselineMetrics Metrics before optimization
     * @param currentMetrics Metrics after optimization
     * @param hadCrash Whether the target app crashed
     * @param hadThermalEvent Whether thermal throttling occurred
     * @return Classification result
     */
    Result classify(const BaselineMetrics& baseline,
                    const BaselineMetrics& current,
                    bool hadCrash = false,
                    bool hadThermalEvent = false);
    
    /**
     * @brief Get current thresholds.
     */
    const Thresholds& thresholds() const { return m_thresholds; }
    
    /**
     * @brief Set classification thresholds.
     */
    void setThresholds(const Thresholds& thresholds) { m_thresholds = thresholds; }
    
signals:
    /**
     * @brief Emitted when a classification is made.
     */
    void classified(Outcome outcome, float delta);
    
private:
    float calculatePerformanceDelta(const BaselineMetrics& baseline,
                                    const BaselineMetrics& current);
    
    Thresholds m_thresholds;
};

} // namespace Zereca

#endif // ZERECA_OUTCOME_CLASSIFIER_H
