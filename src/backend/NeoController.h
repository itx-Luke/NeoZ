#ifndef NEOCONTROLLER_H
#define NEOCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QDateTime>
#include <QVariantList>
#include <QMap>
#include <QStringList>
#include "../core/ai/AiAdvisor.h"
#include "../core/input/LogitechHID.h"
#include "../core/input/InputState.h"
#include "../core/sensitivity/DRCS.h"
#include "../core/sensitivity/VelocityCurve.h"
#include "../core/aim/CrosshairDetector.h"
#include "../core/Services.h"

// Forward declarations for manager classes
namespace NeoZ {
    class InputManager;
    class SensitivityManager;
    class AiManager;
    class DeviceManager;
}

struct JobData {
    int id = -1;
    QString scriptName;
    QString scriptPath;
    QString deviceId;
    QString status;
    QDateTime startTime;
    QString log;
    QString errorLog;
    QProcess* process = nullptr;
};

struct InstalledEmulator {
    QString name;
    QString path;
    QString icon;
};

class NeoController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString emulatorStatus READ emulatorStatus NOTIFY statusChanged)
    Q_PROPERTY(QString adbStatus READ adbStatus NOTIFY statusChanged)
    Q_PROPERTY(QString resolution READ resolution NOTIFY statusChanged)
    Q_PROPERTY(QString processId READ processId NOTIFY statusChanged)
    Q_PROPERTY(QString mobileRes READ mobileRes NOTIFY statusChanged)
    Q_PROPERTY(QString mobileDpi READ mobileDpi NOTIFY statusChanged)
    Q_PROPERTY(bool freeFireRunning READ freeFireRunning NOTIFY statusChanged)
    Q_PROPERTY(QVariantList installedEmulators READ installedEmulators NOTIFY installedEmulatorsChanged)

    Q_PROPERTY(double xMultiplier READ xMultiplier WRITE setXMultiplier NOTIFY sensitivityChanged)
    Q_PROPERTY(double yMultiplier READ yMultiplier WRITE setYMultiplier NOTIFY sensitivityChanged)
    Q_PROPERTY(QString curve READ curve NOTIFY sensitivityChanged)
    Q_PROPERTY(int slowZone READ slowZone NOTIFY sensitivityChanged)
    Q_PROPERTY(int smoothing READ smoothing WRITE setSmoothing NOTIFY sensitivityChanged)
    Q_PROPERTY(int mouseDpi READ mouseDpi WRITE setMouseDpi NOTIFY sensitivityChanged)

    Q_PROPERTY(QString inputStatus READ inputStatus NOTIFY statusChanged)
    Q_PROPERTY(QString displayRefreshRate READ displayRefreshRate NOTIFY statusChanged)
    Q_PROPERTY(QString scriptStatus READ scriptStatus NOTIFY statusChanged)
    Q_PROPERTY(QString aiStatus READ aiStatus NOTIFY aiStatusChanged)
    Q_PROPERTY(bool inputHookActive READ inputHookActive NOTIFY inputHookChanged)
    
    // DRCS
    Q_PROPERTY(bool drcsEnabled READ drcsEnabled WRITE setDrcsEnabled NOTIFY drcsChanged)
    Q_PROPERTY(double drcsRepetitionTolerance READ drcsRepetitionTolerance WRITE setDrcsRepetitionTolerance NOTIFY drcsChanged)
    Q_PROPERTY(double drcsDirectionThreshold READ drcsDirectionThreshold WRITE setDrcsDirectionThreshold NOTIFY drcsChanged)
    Q_PROPERTY(double drcsSuppressionLevel READ drcsSuppressionLevel NOTIFY drcsChanged)

    Q_PROPERTY(QStringList adbDevices READ adbDevices NOTIFY devicesChanged)
    Q_PROPERTY(QString selectedDevice READ selectedDevice WRITE setSelectedDevice NOTIFY devicesChanged)

    Q_PROPERTY(bool aiEnabled READ aiEnabled WRITE setAiEnabled NOTIFY aiEnabledChanged)
    Q_PROPERTY(bool aiProcessing READ aiProcessing NOTIFY aiStatusChanged)
    Q_PROPERTY(double fpsMean READ fpsMean NOTIFY metricsChanged)
    Q_PROPERTY(double fpsStdDev READ fpsStdDev NOTIFY metricsChanged)

    Q_PROPERTY(bool hasRecommendation READ hasRecommendation NOTIFY recommendationChanged)
    Q_PROPERTY(QString lastRecommendationSummary READ lastRecommendationSummary NOTIFY recommendationChanged)
    Q_PROPERTY(double recommendedX READ recommendedX NOTIFY recommendationChanged)
    Q_PROPERTY(double recommendedY READ recommendedY NOTIFY recommendationChanged)
    Q_PROPERTY(double recommendationConfidence READ recommendationConfidence NOTIFY recommendationChanged)
    Q_PROPERTY(double aiConfidenceThreshold READ aiConfidenceThreshold WRITE setAiConfidenceThreshold NOTIFY aiEnabledChanged)

    Q_PROPERTY(QVariantList scriptJobs READ scriptJobs NOTIFY scriptJobsChanged)
    Q_PROPERTY(QString currentScriptLog READ currentScriptLog NOTIFY scriptLogChanged)
    Q_PROPERTY(QString currentScriptError READ currentScriptError NOTIFY scriptLogChanged)
    Q_PROPERTY(bool scriptRunning READ scriptRunning NOTIFY scriptRunningChanged)
    Q_PROPERTY(QString lastScriptPath READ lastScriptPath NOTIFY lastScriptChanged)
    Q_PROPERTY(int activeJobCount READ activeJobCount NOTIFY scriptJobsChanged)

    Q_PROPERTY(int theme READ theme WRITE setTheme NOTIFY themeChanged)

    Q_PROPERTY(bool aimAssistActive READ aimAssistActive NOTIFY aimAssistStateChanged)
    Q_PROPERTY(bool crosshairDetectionEnabled READ crosshairDetectionEnabled WRITE setCrosshairDetectionEnabled NOTIFY aimAssistStateChanged)
    Q_PROPERTY(double aimAssistYReduction READ aimAssistYReduction WRITE setAimAssistYReduction NOTIFY aimAssistStateChanged)

    Q_PROPERTY(bool hasSnapshot READ hasSnapshot NOTIFY snapshotChanged)

    // Velocity Curve
    Q_PROPERTY(int velocityCurvePreset READ velocityCurvePreset WRITE setVelocityCurvePreset NOTIFY velocityCurveChanged)
    Q_PROPERTY(double velocityLowThreshold READ velocityLowThreshold WRITE setVelocityLowThreshold NOTIFY velocityCurveChanged)
    Q_PROPERTY(double velocityHighThreshold READ velocityHighThreshold WRITE setVelocityHighThreshold NOTIFY velocityCurveChanged)
    Q_PROPERTY(double velocityLowMultiplier READ velocityLowMultiplier WRITE setVelocityLowMultiplier NOTIFY velocityCurveChanged)
    Q_PROPERTY(double velocityHighMultiplier READ velocityHighMultiplier WRITE setVelocityHighMultiplier NOTIFY velocityCurveChanged)

    // Telemetry
    Q_PROPERTY(double mouseVelocity READ mouseVelocity NOTIFY telemetryChanged)
    Q_PROPERTY(double mouseAngleDegrees READ mouseAngleDegrees NOTIFY telemetryChanged)
    Q_PROPERTY(double latencyMs READ latencyMs NOTIFY telemetryChanged)

public:
    explicit NeoController(QObject *parent = nullptr);
    ~NeoController();

    QString emulatorStatus() const { return m_emulatorStatus; }
    QString adbStatus() const { return m_adbStatus; }
    QString resolution() const { return m_resolution; }
    QString processId() const { return m_processId; }
    QString mobileRes() const { return m_mobileRes; }
    QString mobileDpi() const { return m_mobileDpi; }
    bool freeFireRunning() const { return m_freeFireRunning; }

    double xMultiplier() const { return m_xMultiplier; }
    double yMultiplier() const { return m_yMultiplier; }
    QString curve() const { return m_curve; }
    int slowZone() const { return m_slowZone; }
    int smoothing() const { return m_smoothing; }
    int mouseDpi() const { return m_mouseDpi; }
    void setMouseDpi(int dpi);
    void setXMultiplier(double value);
    void setYMultiplier(double value);
    void setSmoothing(int value);

    QString inputStatus() const { return m_inputStatus; }
    QString displayRefreshRate() const { return m_displayRefreshRate; }
    QString scriptStatus() const { return m_scriptStatus; }
    QString aiStatus() const { return m_aiStatus; }
    bool inputHookActive() const;
    
    bool drcsEnabled() const;
    void setDrcsEnabled(bool enabled);
    double drcsRepetitionTolerance() const;
    void setDrcsRepetitionTolerance(double value);
    double drcsDirectionThreshold() const;
    void setDrcsDirectionThreshold(double value);
    double drcsSuppressionLevel() const;
    DRCS* drcs() { return m_drcs; }

    QStringList adbDevices() const { return m_adbDevices; }
    QString selectedDevice() const { return m_selectedDevice; }
    void setSelectedDevice(const QString& device);

    bool aiEnabled() const { return m_aiEnabled; }
    void setAiEnabled(bool enabled);
    bool aiProcessing() const { return m_aiProcessing; }
    double fpsMean() const { return m_fpsMean; }
    double fpsStdDev() const { return m_fpsStdDev; }
    double aiConfidenceThreshold() const { return m_aiConfidenceThreshold; }
    void setAiConfidenceThreshold(double threshold);
    bool hasRecommendation() const { return m_hasRecommendation; }
    QString lastRecommendationSummary() const { return m_lastRecommendationSummary; }
    double recommendedX() const { return m_recommendedX; }
    double recommendedY() const { return m_recommendedY; }
    double recommendationConfidence() const { return m_recommendationConfidence; }
    Q_INVOKABLE void scanForDevices();
    Q_INVOKABLE void identifyEmulators();
    Q_INVOKABLE void disconnectAdb();
    Q_INVOKABLE void setSensitivity(double x, double y, QString curve, int slowZone, int smoothing);
    Q_INVOKABLE void toggleInputHook();
    Q_INVOKABLE void runAiAnalysis();
    Q_INVOKABLE void acceptRecommendation();
    Q_INVOKABLE void declineRecommendation();
    Q_INVOKABLE void setGeminiApiKey(const QString& apiKey);
    Q_INVOKABLE void runScript(const QString& scriptPath);
    Q_INVOKABLE void runScriptOnDevice(const QString& scriptPath, const QString& deviceId);
    Q_INVOKABLE void launchEmulator(const QString& path);
    QVariantList installedEmulators() const;
    Q_INVOKABLE void runAdbCommand(const QString& command);
    Q_INVOKABLE void cancelScript(int jobId);
    Q_INVOKABLE void rerunScript(int jobId);
    Q_INVOKABLE void viewJobLogs(int jobId);
    Q_INVOKABLE void clearJobs();
    Q_INVOKABLE QString browseScriptFile();
    Q_INVOKABLE void runLastScript();
    Q_INVOKABLE void takeSnapshot();
    Q_INVOKABLE void rollback();
    bool hasSnapshot() const { return m_hasSnapshot; }

    QVariantList scriptJobs() const;
    QString currentScriptLog() const { return m_currentScriptLog; }
    QString currentScriptError() const { return m_currentScriptError; }
    bool scriptRunning() const;
    QString lastScriptPath() const { return m_lastScriptPath; }
    int activeJobCount() const;

    int theme() const { return m_theme; }
    Q_INVOKABLE void setTheme(int theme);

    bool aimAssistActive() const;
    bool crosshairDetectionEnabled() const;
    void setCrosshairDetectionEnabled(bool enabled);
    double aimAssistYReduction() const;
    void setAimAssistYReduction(double alpha);

    // Telemetry
    double mouseVelocity() const { return m_mouseVelocity; }
    double mouseAngleDegrees() const { return m_mouseAngleDegrees; }
    double latencyMs() const { return m_latencyMs; }

    // Velocity Curve accessors
    int velocityCurvePreset() const;
    void setVelocityCurvePreset(int preset);
    double velocityLowThreshold() const;
    void setVelocityLowThreshold(double v);
    double velocityHighThreshold() const;
    void setVelocityHighThreshold(double v);
    double velocityLowMultiplier() const;
    void setVelocityLowMultiplier(double v);
    double velocityHighMultiplier() const;
    void setVelocityHighMultiplier(double v);

    // Pipeline control
    Q_INVOKABLE void applyOptimization();
    int presetConfidence() const;
    bool inputAuthorityEnabled() const;
    void setInputAuthorityEnabled(bool enabled);

signals:
    void statusChanged();
    void sensitivityChanged();
    void metricsChanged();
    void aiEnabledChanged();
    void aiStatusChanged();
    void recommendationChanged();
    void recommendationReady(QString summary, double newX, double newY, QString severity);
    void devicesChanged();
    void installedEmulatorsChanged();
    void aimAssistStateChanged();
    void scriptJobsChanged();
    void scriptLogChanged();
    void scriptRunningChanged();
    void lastScriptChanged();
    void scriptStarted(int jobId, QString scriptName);
    void scriptFinished(int jobId, int exitCode);
    void scriptOutputReceived(QString line);
    void themeChanged();
    void inputHookChanged();
    void drcsChanged();
    void snapshotChanged();
    void velocityCurveChanged();
    void telemetryChanged();

private slots:
    void onRecommendationReady(const TuningRecommendation& recommendation);
    void onAiError(const QString& error);
    void onJobReadyRead();
    void onJobFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onInputProcessed(const NeoZ::InputState& state);
    void updateTelemetry();

private:
    // Manager accessors (delegates to ServiceLocator)
    NeoZ::InputManager* inputManager() const { return NeoZ::Services::input(); }
    NeoZ::SensitivityManager* sensitivityManager() const { return NeoZ::Services::sensitivity(); }
    NeoZ::AiManager* aiManager() const { return NeoZ::Services::ai(); }
    NeoZ::DeviceManager* deviceManager() const { return NeoZ::Services::device(); }
    
    void updateSystemMetrics();
    void startAdbCheck();
    void checkDisplayResolution();
    void fetchEmulatorDetails();
    void maybeTriggerAi();
    void scanForInstalledEmulators();

    QString getAdbPath();
    SystemSnapshot createSnapshot();
    void saveConfig();
    void scheduleSave();
    void loadConfig();
    void saveJobHistory();
    void loadJobHistory();
    int createJob(const QString& scriptPath, const QString& deviceId);
    JobData* findJob(int jobId);
    void updateJobStatus(int jobId, const QString& status);
    void cleanupJob(int jobId);

private:
    QString m_emulatorStatus;
    QString m_adbStatus;
    QString m_resolution;
    QString m_processId;
    QString m_mobileRes;
    QString m_mobileDpi;
    bool m_freeFireRunning = false;
    double m_xMultiplier;
    double m_yMultiplier;
    QString m_curve;
    int m_slowZone;
    int m_smoothing;
    int m_mouseDpi = 800;
    QString m_inputStatus;
    QString m_displayRefreshRate;
    QString m_scriptStatus;
    QString m_aiStatus;
    QStringList m_adbDevices;
    QString m_selectedDevice;
    bool m_adbManualDisconnected = false;
    AiAdvisor* m_aiAdvisor = nullptr;
    bool m_aiEnabled;
    bool m_aiProcessing;
    double m_fpsMean;
    double m_fpsStdDev;
    bool m_hasRecommendation;
    double m_recommendedX;
    double m_recommendedY;
    QString m_lastRecommendationSummary;
    QString m_recommendationSeverity;
    double m_recommendationConfidence;
    double m_aiConfidenceThreshold;
    int m_prevDisplayWidth;
    int m_prevDisplayHeight;
    QTimer* m_timer = nullptr;
    QString m_cachedAdbPath;
    QMap<int, JobData> m_activeJobs;
    QVariantList m_jobHistory;
    QString m_currentScriptLog;
    QString m_currentScriptError;
    QString m_lastScriptPath;
    int m_nextJobId = 1;
    int m_selectedJobId = -1;
    int m_theme = 1;
    LogitechHIDController* m_logitechHID = nullptr;
    DRCS* m_drcs = nullptr;
    QList<InstalledEmulator> m_installedEmulators;
    QTimer* m_saveTimer = nullptr;
    QProcess* m_adbProcess = nullptr;
    NeoZ::CrosshairDetector* m_crosshairDetector = nullptr;
    struct SensitivitySnapshot {
        double xMultiplier = 0;
        double yMultiplier = 0;
        int slowZone = 35;
        int smoothing = 20;
        int mouseDpi = 800;
    };
    SensitivitySnapshot m_snapshot;
    bool m_hasSnapshot = false;
    NeoZ::VelocityCurve* m_velocityCurve = nullptr;
    
    // Telemetry
    double m_mouseVelocity = 0.0;
    double m_mouseAngleDegrees = 0.0;
    double m_latencyMs = 0.0;
    QTimer* m_telemetryTimer = nullptr;
    double m_pendingVelocity = 0.0;
    double m_pendingAngle = 0.0;
    double m_pendingLatency = 0.0;

};

#endif // NEOCONTROLLER_H
