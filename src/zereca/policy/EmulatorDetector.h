#ifndef ZERECA_EMULATOR_DETECTOR_H
#define ZERECA_EMULATOR_DETECTOR_H

#include "../types/ZerecaTypes.h"
#include <QObject>
#include <QTimer>
#include <QHash>
#include <QString>
#include <vector>

namespace Zereca {

/**
 * @brief Detected emulator information.
 */
struct EmulatorInfo {
    QString name;                    ///< Friendly name (e.g., "Bluestacks 5")
    QString executablePath;          ///< Full path to main executable
    uint32_t processId = 0;          ///< Main process ID
    std::vector<uint32_t> childPids; ///< Child process IDs
    float confidence = 0.0f;         ///< Detection confidence (0.0–1.0)
    uint64_t binaryHash = 0;         ///< Hash of executable for context
};

/**
 * @brief Emulator Detector - Multi-signal emulator detection (System B).
 * 
 * Detects running emulators using multiple signals:
 * - Executable name matching
 * - Child process topology
 * - Loaded modules (DLLs)
 * - Window class names
 * - GPU context signatures
 * - ETW CPU patterns
 * 
 * Supported emulators:
 * - Bluestacks / MSI App Player
 * - LDPlayer
 * - Nox
 * - MEmu
 * - SmartGaGa
 * 
 * The confidence score gates System B proposals.
 * Arbiter rejects proposals if confidence < 0.75.
 */
class EmulatorDetector : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(int detectedCount READ detectedCount NOTIFY emulatorDetected)
    
public:
    /**
     * @brief Known emulator signatures.
     */
    struct EmulatorSignature {
        QString name;
        QStringList executableNames;      ///< e.g., ["HD-Player.exe", "Bluestacks.exe"]
        QStringList windowClasses;        ///< e.g., ["BlueStacksApp"]
        QStringList requiredModules;      ///< DLLs that should be loaded
        float baseConfidence = 0.5f;      ///< Base confidence when exe matches
    };
    
    explicit EmulatorDetector(QObject* parent = nullptr);
    ~EmulatorDetector() override;
    
    /**
     * @brief Start continuous scanning for emulators.
     * @param intervalMs Scan interval in milliseconds
     */
    Q_INVOKABLE void startScanning(int intervalMs = 2000);
    
    /**
     * @brief Stop scanning.
     */
    Q_INVOKABLE void stopScanning();
    
    /**
     * @brief Perform a single scan now.
     * @return List of detected emulators
     */
    Q_INVOKABLE std::vector<EmulatorInfo> scanNow();
    
    /**
     * @brief Check if scanning is active.
     */
    bool isScanning() const { return m_scanning; }
    
    /**
     * @brief Get count of currently detected emulators.
     */
    int detectedCount() const { return static_cast<int>(m_detected.size()); }
    
    /**
     * @brief Get all currently detected emulators.
     */
    const std::vector<EmulatorInfo>& detected() const { return m_detected; }
    
    /**
     * @brief Get the primary (highest confidence) emulator.
     * @return EmulatorInfo or empty if none detected
     */
    EmulatorInfo primaryEmulator() const;
    
    /**
     * @brief Add a custom emulator signature.
     */
    void addSignature(const EmulatorSignature& sig);
    
signals:
    void scanningChanged(bool scanning);
    
    /**
     * @brief Emitted when an emulator is detected.
     */
    void emulatorDetected(const EmulatorInfo& info);
    
    /**
     * @brief Emitted when an emulator exits.
     */
    void emulatorLost(uint32_t processId);
    
    /**
     * @brief Emitted after each scan completes.
     */
    void scanComplete(int detectedCount);
    
private slots:
    void onScanTick();
    
private:
    void initDefaultSignatures();
    
    // Detection methods
    std::vector<EmulatorInfo> detectByExecutable();
    float boostConfidenceByWindowClass(const EmulatorInfo& info);
    float boostConfidenceByModules(const EmulatorInfo& info);
    float boostConfidenceByChildProcesses(const EmulatorInfo& info);
    std::vector<uint32_t> getChildProcesses(uint32_t parentPid);
    QStringList getLoadedModules(uint32_t pid);
    QString getWindowClassForProcess(uint32_t pid);
    
    QTimer* m_timer = nullptr;
    bool m_scanning = false;
    
    std::vector<EmulatorSignature> m_signatures;
    std::vector<EmulatorInfo> m_detected;
    QHash<uint32_t, EmulatorInfo> m_tracked;  // PID → Info
};

} // namespace Zereca

#endif // ZERECA_EMULATOR_DETECTOR_H
