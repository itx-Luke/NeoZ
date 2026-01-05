#ifndef ZERECA_FLIGHT_RECORDER_H
#define ZERECA_FLIGHT_RECORDER_H

#include "../types/ZerecaTypes.h"
#include <QObject>
#include <QMutex>
#include <vector>
#include <QString>

namespace Zereca {

/**
 * @brief Flight Recorder - Audit system for state changes.
 * 
 * Maintains a circular buffer of the last 5 minutes of state changes.
 * On failure (crash, thermal, BSOD), the buffer is dumped to disk
 * for post-mortem analysis.
 * 
 * Purpose:
 * - User trust (transparent operation)
 * - Support (debugging)
 * - Anti-cheat defensibility (proof of legitimate operation)
 */
class FlightRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int recordCount READ recordCount NOTIFY recordCountChanged)
    
public:
    explicit FlightRecorder(QObject* parent = nullptr);
    ~FlightRecorder() override;
    
    /**
     * @brief Record a state change.
     */
    void record(const StateChangeRecord& entry);
    
    /**
     * @brief Record a state change with individual fields.
     */
    void record(uint32_t component, uint64_t oldVal, uint64_t newVal,
                float expectedGain = 0.0f, float actualDelta = 0.0f,
                uint8_t rollbackReason = 0);
    
    /**
     * @brief Dump all records to disk.
     * Called automatically on failure.
     * @param reason Reason for the dump
     * @return Path to the dump file
     */
    QString dumpToDisk(const QString& reason);
    
    /**
     * @brief Get recent records (last N entries).
     * @param count Number of entries to retrieve
     */
    std::vector<StateChangeRecord> recentRecords(size_t count = 100) const;
    
    /**
     * @brief Get all records.
     */
    std::vector<StateChangeRecord> allRecords() const;
    
    /**
     * @brief Clear all records.
     */
    void clear();
    
    /**
     * @brief Get the number of records currently stored.
     */
    int recordCount() const;
    
    /**
     * @brief Maximum buffer duration in milliseconds (5 minutes).
     */
    static constexpr uint64_t MAX_BUFFER_DURATION_MS = 5 * 60 * 1000;
    
    /**
     * @brief Maximum number of records (prevents unbounded growth).
     */
    static constexpr size_t MAX_RECORDS = 10000;
    
signals:
    void recordCountChanged(int count);
    void dumpCreated(const QString& path, const QString& reason);
    
private:
    void pruneOldRecords();
    
    mutable QMutex m_mutex;
    std::vector<StateChangeRecord> m_buffer;
    QString m_dumpDir;
};

// ============================================================================
// Component IDs for StateChangeRecord
// ============================================================================

namespace Component {
    constexpr uint32_t POWER_MODE = 1;
    constexpr uint32_t TIMER_RESOLUTION = 2;
    constexpr uint32_t CPU_PARKING = 3;
    constexpr uint32_t PROCESS_AFFINITY = 4;
    constexpr uint32_t PROCESS_PRIORITY = 5;
    constexpr uint32_t IO_PRIORITY = 6;
    constexpr uint32_t STANDBY_PURGE = 7;
    constexpr uint32_t GPU_PREFERENCE = 8;
}

// ============================================================================
// Rollback Reasons
// ============================================================================

namespace RollbackReason {
    constexpr uint8_t NONE = 0;
    constexpr uint8_t USER_REQUESTED = 1;
    constexpr uint8_t DRIFT_DETECTED = 2;
    constexpr uint8_t NEGATIVE_STABILITY = 3;
    constexpr uint8_t NEGATIVE_SAFETY = 4;
    constexpr uint8_t EMERGENCY = 5;
    constexpr uint8_t PRIVILEGE_LOST = 6;
}

} // namespace Zereca

#endif // ZERECA_FLIGHT_RECORDER_H
