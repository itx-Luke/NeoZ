#ifndef ZERECA_TYPES_H
#define ZERECA_TYPES_H

#include <cstdint>
#include <QString>
#include <QHash>
#include <QJsonObject>

namespace Zereca {

// ============================================================================
// OUTCOME CLASSIFICATION (Section 6 of Spec)
// ============================================================================

/**
 * @brief Trial outcome classification.
 * Every optimization trial ends in exactly one of these states.
 */
enum class Outcome : uint8_t {
    POSITIVE,   ///< 🟢 Improvement above threshold → COMMIT
    NEUTRAL,    ///< 🟡 Delta below threshold → REVERT + STATE_INEFFECTIVE
    NEGATIVE_STABILITY,  ///< 🟠 App crash/stutter → ROLLBACK + Severity 2
    NEGATIVE_SAFETY      ///< 🔴 BSOD/thermal → HARD STOP + Severity 3
};

/**
 * @brief Severity levels for probation entries.
 */
enum class Severity : uint8_t {
    NONE = 0,
    LOW = 1,         ///< FPS regression (auto-retry allowed)
    MEDIUM = 2,      ///< App crash (context-shift retry only)
    CRITICAL = 3     ///< BSOD (never auto-retry, manual reset only)
};

/**
 * @brief Change type classification for Arbiter decisions.
 */
enum class ChangeType : uint8_t {
    PRIORITY,       ///< Process priority class (low cooldown)
    AFFINITY,       ///< CPU affinity (medium cooldown)
    IO_PRIORITY,    ///< I/O priority (low cooldown)
    TIMER,          ///< Timer resolution (high cooldown)
    POWER_PLAN,     ///< Power plan switch (high cooldown)
    HPET            ///< HPET disable (extreme cooldown, no shadow)
};

// ============================================================================
// PROBATION LEDGER (Section 7 of Spec)
// ============================================================================

/**
 * @brief Entry in the probation ledger tracking failed configurations.
 */
struct ProbationEntry {
    uint64_t configHash = 0;       ///< Hash of the optimization config
    uint64_t lastFailureTs = 0;    ///< Timestamp of last failure (ms since epoch)
    Severity severity = Severity::NONE;
    uint64_t driverVersion = 0;    ///< GPU driver version at failure
    uint64_t osBuild = 0;          ///< Windows OS build at failure
    float backoff = 1.0f;          ///< Exponential backoff multiplier
};

// ============================================================================
// FLIGHT DATA RECORDER (Section 9 of Spec)
// ============================================================================

/**
 * @brief Record of a state change for audit purposes.
 */
struct StateChangeRecord {
    uint64_t timestamp = 0;        ///< When the change occurred
    uint32_t component = 0;        ///< Which component (encoded as enum)
    uint64_t oldVal = 0;           ///< Previous value
    uint64_t newVal = 0;           ///< New value
    float expectedGain = 0.0f;     ///< Expected performance gain
    float actualDelta = 0.0f;      ///< Actual measured delta
    uint8_t rollbackReason = 0;    ///< Reason for rollback (0 = none)
};

// ============================================================================
// TARGET STATE DOCUMENT (Section 3.1 of Spec)
// ============================================================================

/**
 * @brief Process-specific affinity configuration.
 */
struct ProcessAffinity {
    QString processName;           ///< e.g., "HD-Player.exe"
    QString coreGroup;             ///< e.g., "gold_cores", "all", or bitmask
};

/**
 * @brief The Target State Document (TSD) - desired system state.
 * System A reconciles actual OS state to match this document.
 */
struct TargetState {
    QString powerMode = "balanced";          ///< "performance", "balanced", "power_saver"
    QString timerResolution = "default";     ///< "default", "1ms", "0.5ms"
    bool cpuParking = true;                  ///< CPU core parking enabled
    QString standbyPurge = "off";            ///< "off", "conditional", "aggressive"
    QHash<QString, QString> processAffinity; ///< Process name → core group
    
    /**
     * @brief Serialize to JSON for persistence.
     */
    QJsonObject toJson() const;
    
    /**
     * @brief Deserialize from JSON.
     */
    static TargetState fromJson(const QJsonObject& json);
    
    /**
     * @brief Generate a hash for comparison/caching.
     */
    uint64_t hash() const;
};

// ============================================================================
// OPTIMIZATION PROPOSAL (System B → System C)
// ============================================================================

/**
 * @brief A proposed optimization from the Hypothesis Engine.
 */
struct OptimizationProposal {
    ChangeType type;
    QString targetProcess;          ///< Empty for system-wide changes
    uint64_t currentValue = 0;
    uint64_t proposedValue = 0;
    float expectedGain = 0.0f;      ///< Expected performance improvement (0.0–1.0)
    float confidence = 0.0f;        ///< Confidence in the prediction (0.0–1.0)
    bool shadowTestAllowed = false; ///< Can this be A/B tested?
};

// ============================================================================
// BASELINE METRICS (System B Observation Phase)
// ============================================================================

/**
 * @brief Baseline metrics collected during observation phase.
 */
struct BaselineMetrics {
    double fps = 0.0;               ///< Average FPS during observation
    double fpsVariance = 0.0;
    double avgFrameTime = 0.0;      ///< Milliseconds
    double cpuResidency = 0.0;      ///< % time in high-perf state
    double gpuQueueDepth = 0.0;
    double memoryPressure = 0.0;    ///< 0.0–1.0
    double thermalHeadroom = 0.0;   ///< Degrees below throttle
    uint64_t observationDurationMs = 0;
};

// ============================================================================
// CONTEXT HASH (Section 7.2 of Spec)
// ============================================================================

/**
 * @brief System context for probation scoping.
 * Failures are only valid within their original context.
 */
struct SystemContext {
    uint64_t gpuDriverVersion = 0;
    uint64_t osBuild = 0;
    uint64_t biosVersion = 0;
    uint64_t emulatorBinaryHash = 0;
    
    /**
     * @brief Generate context hash for comparison.
     */
    uint64_t hash() const;
    
    /**
     * @brief Check if context has shifted from another.
     */
    bool hasShiftedFrom(const SystemContext& other) const;
};

} // namespace Zereca

#endif // ZERECA_TYPES_H
