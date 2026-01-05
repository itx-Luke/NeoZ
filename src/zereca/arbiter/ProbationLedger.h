#ifndef ZERECA_PROBATION_LEDGER_H
#define ZERECA_PROBATION_LEDGER_H

#include "../types/ZerecaTypes.h"
#include <QObject>
#include <QHash>
#include <QMutex>
#include <vector>

namespace Zereca {

/**
 * @brief Probation Ledger - Tracks failed configurations.
 * 
 * Failures are scoped to their system context (GPU driver, OS build, etc.).
 * A configuration that failed under one context may be retried after
 * a context shift (driver update, OS upgrade).
 * 
 * Resurrection Rules:
 * - Severity 1 (FPS Regression): Auto-retry allowed
 * - Severity 2 (App Crash): Context shift only
 * - Severity 3 (BSOD): NEVER auto-retry, manual reset only
 */
class ProbationLedger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int entryCount READ entryCount NOTIFY entriesChanged)
    
public:
    explicit ProbationLedger(QObject* parent = nullptr);
    ~ProbationLedger() override;
    
    /**
     * @brief Check if a configuration is on probation.
     * @param configHash Hash of the optimization configuration
     * @param currentContext Current system context
     * @return true if configuration should be blocked
     */
    bool isOnProbation(uint64_t configHash, const SystemContext& currentContext) const;
    
    /**
     * @brief Add a configuration to probation.
     * @param configHash Hash of the optimization configuration
     * @param severity Failure severity
     * @param context System context at time of failure
     */
    void addToProbation(uint64_t configHash, Severity severity, const SystemContext& context);
    
    /**
     * @brief Get probation entry for a configuration (if exists).
     */
    std::optional<ProbationEntry> getEntry(uint64_t configHash) const;
    
    /**
     * @brief Clear probation for a specific configuration (manual reset).
     */
    void clearEntry(uint64_t configHash);
    
    /**
     * @brief Clear all probation entries.
     * Only for manual reset after BSOD.
     */
    Q_INVOKABLE void clearAll();
    
    /**
     * @brief Get all probation entries.
     */
    std::vector<ProbationEntry> allEntries() const;
    
    /**
     * @brief Get total entry count.
     */
    int entryCount() const;
    
    /**
     * @brief Load probation ledger from disk.
     */
    bool load();
    
    /**
     * @brief Save probation ledger to disk.
     */
    bool save();
    
signals:
    void entriesChanged(int count);
    void entryAdded(uint64_t configHash, Severity severity);
    void entryCleared(uint64_t configHash);
    
private:
    bool canResurrect(const ProbationEntry& entry, const SystemContext& currentContext) const;
    
    mutable QMutex m_mutex;
    QHash<uint64_t, ProbationEntry> m_entries;
    QString m_storagePath;
};

} // namespace Zereca

#endif // ZERECA_PROBATION_LEDGER_H
