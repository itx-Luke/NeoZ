#ifndef ZERECA_OPTIMIZATION_ARBITER_H
#define ZERECA_OPTIMIZATION_ARBITER_H

#include "../types/ZerecaTypes.h"
#include "../core/FlightRecorder.h"
#include "../core/TelemetryReader.h"  // for PrivilegeTier
#include <QObject>
#include <QHash>

namespace Zereca {

// Forward declarations
class ProbationLedger;
class HysteresisRules;

/**
 * @brief Optimization Arbiter - The central safety gate (System C).
 * 
 * The Arbiter decides IF a proposed change is allowed RIGHT NOW.
 * 
 * It prevents:
 * - Oscillation (rapid back-and-forth changes)
 * - Over-aggressive learning
 * - Unsafe transitions
 * 
 * Hard rules locked in:
 * - Emulator confidence must be >= 0.75
 * - Probation entries block repeat failures
 * - Cooldown periods per change type
 */
class OptimizationArbiter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pendingProposals READ pendingProposals NOTIFY proposalQueueChanged)
    Q_PROPERTY(int rejectedCount READ rejectedCount NOTIFY statsChanged)
    Q_PROPERTY(int approvedCount READ approvedCount NOTIFY statsChanged)
    
public:
    /**
     * @brief Rejection reason codes.
     */
    enum class RejectionReason {
        None,
        LowEmulatorConfidence,  ///< Emulator detection < 0.75
        OnProbation,            ///< Configuration previously failed
        CooldownActive,         ///< Too soon since last change of this type
        InsufficientConfidence, ///< Proposal confidence too low
        PrivilegeRequired,      ///< Needs Operator mode
        UnsafeChange,           ///< Change type not allowed in current state
        RollbackActive          ///< System is in rollback state
    };
    Q_ENUM(RejectionReason)
    
    /**
     * @brief Decision result from the Arbiter.
     */
    struct Decision {
        bool approved = false;
        RejectionReason reason = RejectionReason::None;
        QString explanation;
        uint64_t cooldownRemainingMs = 0;
    };
    
    explicit OptimizationArbiter(ProbationLedger* ledger,
                                  FlightRecorder* recorder,
                                  QObject* parent = nullptr);
    ~OptimizationArbiter() override;
    
    /**
     * @brief Evaluate a proposal for approval.
     * @param proposal The optimization proposal from System B
     * @param emulatorConfidence Detection confidence (0.0–1.0)
     * @return Decision with approval status and reason
     */
    Decision evaluate(const OptimizationProposal& proposal, float emulatorConfidence);
    
    /**
     * @brief Record the outcome of an approved optimization.
     * @param proposal The original proposal
     * @param outcome The observed outcome
     * @param metrics Performance delta metrics
     */
    void recordOutcome(const OptimizationProposal& proposal, 
                       Outcome outcome,
                       float actualDelta);
    
    /**
     * @brief Set whether the system is in rollback state.
     */
    void setRollbackActive(bool active) { m_rollbackActive = active; }
    
    /**
     * @brief Set the current privilege tier.
     */
    void setPrivilegeTier(PrivilegeTier tier) { m_privilegeTier = tier; }
    
    // Stats
    int pendingProposals() const { return m_pendingCount; }
    int rejectedCount() const { return m_rejectedCount; }
    int approvedCount() const { return m_approvedCount; }
    
    /**
     * @brief Minimum emulator confidence threshold (LOCKED).
     */
    static constexpr float MIN_EMULATOR_CONFIDENCE = 0.75f;
    
signals:
    void proposalApproved(const OptimizationProposal& proposal);
    void proposalRejected(const OptimizationProposal& proposal, RejectionReason reason);
    void proposalQueueChanged(int count);
    void statsChanged();
    
private:
    bool checkCooldown(ChangeType type, uint64_t& remainingMs) const;
    void updateCooldown(ChangeType type);
    uint64_t getCooldownDuration(ChangeType type) const;
    bool requiresOperatorMode(ChangeType type) const;
    
    ProbationLedger* m_probationLedger = nullptr;
    FlightRecorder* m_flightRecorder = nullptr;
    
    // Cooldown tracking (change type → last applied timestamp)
    QHash<ChangeType, uint64_t> m_lastApplied;
    
    // State
    bool m_rollbackActive = false;
    PrivilegeTier m_privilegeTier = PrivilegeTier::Standard;
    
    // Stats
    int m_pendingCount = 0;
    int m_rejectedCount = 0;
    int m_approvedCount = 0;
};

} // namespace Zereca

#endif // ZERECA_OPTIMIZATION_ARBITER_H
