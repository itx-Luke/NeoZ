#include "OptimizationArbiter.h"
#include "ProbationLedger.h"
#include "../types/ContextHash.h"
#include <QDateTime>
#include <QDebug>

namespace Zereca {

OptimizationArbiter::OptimizationArbiter(ProbationLedger* ledger,
                                           FlightRecorder* recorder,
                                           QObject* parent)
    : QObject(parent)
    , m_probationLedger(ledger)
    , m_flightRecorder(recorder)
{
}

OptimizationArbiter::~OptimizationArbiter() = default;

OptimizationArbiter::Decision OptimizationArbiter::evaluate(
    const OptimizationProposal& proposal, 
    float emulatorConfidence)
{
    Decision decision;
    
    // ===== RULE 1: Rollback state blocks all proposals =====
    if (m_rollbackActive) {
        decision.approved = false;
        decision.reason = RejectionReason::RollbackActive;
        decision.explanation = "System is in rollback state. Acknowledge rollback first.";
        m_rejectedCount++;
        emit proposalRejected(proposal, decision.reason);
        return decision;
    }
    
    // ===== RULE 2: Emulator confidence gate (LOCKED at 0.75) =====
    if (emulatorConfidence < MIN_EMULATOR_CONFIDENCE) {
        decision.approved = false;
        decision.reason = RejectionReason::LowEmulatorConfidence;
        decision.explanation = QString("Emulator confidence %1 < required %2")
                                .arg(emulatorConfidence, 0, 'f', 2)
                                .arg(MIN_EMULATOR_CONFIDENCE, 0, 'f', 2);
        m_rejectedCount++;
        emit proposalRejected(proposal, decision.reason);
        qDebug() << "[Zereca] Rejected: low emulator certainty";
        return decision;
    }
    
    // ===== RULE 3: Check probation ledger =====
    if (m_probationLedger) {
        // Generate config hash from proposal
        uint64_t configHash = proposal.currentValue ^ proposal.proposedValue ^ 
                              static_cast<uint64_t>(proposal.type);
        
        SystemContext currentContext = ContextHash::capture();
        
        if (m_probationLedger->isOnProbation(configHash, currentContext)) {
            decision.approved = false;
            decision.reason = RejectionReason::OnProbation;
            decision.explanation = "Configuration previously failed under similar context.";
            m_rejectedCount++;
            emit proposalRejected(proposal, decision.reason);
            return decision;
        }
    }
    
    // ===== RULE 4: Check privilege requirements =====
    if (requiresOperatorMode(proposal.type) && m_privilegeTier == PrivilegeTier::Standard) {
        decision.approved = false;
        decision.reason = RejectionReason::PrivilegeRequired;
        decision.explanation = "This optimization requires Operator (admin) mode.";
        m_rejectedCount++;
        emit proposalRejected(proposal, decision.reason);
        return decision;
    }
    
    // ===== RULE 5: Check cooldown =====
    uint64_t remainingMs = 0;
    if (!checkCooldown(proposal.type, remainingMs)) {
        decision.approved = false;
        decision.reason = RejectionReason::CooldownActive;
        decision.cooldownRemainingMs = remainingMs;
        decision.explanation = QString("Cooldown active, %1 seconds remaining")
                                .arg(remainingMs / 1000);
        m_rejectedCount++;
        emit proposalRejected(proposal, decision.reason);
        return decision;
    }
    
    // ===== APPROVED =====
    decision.approved = true;
    decision.reason = RejectionReason::None;
    
    // Update cooldown for this change type
    updateCooldown(proposal.type);
    
    m_approvedCount++;
    emit proposalApproved(proposal);
    emit statsChanged();
    
    qDebug() << "[Zereca] Approved proposal, type:" << static_cast<int>(proposal.type);
    
    return decision;
}

void OptimizationArbiter::recordOutcome(const OptimizationProposal& proposal,
                                         Outcome outcome,
                                         float actualDelta)
{
    // Log to flight recorder
    if (m_flightRecorder) {
        uint8_t rollbackReason = 0;
        switch (outcome) {
            case Outcome::NEUTRAL:
                rollbackReason = RollbackReason::NONE;  // Reverted, not rollback
                break;
            case Outcome::NEGATIVE_STABILITY:
                rollbackReason = RollbackReason::NEGATIVE_STABILITY;
                break;
            case Outcome::NEGATIVE_SAFETY:
                rollbackReason = RollbackReason::NEGATIVE_SAFETY;
                break;
            default:
                rollbackReason = RollbackReason::NONE;
                break;
        }
        
        m_flightRecorder->record(
            static_cast<uint32_t>(proposal.type),
            proposal.currentValue,
            proposal.proposedValue,
            proposal.expectedGain,
            actualDelta,
            rollbackReason
        );
    }
    
    // Update probation ledger for negative outcomes
    if (m_probationLedger && 
        (outcome == Outcome::NEGATIVE_STABILITY || outcome == Outcome::NEGATIVE_SAFETY)) {
        
        uint64_t configHash = proposal.currentValue ^ proposal.proposedValue ^ 
                              static_cast<uint64_t>(proposal.type);
        
        Severity severity = (outcome == Outcome::NEGATIVE_SAFETY) 
                            ? Severity::CRITICAL 
                            : Severity::MEDIUM;
        
        SystemContext context = ContextHash::capture();
        m_probationLedger->addToProbation(configHash, severity, context);
    }
}

bool OptimizationArbiter::checkCooldown(ChangeType type, uint64_t& remainingMs) const
{
    auto it = m_lastApplied.find(type);
    if (it == m_lastApplied.end()) {
        return true;  // Never applied, no cooldown
    }
    
    uint64_t now = QDateTime::currentMSecsSinceEpoch();
    uint64_t cooldown = getCooldownDuration(type);
    uint64_t elapsed = now - *it;
    
    if (elapsed >= cooldown) {
        return true;  // Cooldown expired
    }
    
    remainingMs = cooldown - elapsed;
    return false;
}

void OptimizationArbiter::updateCooldown(ChangeType type)
{
    m_lastApplied[type] = QDateTime::currentMSecsSinceEpoch();
}

uint64_t OptimizationArbiter::getCooldownDuration(ChangeType type) const
{
    // Cooldown durations per spec (Section 5.1)
    switch (type) {
        case ChangeType::PRIORITY:
        case ChangeType::IO_PRIORITY:
            return 5 * 1000;        // 5 seconds (Low)
        
        case ChangeType::AFFINITY:
            return 30 * 1000;       // 30 seconds (Medium)
        
        case ChangeType::TIMER:
        case ChangeType::POWER_PLAN:
            return 2 * 60 * 1000;   // 2 minutes (High)
        
        case ChangeType::HPET:
            return 10 * 60 * 1000;  // 10 minutes (Extreme)
    }
    
    return 60 * 1000;  // Default 1 minute
}

bool OptimizationArbiter::requiresOperatorMode(ChangeType type) const
{
    // These changes require admin privileges
    switch (type) {
        case ChangeType::TIMER:
        case ChangeType::POWER_PLAN:
        case ChangeType::HPET:
            return true;
        
        default:
            return false;
    }
}

} // namespace Zereca
