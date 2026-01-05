#include "HypothesisEngine.h"
#include <QDebug>
#include <QDateTime>
#include <algorithm>

namespace Zereca {

// Forward declaration
static bool canShadowTest(ChangeType type);

HypothesisEngine::HypothesisEngine(QObject* parent)
    : QObject(parent)
    , m_rng(std::random_device{}())
{
    initDefaultParameters();
}

HypothesisEngine::~HypothesisEngine() = default;

void HypothesisEngine::initDefaultParameters()
{
    // Process Priority (5 levels)
    m_parameters.push_back({
        ChangeType::PRIORITY,
        "",  // Will be set per-emulator
        {0x00000040, 0x00008000, 0x00000020, 0x00000080, 0x00000100},  // IDLE to REALTIME
        0.03f,  // ~3% expected gain
        0.4f
    });
    
    // IO Priority (3 levels)
    m_parameters.push_back({
        ChangeType::IO_PRIORITY,
        "",
        {0, 1, 2},  // Low, Normal, High
        0.02f,
        0.3f
    });
    
    // CPU Affinity (gold cores vs all)
    m_parameters.push_back({
        ChangeType::AFFINITY,
        "",
        {0, 1},  // 0 = all cores, 1 = gold cores (P-cores)
        0.05f,
        0.5f
    });
    
    // Timer Resolution (3 settings)
    m_parameters.push_back({
        ChangeType::TIMER,
        "",  // System-wide
        {0, 1, 2},  // default, 1ms, 0.5ms
        0.04f,
        0.6f
    });
    
    // Power Plan (3 settings)
    m_parameters.push_back({
        ChangeType::POWER_PLAN,
        "",  // System-wide
        {0, 1, 2},  // power_saver, balanced, performance
        0.05f,
        0.7f
    });
}

std::vector<Hypothesis> HypothesisEngine::generateHypotheses(
    const BaselineMetrics& baseline,
    const QString& emulatorName)
{
    m_generating = true;
    emit generatingChanged(true);
    
    m_hypotheses.clear();
    
    // Analyze baseline to prioritize parameters
    bool highCpuPressure = baseline.cpuResidency > 80.0;
    bool highMemPressure = baseline.memoryPressure > 0.7;
    bool lowFps = baseline.fps < 30.0;
    bool highVariance = baseline.fpsVariance > 100.0;
    
    // Generate hypotheses for each parameter space
    for (auto& param : m_parameters) {
        // Skip if we've hit max dimensions
        if (static_cast<int>(m_hypotheses.size()) >= m_config.maxActiveParameters) {
            break;
        }
        
        // Estimate expected gain based on baseline
        float expectedGain = estimateGain(param.type, baseline);
        float confidence = computeConfidence(param.type, emulatorName);
        
        // Skip low-confidence hypotheses
        if (confidence < m_config.minConfidence) {
            continue;
        }
        
        // Exploration vs exploitation
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        bool explore = dist(m_rng) < m_config.explorationRate;
        
        // Select value to try
        uint64_t valueToTry = 0;
        if (explore || m_trialCounts.isEmpty()) {
            // Random exploration
            std::uniform_int_distribution<size_t> valueDist(0, param.values.size() - 1);
            valueToTry = param.values[valueDist(m_rng)];
        } else {
            // Exploit: use best known value
            float bestGain = -1.0f;
            for (uint64_t val : param.values) {
                uint64_t hash = static_cast<uint64_t>(param.type) ^ val;
                if (m_gainPriors.contains(hash) && m_gainPriors[hash] > bestGain) {
                    bestGain = m_gainPriors[hash];
                    valueToTry = val;
                }
            }
            if (bestGain < 0) {
                // No prior, pick first
                valueToTry = param.values[0];
            }
        }
        
        // Create hypothesis
        Hypothesis h;
        h.proposal.type = param.type;
        h.proposal.targetProcess = param.processName.isEmpty() ? emulatorName : param.processName;
        h.proposal.currentValue = 0;  // Will be read at apply time
        h.proposal.proposedValue = valueToTry;
        h.proposal.expectedGain = expectedGain;
        h.proposal.confidence = confidence;
        h.proposal.shadowTestAllowed = canShadowTest(param.type);
        
        h.expectedGain = expectedGain;
        h.confidence = confidence;
        h.priority = computePriority(expectedGain, confidence, param.type);
        
        m_hypotheses.push_back(h);
        emit hypothesisGenerated(h);
    }
    
    // Sort by priority (highest first)
    std::sort(m_hypotheses.begin(), m_hypotheses.end(),
        [](const Hypothesis& a, const Hypothesis& b) {
            return a.priority > b.priority;
        });
    
    m_generating = false;
    emit generatingChanged(false);
    emit hypothesesChanged(static_cast<int>(m_hypotheses.size()));
    
    qDebug() << "[Zereca] Generated" << m_hypotheses.size() << "hypotheses for" << emulatorName;
    
    return m_hypotheses;
}

void HypothesisEngine::updatePriors(const OptimizationProposal& proposal,
                                     Outcome outcome,
                                     float actualDelta)
{
    uint64_t hash = static_cast<uint64_t>(proposal.type) ^ proposal.proposedValue;
    
    // Get existing priors
    float oldGain = m_gainPriors.value(hash, 0.0f);
    float oldConf = m_confidencePriors.value(hash, 0.5f);
    int trials = m_trialCounts.value(hash, 0) + 1;
    
    // Bayesian update (simplified)
    // New estimate = weighted average of prior and observed
    float weight = 1.0f / trials;  // Decreasing weight for new evidence
    float newGain = oldGain * (1 - weight) + actualDelta * weight;
    
    // Confidence increases with trials, decreases on negative outcomes
    float newConf = oldConf;
    if (outcome == Outcome::POSITIVE) {
        newConf = std::min(0.95f, oldConf + 0.1f);
    } else if (outcome == Outcome::NEUTRAL) {
        newConf = std::max(0.1f, oldConf - 0.05f);
    } else {
        newConf = std::max(0.0f, oldConf - 0.3f);
    }
    
    m_gainPriors[hash] = newGain;
    m_confidencePriors[hash] = newConf;
    m_trialCounts[hash] = trials;
    
    qDebug() << "[Zereca] Updated priors for" << static_cast<int>(proposal.type)
             << "gain:" << oldGain << "->" << newGain
             << "conf:" << oldConf << "->" << newConf;
    
    emit priorsUpdated();
}

Hypothesis HypothesisEngine::nextHypothesis()
{
    if (m_hypotheses.empty()) {
        return Hypothesis();
    }
    
    Hypothesis next = m_hypotheses.front();
    m_hypotheses.erase(m_hypotheses.begin());
    emit hypothesesChanged(static_cast<int>(m_hypotheses.size()));
    
    return next;
}

void HypothesisEngine::registerParameter(const ParameterSpace& param)
{
    m_parameters.push_back(param);
}

void HypothesisEngine::resetPriors()
{
    m_gainPriors.clear();
    m_confidencePriors.clear();
    m_trialCounts.clear();
    qDebug() << "[Zereca] HypothesisEngine priors reset";
    emit priorsUpdated();
}

float HypothesisEngine::estimateGain(ChangeType type, const BaselineMetrics& baseline)
{
    // Context-aware gain estimation
    switch (type) {
        case ChangeType::PRIORITY:
            // Higher gain if CPU is contended
            return baseline.cpuResidency > 70 ? 0.05f : 0.02f;
        
        case ChangeType::AFFINITY:
            // Higher gain on hybrid CPUs with high load
            return baseline.cpuResidency > 60 ? 0.06f : 0.03f;
        
        case ChangeType::IO_PRIORITY:
            // Modest gains
            return 0.02f;
        
        case ChangeType::TIMER:
            // Higher gain if frame times are inconsistent
            return baseline.fpsVariance > 50 ? 0.05f : 0.02f;
        
        case ChangeType::POWER_PLAN:
            // Higher gain if not already on performance
            return 0.04f;
        
        default:
            return 0.01f;
    }
}

float HypothesisEngine::computeConfidence(ChangeType type, const QString& emulator)
{
    // Base confidence from parameter definition
    float base = 0.5f;
    for (const auto& param : m_parameters) {
        if (param.type == type) {
            base = param.priorConfidence;
            break;
        }
    }
    
    // Boost for well-known emulators
    if (emulator.contains("Bluestacks", Qt::CaseInsensitive)) {
        base += 0.1f;  // Well-tested
    } else if (emulator.contains("LDPlayer", Qt::CaseInsensitive)) {
        base += 0.05f;
    }
    
    return std::min(0.9f, base);
}

int HypothesisEngine::computePriority(float gain, float confidence, ChangeType type)
{
    // Priority = gain * confidence * 100, with type-based bonus
    int priority = static_cast<int>(gain * confidence * 100);
    
    // Prefer safer, lower-cooldown changes
    switch (type) {
        case ChangeType::PRIORITY:
        case ChangeType::IO_PRIORITY:
            priority += 10;  // Try these first
            break;
        case ChangeType::AFFINITY:
            priority += 5;
            break;
        default:
            break;
    }
    
    return priority;
}

// Helper function (should be in header but keeping implementation simple)
static bool canShadowTest(ChangeType type)
{
    switch (type) {
        case ChangeType::PRIORITY:
        case ChangeType::AFFINITY:
        case ChangeType::IO_PRIORITY:
            return true;  // Process-scoped, reversible
        default:
            return false; // System-wide or risky
    }
}

} // namespace Zereca
