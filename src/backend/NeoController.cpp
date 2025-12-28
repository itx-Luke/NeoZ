#include "NeoController.h"
#include "../core/ai/AiAdvisor.h"
#include "../core/input/InputHook.h"

#include <QDebug>
#include <QProcess>
#include <QGuiApplication>
#include <QScreen>
#include <QRegularExpression>
#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <cmath>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static constexpr int SCRIPT_TIMEOUT_MS = 5 * 60 * 1000;

// ==========================
// Constructor
// ==========================
NeoController::NeoController(QObject *parent)
    : QObject(parent),
      m_emulatorStatus("Searching..."),
      m_adbStatus("Offline"),
      m_resolution("Detecting..."),
      m_processId("---"),
      m_mobileRes("-"),
      m_mobileDpi("-"),
      m_freeFireRunning(false),
      m_xMultiplier(1.12),
      m_yMultiplier(1.15),
      m_inputStatus("Safe Mode"),
      m_displayRefreshRate("Unknown"),
      m_scriptStatus("Idle"),
      m_aiStatus("Initializing..."),
      m_curve("FF_OneTap_v2"),
      m_slowZone(35),
      m_smoothing(20),
      m_aiEnabled(true),
      m_aiProcessing(false),
      m_fpsMean(0.0),
      m_fpsStdDev(0.0),
      m_hasRecommendation(false),
      m_recommendedX(0.0),
      m_recommendedY(0.0),
      m_recommendationConfidence(0.0),
      m_aiConfidenceThreshold(0.65),
      m_prevDisplayWidth(0),
      m_prevDisplayHeight(0),
      m_nextJobId(1),
      m_selectedJobId(-1),
      m_adbManualDisconnected(true) // Disable auto-connect on startup
{
    qDebug() << "[NeoController] Constructor starting...";
    
    qDebug() << "[NeoController] Creating AiAdvisor...";
    m_aiAdvisor = new AiAdvisor(this);

    connect(m_aiAdvisor, &AiAdvisor::recommendationReady,
            this, &NeoController::onRecommendationReady);
    connect(m_aiAdvisor, &AiAdvisor::analysisError,
            this, &NeoController::onAiError);

    connect(m_aiAdvisor, &AiAdvisor::statusChanged, this, [this]() {
        m_aiStatus = m_aiAdvisor->status();
        m_aiProcessing = m_aiAdvisor->isProcessing();
        emit aiStatusChanged();
    });
    qDebug() << "[NeoController] AiAdvisor created";

    qDebug() << "[NeoController] Loading config...";
    loadConfig();
    qDebug() << "[NeoController] Config loaded";
    
    qDebug() << "[NeoController] Loading job history...";
    loadJobHistory();
    qDebug() << "[NeoController] Job history loaded";

    // Mark interrupted jobs from previous session
    for (auto &v : m_jobHistory) {
        QVariantMap m = v.toMap();
        if (m["status"] == "Running")
            m["status"] = "Interrupted";
        v = m;
    }

    qDebug() << "[NeoController] Getting primary screen...";
    if (QScreen *s = QGuiApplication::primaryScreen()) {
        m_prevDisplayWidth = s->geometry().width();
        m_prevDisplayHeight = s->geometry().height();
    }
    qDebug() << "[NeoController] Screen info acquired";

    qDebug() << "[NeoController] Creating timer...";
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &NeoController::updateSystemMetrics);
    m_timer->start(3000);
    
    // Telemetry Timer (60 FPS update rate for UI)
    m_telemetryTimer = new QTimer(this);
    m_telemetryTimer->setInterval(16); 
    connect(m_telemetryTimer, &QTimer::timeout, this, &NeoController::updateTelemetry);
    m_telemetryTimer->start();
    qDebug() << "[NeoController] Timer created";

    // CRITICAL: Initialize m_adbProcess BEFORE calling updateSystemMetrics
    // because updateSystemMetrics()->startAdbCheck() uses m_adbProcess
    qDebug() << "[NeoController] Creating ADB process...";
    m_adbProcess = new QProcess(this);
    connect(m_adbProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this]() {
        QString out = QString::fromUtf8(m_adbProcess->readAllStandardOutput());
        m_adbDevices.clear();

        for (const QString &l : out.split('\n')) {
            if (l.contains("\tdevice"))
                m_adbDevices << l.section('\t', 0, 0);
        }

        m_adbStatus = m_adbDevices.isEmpty() ? "Offline" : "Connected";

        if (!m_adbDevices.isEmpty() && !m_adbDevices.contains(m_selectedDevice)) {
            // Only auto-select if not manually disconnected
            if (!m_adbManualDisconnected) {
                m_selectedDevice = m_adbDevices.first();
            }
        }

        emit devicesChanged();
        emit statusChanged();
    });
    qDebug() << "[NeoController] ADB process created";

    // Initialize save timer early too
    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(2000);
    connect(m_saveTimer, &QTimer::timeout, this, &NeoController::saveConfig);
    qDebug() << "[NeoController] Save timer created";

    qDebug() << "[NeoController] Calling updateSystemMetrics...";
    updateSystemMetrics();
    qDebug() << "[NeoController] updateSystemMetrics completed";
    
    qDebug() << "[NeoController] Creating LogitechHID...";
    // Initialize Logitech HID++ controller
    m_logitechHID = new LogitechHIDController(this);
    qDebug() << "[NeoController] LogitechHID created";
    
    connect(m_logitechHID, &LogitechHIDController::dpiChanged, this, [this](int dpi) {
        m_mouseDpi = dpi;
        emit sensitivityChanged();
        qDebug() << "[NeoController] Real DPI changed to:" << dpi;
    });
    connect(m_logitechHID, &LogitechHIDController::connectionChanged, this, [this]() {
        emit statusChanged();
    });
    connect(m_logitechHID, &LogitechHIDController::error, this, [this](QString msg) {
        qWarning() << "[NeoController] Logitech HID error:" << msg;
    });
    qDebug() << "[NeoController] LogitechHID signals connected";
    
    // Try to connect to a Logitech mouse
    qDebug() << "[NeoController] Scanning for Logitech devices...";
    if (m_logitechHID->scanForDevices()) {
        qDebug() << "[NeoController] Found Logitech device, connecting...";
        m_logitechHID->connectToDevice();
    } else {
        qDebug() << "[NeoController] No Logitech devices found";
    }
    qDebug() << "[NeoController] Logitech scan complete";
    
    qDebug() << "[NeoController] Creating DRCS...";
    // Initialize DRCS - Directional Repetition Constraint System
    m_drcs = new DRCS(this);
    connect(m_drcs, &DRCS::enabledChanged, this, &NeoController::drcsChanged);
    connect(m_drcs, &DRCS::parametersChanged, this, &NeoController::drcsChanged);
    connect(m_drcs, &DRCS::suppressionChanged, this, &NeoController::drcsChanged);
    qDebug() << "[NeoController] DRCS initialized";

    qDebug() << "[NeoController] Constructor completed successfully!";
}

// ==========================
// Destructor
// ==========================
NeoController::~NeoController()
{
    saveConfig();
    saveJobHistory();

    for (auto &j : m_activeJobs) {
        if (j.process) {
            j.process->kill();
            j.process->waitForFinished(200);
            delete j.process;
        }
    }
    m_activeJobs.clear();
}

// ==========================
// Config Persistence
// ==========================
// ==========================
// Config Persistence
// ==========================
void NeoController::scheduleSave()
{
    if (m_saveTimer)
        m_saveTimer->start();
}

void NeoController::saveConfig()
{
    QSettings s("Neo", "NeoController");
    s.setValue("x", m_xMultiplier);
    s.setValue("y", m_yMultiplier);
    s.setValue("curve", m_curve);
    s.setValue("slowZone", m_slowZone);
    s.setValue("smoothing", m_smoothing);
    s.setValue("aiEnabled", m_aiEnabled);
    s.setValue("aiConfidence", m_aiConfidenceThreshold);
    s.setValue("device", m_selectedDevice);
    s.setValue("theme", m_theme);
    s.setValue("mouseDpi", m_mouseDpi);
}

void NeoController::loadConfig()
{
    QSettings s("Neo", "NeoController");
    m_xMultiplier = s.value("x", m_xMultiplier).toDouble();
    m_yMultiplier = s.value("y", m_yMultiplier).toDouble();
    m_curve = s.value("curve", m_curve).toString();
    m_slowZone = s.value("slowZone", m_slowZone).toInt();
    m_smoothing = s.value("smoothing", m_smoothing).toInt();
    m_aiEnabled = s.value("aiEnabled", m_aiEnabled).toBool();
    m_aiConfidenceThreshold = s.value("aiConfidence", m_aiConfidenceThreshold).toDouble();
    m_selectedDevice = s.value("device").toString();
    m_theme = s.value("theme", 1).toInt();  // Default: GlassGradient
    m_mouseDpi = s.value("mouseDpi", 800).toInt();
}

// ==========================
// Job Persistence
// ==========================
void NeoController::saveJobHistory()
{
    QSettings s("Neo", "NeoController");
    s.setValue("jobHistory", m_jobHistory);
}

void NeoController::loadJobHistory()
{
    QSettings s("Neo", "NeoController");
    m_jobHistory = s.value("jobHistory").toList();
}

// ==========================
// ADB Path (with emulator bundled paths)
// ==========================
QString NeoController::getAdbPath()
{
    if (!m_cachedAdbPath.isEmpty())
        return m_cachedAdbPath;

#ifdef Q_OS_WIN
    QStringList candidates = {
        // Standard Android SDK paths
        QDir::homePath() + "/AppData/Local/Android/Sdk/platform-tools/adb.exe",
        "C:/Android/platform-tools/adb.exe",
        "C:/platform-tools/adb.exe",
        
        // BlueStacks paths
        "C:/Program Files/BlueStacks_nxt/HD-Adb.exe",
        "C:/Program Files (x86)/BlueStacks_nxt/HD-Adb.exe",
        "C:/Program Files/BlueStacks/HD-Adb.exe",
        "C:/Program Files (x86)/BlueStacks/HD-Adb.exe",
        
        // HD-Player / BlueStacks HD paths
        "C:/Program Files/BlueStacks_nxt/adb.exe",
        "C:/Program Files (x86)/BlueStacks_nxt/adb.exe",
        
        // LDPlayer paths
        "C:/LDPlayer/LDPlayer9/adb.exe",
        "C:/LDPlayer/LDPlayer4.0/adb.exe",
        QDir::homePath() + "/AppData/Local/Programs/LDPlayer9/adb.exe",
        "C:/Program Files/LDPlayer/LDPlayer9/adb.exe",
        
        // NoxPlayer paths
        "C:/Program Files/Nox/bin/adb.exe",
        "C:/Program Files (x86)/Nox/bin/adb.exe",
        QDir::homePath() + "/AppData/Local/Nox/bin/adb.exe",
        
        // MuMu Player paths
        "C:/Program Files/MuMu/emulator/nemu/vmonitor/bin/adb_server.exe",
        "C:/Program Files/Netease/MuMuPlayer-12.0/shell/adb.exe",
        
        // MEmu paths
        "C:/Program Files/Microvirt/MEmu/adb.exe",
        "C:/Program Files (x86)/Microvirt/MEmu/adb.exe",
        
        // Generic adb in PATH (Windows will find it)
        "adb.exe"
    };
#else
    QStringList candidates = {"adb", "/usr/bin/adb"};
#endif

    for (const QString &p : candidates) {
        if (QFileInfo::exists(p)) {
            m_cachedAdbPath = p;
            qDebug() << "[ADB] Found at:" << p;
            return p;
        }
    }
    
    qWarning() << "[ADB] Not found";
    return {};
}

// ==========================
// Metrics Loop
// ==========================
void NeoController::updateSystemMetrics()
{
    startAdbCheck();
    if (m_adbStatus == "Connected" && !m_selectedDevice.isEmpty()) {
        fetchEmulatorDetails();
    }
    checkDisplayResolution();
    maybeTriggerAi();
    emit statusChanged();
}

// ==========================
// ADB Device Scan (Async)
// ==========================
void NeoController::startAdbCheck()
{
    QString adb = getAdbPath();
    if (adb.isEmpty()) {
        m_adbStatus = "No ADB";
        return;
    }

    if (m_adbProcess->state() != QProcess::NotRunning)
        return; // Skip if already running

    m_adbProcess->start(adb, {"devices"});
}

// ==========================
// Fetch Emulator Details
// ==========================
void NeoController::fetchEmulatorDetails()
{
    QString adb = getAdbPath();
    if (adb.isEmpty() || m_selectedDevice.isEmpty()) return;

    // Only fetch static specs if missing (Caching)
    if (m_mobileRes == "-" || m_mobileRes.isEmpty()) {
        auto *pRes = new QProcess(this);
        connect(pRes, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, pRes]() {
            QString out = QString::fromUtf8(pRes->readAllStandardOutput()).trimmed();
            if (out.contains("Physical size:")) {
                m_mobileRes = out.section("Physical size: ", 1, 1).trimmed();
            }
            pRes->deleteLater();
        });
        pRes->start(adb, {"-s", m_selectedDevice, "shell", "wm", "size"});
    }

    if (m_mobileDpi == "-" || m_mobileDpi.isEmpty()) {
        auto *pDpi = new QProcess(this);
        connect(pDpi, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, pDpi]() {
            QString out = QString::fromUtf8(pDpi->readAllStandardOutput()).trimmed();
            if (out.contains("Physical density:")) {
                m_mobileDpi = out.section("Physical density: ", 1, 1).trimmed();
            }
            pDpi->deleteLater();
        });
        pDpi->start(adb, {"-s", m_selectedDevice, "shell", "wm", "density"});
    }
    
    // Check if Free Fire is running (Dynamic)
    auto *pFF = new QProcess(this);
    connect(pFF, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, pFF]() {
        QString out = QString::fromUtf8(pFF->readAllStandardOutput()).trimmed();
        bool running = out.contains("com.dts.freefireth");
        if (m_freeFireRunning != running) {
            m_freeFireRunning = running;
            emit statusChanged();
        }
        pFF->deleteLater();
    });
    pFF->start(adb, {"-s", m_selectedDevice, "shell", "pidof", "com.dts.freefireth"});
}

// ==========================
// Display Resolution
// ==========================
void NeoController::checkDisplayResolution()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect g = screen->geometry();
    int hz = static_cast<int>(screen->refreshRate());

    m_resolution = QString("%1x%2 @ %3Hz").arg(g.width()).arg(g.height()).arg(hz);
    m_displayRefreshRate = QString::number(hz) + " Hz";
}

// ==========================
// AI Trigger Guard
// ==========================
void NeoController::maybeTriggerAi()
{
    if (!m_aiEnabled || m_aiProcessing)
        return;

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    int w = screen->geometry().width();
    int h = screen->geometry().height();

    if (m_prevDisplayWidth > 0 && (w != m_prevDisplayWidth || h != m_prevDisplayHeight)) {
        m_aiProcessing = true;
        SystemSnapshot current = createSnapshot();
        SystemSnapshot previous = current;
        previous.displayWidth = m_prevDisplayWidth;
        previous.displayHeight = m_prevDisplayHeight;
        m_aiAdvisor->requestTuning(current, &previous);
    }

    m_prevDisplayWidth = w;
    m_prevDisplayHeight = h;
}

// ==========================
// Job Management
// ==========================
int NeoController::createJob(const QString& scriptPath, const QString& deviceId)
{
    JobData job;
    job.id = m_nextJobId++;
    job.scriptName = QFileInfo(scriptPath).fileName();
    job.scriptPath = scriptPath;
    job.deviceId = deviceId.isEmpty() ? m_selectedDevice : deviceId;
    job.status = "Queued";
    job.startTime = QDateTime::currentDateTime();
    job.process = nullptr;
    
    m_activeJobs.insert(job.id, job);
    m_lastScriptPath = scriptPath;
    
    emit scriptJobsChanged();
    emit lastScriptChanged();
    
    return job.id;
}

JobData* NeoController::findJob(int jobId)
{
    if (m_activeJobs.contains(jobId)) {
        return &m_activeJobs[jobId];
    }
    return nullptr;
}

void NeoController::updateJobStatus(int jobId, const QString& status)
{
    JobData* job = findJob(jobId);
    if (job) {
        job->status = status;
        emit scriptJobsChanged();
    }
}

void NeoController::cleanupJob(int jobId)
{
    if (!m_activeJobs.contains(jobId)) return;
    
    JobData& job = m_activeJobs[jobId];
    
    // Move to history
    QVariantMap historyEntry;
    historyEntry["id"] = job.id;
    historyEntry["script"] = job.scriptName;
    historyEntry["device"] = job.deviceId;
    historyEntry["status"] = job.status;
    historyEntry["started"] = job.startTime.toString("hh:mm:ss");
    
    qint64 durationMs = job.startTime.msecsTo(QDateTime::currentDateTime());
    int secs = durationMs / 1000;
    historyEntry["duration"] = QString("%1:%2:%3")
        .arg(secs / 3600, 2, 10, QChar('0'))
        .arg((secs % 3600) / 60, 2, 10, QChar('0'))
        .arg(secs % 60, 2, 10, QChar('0'));
    historyEntry["scriptPath"] = job.scriptPath;
    historyEntry["log"] = job.log;
    historyEntry["errorLog"] = job.errorLog;
    
    m_jobHistory.prepend(historyEntry);
    
    // Limit history size
    while (m_jobHistory.size() > 50) {
        m_jobHistory.removeLast();
    }
    
    // Clean up process
    if (job.process) {
        job.process->deleteLater();
    }
    
    m_activeJobs.remove(jobId);
    emit scriptJobsChanged();
    emit scriptRunningChanged();
}

// ==========================
// Script Execution (Async with timeout)
// ==========================
void NeoController::runScript(const QString &scriptPath)
{
    runScriptOnDevice(scriptPath, m_selectedDevice);
}

void NeoController::runScriptOnDevice(const QString &scriptPath, const QString &device)
{
    QString adb = getAdbPath();
    if (adb.isEmpty() || scriptPath.isEmpty()) {
        qWarning() << "[Script] ADB not found or empty script path";
        return;
    }

    int jobId = createJob(scriptPath, device);
    JobData *job = findJob(jobId);
    if (!job) return;

    QString remote = "/data/local/tmp/" + QFileInfo(scriptPath).fileName();
    QStringList push = device.isEmpty()
        ? QStringList{"push", scriptPath, remote}
        : QStringList{"-s", device, "push", scriptPath, remote};

    // Push file (blocking is OK, it's quick)
    if (QProcess::execute(adb, push) != 0) {
        job->status = "Failed";
        job->errorLog = "Failed to push script to device";
        cleanupJob(jobId);
        return;
    }

    job->process = new QProcess(this);
    job->status = "Running";
    job->startTime = QDateTime::currentDateTime();
    job->process->setProperty("jobId", jobId);

    connect(job->process, &QProcess::readyReadStandardOutput, this, &NeoController::onJobReadyRead);
    connect(job->process, &QProcess::readyReadStandardError, this, &NeoController::onJobReadyRead);
    connect(job->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &NeoController::onJobFinished);

    // Timeout handler
    QTimer::singleShot(SCRIPT_TIMEOUT_MS, job->process, [this, jobId]() {
        JobData *j = findJob(jobId);
        if (j && j->status == "Running") {
            j->status = "Timeout";
            if (j->process) j->process->kill();
            cleanupJob(jobId);
        }
    });

    QStringList exec = device.isEmpty()
        ? QStringList{"shell", "sh " + remote}
        : QStringList{"-s", device, "shell", "sh " + remote};

    job->process->start(adb, exec);
    
    m_scriptStatus = "Running";
    m_selectedJobId = jobId;
    
    emit scriptStarted(jobId, job->scriptName);
    emit scriptJobsChanged();
    emit scriptRunningChanged();
    emit statusChanged();
}

void NeoController::onJobReadyRead()
{
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;

    int jobId = proc->property("jobId").toInt();
    JobData* job = findJob(jobId);
    if (!job) return;

    QString out = QString::fromUtf8(proc->readAllStandardOutput());
    QString err = QString::fromUtf8(proc->readAllStandardError());

    if (!out.isEmpty()) {
        job->log += out;
        if (jobId == m_selectedJobId) {
            m_currentScriptLog = job->log;
            emit scriptOutputReceived(out);
        }
    }

    if (!err.isEmpty()) {
        job->errorLog += err;
        if (jobId == m_selectedJobId) {
            m_currentScriptError = job->errorLog;
        }
    }

    if (jobId == m_selectedJobId) {
        emit scriptLogChanged();
    }
}

void NeoController::onJobFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;

    int jobId = proc->property("jobId").toInt();
    JobData* job = findJob(jobId);
    if (!job) return;

    // Set final status
    if (job->status == "Running") {
        if (exitStatus == QProcess::CrashExit) {
            job->status = "Crashed";
        } else if (exitCode == 0) {
            job->status = "Success";
        } else {
            job->status = "Failed";
        }
    }

    qDebug() << "[Script] Job" << jobId << "finished with code" << exitCode << "status" << job->status;

    emit scriptFinished(jobId, exitCode);

    cleanupJob(jobId);

    if (m_activeJobs.isEmpty()) {
        m_scriptStatus = "Idle";
    }

    emit statusChanged();
}

// ==========================
// Script Runner Methods
// ==========================
void NeoController::cancelScript(int jobId)
{
    JobData* job = findJob(jobId);
    if (job) {
        job->status = "Cancelled";
        if (job->process) {
            job->process->kill();
        }
        cleanupJob(jobId);
    }
}

void NeoController::rerunScript(int jobId)
{
    // Check active jobs
    JobData* job = findJob(jobId);
    if (job) {
        runScriptOnDevice(job->scriptPath, job->deviceId);
        return;
    }

    // Check history
    for (const QVariant& v : m_jobHistory) {
        QVariantMap entry = v.toMap();
        if (entry["id"].toInt() == jobId) {
            runScriptOnDevice(entry["scriptPath"].toString(), entry["device"].toString());
            return;
        }
    }
}

void NeoController::viewJobLogs(int jobId)
{
    m_selectedJobId = jobId;

    // Check active jobs
    JobData* job = findJob(jobId);
    if (job) {
        m_currentScriptLog = job->log;
        m_currentScriptError = job->errorLog;
        emit scriptLogChanged();
        return;
    }

    // Check history
    for (const QVariant& v : m_jobHistory) {
        QVariantMap entry = v.toMap();
        if (entry["id"].toInt() == jobId) {
            m_currentScriptLog = entry["log"].toString();
            m_currentScriptError = entry["errorLog"].toString();
            emit scriptLogChanged();
            return;
        }
    }
}

void NeoController::clearJobs()
{
    for (auto &j : m_activeJobs) {
        if (j.process) {
            j.process->kill();
        }
    }
    m_activeJobs.clear();
    m_jobHistory.clear();
    m_currentScriptLog.clear();
    m_currentScriptError.clear();
    m_selectedJobId = -1;
    
    emit scriptJobsChanged();
    emit scriptLogChanged();
}

QString NeoController::browseScriptFile()
{
    return m_lastScriptPath;
}

void NeoController::runLastScript()
{
    if (!m_lastScriptPath.isEmpty()) {
        runScript(m_lastScriptPath);
    }
}

QVariantList NeoController::scriptJobs() const
{
    QVariantList result;
    
    // Add active jobs first
    for (auto it = m_activeJobs.constBegin(); it != m_activeJobs.constEnd(); ++it) {
        QVariantMap entry;
        entry["id"] = it->id;
        entry["script"] = it->scriptName;
        entry["device"] = it->deviceId;
        entry["status"] = it->status;
        entry["started"] = it->startTime.toString("hh:mm:ss");
        
        qint64 durationMs = it->startTime.msecsTo(QDateTime::currentDateTime());
        int secs = durationMs / 1000;
        entry["duration"] = QString("%1:%2:%3")
            .arg(secs / 3600, 2, 10, QChar('0'))
            .arg((secs % 3600) / 60, 2, 10, QChar('0'))
            .arg(secs % 60, 2, 10, QChar('0'));
        entry["scriptPath"] = it->scriptPath;
        
        result.append(entry);
    }
    
    // Add history
    result.append(m_jobHistory);
    
    return result;
}

bool NeoController::scriptRunning() const
{
    for (auto it = m_activeJobs.constBegin(); it != m_activeJobs.constEnd(); ++it) {
        if (it->status == "Running") {
            return true;
        }
    }
    return false;
}

int NeoController::activeJobCount() const
{
    int count = 0;
    for (auto it = m_activeJobs.constBegin(); it != m_activeJobs.constEnd(); ++it) {
        if (it->status == "Running" || it->status == "Queued") {
            count++;
        }
    }
    return count;
}

// ==========================
// Snapshot
// ==========================
SystemSnapshot NeoController::createSnapshot()
{
    SystemSnapshot s;

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        s.displayWidth = screen->geometry().width();
        s.displayHeight = screen->geometry().height();
        s.displayRefreshHz = static_cast<int>(screen->refreshRate());
    }

    s.xMultiplier = m_xMultiplier;
    s.yMultiplier = m_yMultiplier;
    s.curveId = m_curve;
    s.slowZone = m_slowZone;
    s.smoothingMs = m_smoothing;
    s.mouseDpi = 800;

    return s;
}

// ==========================
// Core Methods
// ==========================
void NeoController::applyOptimization()
{
    qDebug() << "[NeoController] Applying optimization with X:" << m_xMultiplier << "Y:" << m_yMultiplier;
    saveConfig();
    emit sensitivityChanged();
}

void NeoController::scanForDevices()
{
    startAdbCheck();
}

void NeoController::disconnectAdb()
{
    QString adb = getAdbPath();
    if (adb.isEmpty()) {
        qWarning() << "[ADB] Cannot disconnect: ADB not found";
        return;
    }

    qDebug() << "[ADB] Disconnecting all devices...";

    // Run adb disconnect (disconnects all TCP/IP devices)
    QProcess::execute(adb, {"disconnect"});

    // Kill the ADB server to fully disconnect
    QProcess::execute(adb, {"kill-server"});
    
    m_adbManualDisconnected = true;
    m_selectedDevice.clear();
    m_adbStatus = "Offline";
    emit devicesChanged();
    emit statusChanged();
}



void NeoController::runAdbCommand(const QString& command)
{
    QString adb = getAdbPath();
    if (adb.isEmpty()) {
        m_currentScriptLog += "\n[Error] ADB not found\n";
        emit scriptLogChanged();
        return;
    }

    if (command.isEmpty()) {
        m_currentScriptLog += "\n[Error] Empty command\n";
        emit scriptLogChanged();
        return;
    }

    QString device = m_selectedDevice.isEmpty() ? "" : m_selectedDevice;
    QStringList args;
    
    if (!device.isEmpty()) {
        args << "-s" << device;
    }
    args << "shell" << command;

    QProcess* proc = new QProcess(this);
    
    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc]() {
        QString output = QString::fromUtf8(proc->readAllStandardOutput());
        m_currentScriptLog += output;
        emit scriptLogChanged();
        emit scriptOutputReceived(output);
    });

    connect(proc, &QProcess::readyReadStandardError, this, [this, proc]() {
        QString err = QString::fromUtf8(proc->readAllStandardError());
        m_currentScriptLog += "[stderr] " + err;
        m_currentScriptError += err;
        emit scriptLogChanged();
    });

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, command](int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            m_currentScriptLog += "\n[Success] Command completed\n";
        } else {
            m_currentScriptLog += "\n[Failed] Exit code: " + QString::number(exitCode) + "\n";
        }
        emit scriptLogChanged();
        emit scriptFinished(-1, exitCode);
        proc->deleteLater();
    });

    m_currentScriptLog += "\n$ adb shell " + command + "\n";
    emit scriptLogChanged();
    
    proc->start(adb, args);
}

void NeoController::setSensitivity(double x, double y, QString curve, int slowZone, int smoothing)
{
    m_xMultiplier = x;
    m_yMultiplier = y;
    m_curve = curve;
    m_slowZone = slowZone;
    m_smoothing = smoothing;
    scheduleSave();
    emit sensitivityChanged();
}

// ==========================
// Setters
// ==========================
void NeoController::setAiEnabled(bool enabled)
{
    if (m_aiEnabled != enabled) {
        m_aiEnabled = enabled;
        scheduleSave();
        emit aiEnabledChanged();
    }
}

void NeoController::setAiConfidenceThreshold(double threshold)
{
    if (!qFuzzyCompare(m_aiConfidenceThreshold, threshold)) {
        m_aiConfidenceThreshold = threshold;
        scheduleSave();
        emit aiEnabledChanged();
    }
}

void NeoController::setSelectedDevice(const QString& device)
{
    if (m_selectedDevice != device) {
        m_selectedDevice = device;
        m_adbManualDisconnected = false; // User explicitly selected a device
        scheduleSave();
        emit devicesChanged();
    }
}

void NeoController::setMouseDpi(int dpi)
{
    // Clamp DPI to valid range (100-16000)
    if (dpi < 100) dpi = 100;
    if (dpi > 16000) dpi = 16000;
    
    // Try to set real hardware DPI via Logitech HID++
    if (m_logitechHID && m_logitechHID->isConnected()) {
        if (m_logitechHID->setDpi(dpi)) {
            // Real DPI set successfully - signal will update m_mouseDpi
            scheduleSave();
            qDebug() << "[NeoController] Real mouse DPI set to:" << dpi;
            return;
        } else {
            qDebug() << "[NeoController] Hardware DPI control unavailable, using Windows cursor speed";
        }
    }
    
    // Apply Windows cursor speed control
    // Windows mouse speed range is 1-20, with 10 being default
    // Map DPI range (100-16000) to Windows speed (1-20)
    // Lower DPI = Lower speed, Higher DPI = Higher speed
#ifdef Q_OS_WIN
    // Calculate Windows mouse speed based on DPI
    // DPI 100-400 = Speed 1-5 (slow)
    // DPI 400-800 = Speed 5-10 (default range)
    // DPI 800-1600 = Speed 10-15 (fast)
    // DPI 1600-16000 = Speed 15-20 (very fast)
    int winSpeed;
    if (dpi <= 400) {
        winSpeed = 1 + (dpi - 100) * 4 / 300;  // 1-5
    } else if (dpi <= 800) {
        winSpeed = 5 + (dpi - 400) * 5 / 400;  // 5-10
    } else if (dpi <= 1600) {
        winSpeed = 10 + (dpi - 800) * 5 / 800;  // 10-15
    } else {
        winSpeed = 15 + (dpi - 1600) * 5 / 14400;  // 15-20
    }
    
    // Clamp to valid range
    winSpeed = qBound(1, winSpeed, 20);
    
    // Apply Windows mouse speed using SystemParametersInfo
    BOOL success = SystemParametersInfoW(SPI_SETMOUSESPEED, 0, (LPVOID)(intptr_t)winSpeed, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    if (success) {
        qDebug() << "[NeoController] Windows cursor speed set to" << winSpeed << "(DPI:" << dpi << ")";
    } else {
        qWarning() << "[NeoController] Failed to set Windows cursor speed";
    }
#endif
    
    // Store DPI value locally
    if (m_mouseDpi != dpi) {
        m_mouseDpi = dpi;
        
        if (auto* pipeline = InputHookManager::instance().pipeline()) {
            pipeline->setMouseDpi(dpi);
        }
        
        scheduleSave();
        emit sensitivityChanged();
        qDebug() << "[NeoController] Mouse DPI saved:" << dpi;
    }
}

// ==========================
// AI Methods
// ==========================
void NeoController::setXMultiplier(double value)
{
    // Clamp to valid range (-1 to +1 for center-zero)
    value = qBound(-1.0, value, 1.0);
    
    if (!qFuzzyCompare(m_xMultiplier, value)) {
        m_xMultiplier = value;
        scheduleSave();
        
        if (auto* pipeline = InputHookManager::instance().pipeline()) {
            pipeline->setAxisMultiplierX(value);
        }
        
        emit sensitivityChanged();
        qDebug() << "[NeoController] X Multiplier set to:" << value;
    }
}

void NeoController::setYMultiplier(double value)
{
    // Clamp to valid range (-1 to +1 for center-zero)
    value = qBound(-1.0, value, 1.0);
    
    if (!qFuzzyCompare(m_yMultiplier, value)) {
        m_yMultiplier = value;
        scheduleSave();
        
        if (auto* pipeline = InputHookManager::instance().pipeline()) {
            pipeline->setAxisMultiplierY(value);
        }
        
        emit sensitivityChanged();
        qDebug() << "[NeoController] Y Multiplier set to:" << value;
    }
}

// ==========================
// AI Analysis Methods
// ==========================
void NeoController::runAiAnalysis()
{
    if (m_aiProcessing) return;

    m_aiProcessing = true;
    emit aiStatusChanged();

    SystemSnapshot snapshot = createSnapshot();
    m_aiAdvisor->requestTuning(snapshot, nullptr);
}

void NeoController::acceptRecommendation()
{
    if (m_hasRecommendation) {
        m_xMultiplier = m_recommendedX;
        m_yMultiplier = m_recommendedY;
        m_hasRecommendation = false;
        scheduleSave();
        emit sensitivityChanged();
        emit recommendationChanged();
    }
}

void NeoController::declineRecommendation()
{
    m_hasRecommendation = false;
    emit recommendationChanged();
}

void NeoController::setGeminiApiKey(const QString &apiKey)
{
    m_aiAdvisor->setApiKey(apiKey);
}

// ==========================
// AI Callback Handlers
// ==========================
void NeoController::onRecommendationReady(const TuningRecommendation &rec)
{
    m_aiProcessing = false;

    // Gate by validity + confidence
    if (!rec.isValid || rec.confidence < m_aiConfidenceThreshold) {
        m_hasRecommendation = false;
        m_lastRecommendationSummary = "Recommendation confidence too low";
        emit aiStatusChanged();
        emit recommendationChanged();
        return;
    }

    // Apply recommendation data
    m_hasRecommendation = true;
    m_recommendedX = rec.xMultiplier;
    m_recommendedY = rec.yMultiplier;
    m_recommendationSeverity = rec.severity;
    m_recommendationConfidence = rec.confidence;
    m_lastRecommendationSummary = rec.reasoning.join(" ");

    emit aiStatusChanged();
    emit recommendationChanged();

    // Emit UI-friendly signal
    emit recommendationReady(
        m_lastRecommendationSummary,
        rec.xMultiplier,
        rec.yMultiplier,
        rec.severity
    );
}

void NeoController::onAiError(const QString &error)
{
    m_aiProcessing = false;
    m_aiStatus = "AI Error: " + error;
    emit aiStatusChanged();
    qWarning() << "[AI] Error:" << error;
}

void NeoController::setTheme(int theme)
{
    if (m_theme != theme && theme >= 0 && theme <= 2) {  // 0=DarkMinimal, 1=GlassGradient, 2=FrostGlass
        m_theme = theme;
        scheduleSave();
        emit themeChanged();
        qDebug() << "[NeoController] Theme saved:" << theme;
    }
}

// ==========================
// Input Hook Control
// ==========================
bool NeoController::inputHookActive() const
{
    return InputHookManager::instance().isHookActive();
}

void NeoController::toggleInputHook()
{
    auto& hookManager = InputHookManager::instance();
    
    if (hookManager.isHookActive()) {
        hookManager.stopHook();
        m_inputStatus = "Inactive";
        qDebug() << "[NeoController] Input hook stopped";
    } else {
        // Pass current X/Y multipliers to the hook before starting
        if (auto* pipeline = hookManager.pipeline()) {
            pipeline->setAxisMultiplierX(m_xMultiplier);
            pipeline->setAxisMultiplierY(m_yMultiplier);
        }
        hookManager.startHook();
        
        // Connect pipeline telemetry
        if (auto* pipeline = hookManager.pipeline()) {
            connect(pipeline, &NeoZ::SensitivityPipeline::inputProcessed, 
                    this, &NeoController::onInputProcessed, Qt::UniqueConnection);
        }

        m_inputStatus = "Active";
        qDebug() << "[NeoController] Input hook started with multipliers X=" << m_xMultiplier << "Y=" << m_yMultiplier;
    }
    
    emit inputHookChanged();
    emit statusChanged();
}

void NeoController::onInputProcessed(const NeoZ::InputState& state)
{
    // Calculate velocity magnitude
    double v = std::sqrt(state.deltaX * state.deltaX + state.deltaY * state.deltaY);
    
    // Calculate angle in degrees (0-360)
    double angle = 0.0;
    if (v > 0.1) {
        angle = std::atan2(state.deltaY, state.deltaX) * 180.0 / M_PI;
        if (angle < 0) angle += 360.0;
    }
    
    // Update pending values (Max hold for velocity to catch peaks, current for angle)
    if (v > m_pendingVelocity) m_pendingVelocity = v;
    else m_pendingVelocity *= 0.95; // Decay
    
    m_pendingAngle = angle;
}

void NeoController::updateTelemetry()
{
    bool changed = false;
    
    if (std::abs(m_mouseVelocity - m_pendingVelocity) > 0.1) {
        m_mouseVelocity = m_pendingVelocity;
        changed = true;
    }
    
    if (std::abs(m_mouseAngleDegrees - m_pendingAngle) > 0.5) {
        m_mouseAngleDegrees = m_pendingAngle;
        changed = true;
    }
    
    if (changed) {
        emit telemetryChanged();
    }
}

int NeoController::presetConfidence() const
{
    if (auto* pipeline = InputHookManager::instance().pipeline()) {
        return pipeline->hostNormalizer()->presetConfidence();
    }
    return 0; // Default to Native
}

bool NeoController::inputAuthorityEnabled() const
{
    return InputHookManager::instance().isHookActive();
}

void NeoController::setInputAuthorityEnabled(bool enabled)
{
    if (inputAuthorityEnabled() != enabled) {
        toggleInputHook();
    }
}



// ==========================
// DRCS - Directional Repetition Constraint System
// ==========================
bool NeoController::drcsEnabled() const
{
    return m_drcs ? m_drcs->isEnabled() : false;
}

void NeoController::setDrcsEnabled(bool enabled)
{
    if (m_drcs) {
        m_drcs->setEnabled(enabled);
    }
}

double NeoController::drcsRepetitionTolerance() const
{
    return m_drcs ? m_drcs->repetitionTolerance() : 4.0;
}

void NeoController::setDrcsRepetitionTolerance(double value)
{
    if (m_drcs) {
        m_drcs->setRepetitionTolerance(value);
    }
}

double NeoController::drcsDirectionThreshold() const
{
    return m_drcs ? m_drcs->directionThreshold() : 0.95;
}

void NeoController::setDrcsDirectionThreshold(double value)
{
    if (m_drcs) {
        m_drcs->setDirectionThreshold(value);
    }
}

double NeoController::drcsSuppressionLevel() const
{
    return m_drcs ? m_drcs->currentSuppression() : 1.0;
}

// ==========================
// Identify Emulators (Aggressive Scan)
// ==========================
void NeoController::identifyEmulators()
{
    scanForInstalledEmulators(); // Also scan for installed apps

    QString adb = getAdbPath();
    if (adb.isEmpty()) return;

    // Common emulator ports
    // 5555: BlueStacks / Default
    // 5557-5585: BlueStacks instances (+2)
    // 62001: Nox
    // 62025: Nox instance
    // 21503: LDPlayer
    // 7555: MuMu
    QList<int> ports = {5555, 5557, 5559, 5561, 5563, 62001, 62025, 21503, 7555};

    for (int port : ports) {
        auto *p = new QProcess(this);
        // Fire and forget connect attempts
        connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [p]() {
            p->deleteLater();
        });
        p->start(adb, {"connect", QString("127.0.0.1:%1").arg(port)});
    }

    // Trigger a scan after a short delay to allow connections to establish
    QTimer::singleShot(1500, this, &NeoController::scanForDevices);
}

// ==========================
// Installed Emulator Management
// ==========================
void NeoController::scanForInstalledEmulators()
{
    m_installedEmulators.clear();

    // Define known emulators and their paths
    struct EmuDef { QString name; QString path; QString icon; };
    QList<EmuDef> definitions = {
        {"BlueStacks", "C:/Program Files/BlueStacks_nxt/HD-Player.exe", "🟦"},
        {"BlueStacks 5", "C:/Program Files/BlueStacks/HD-Player.exe", "🟦"},
        {"NoxPlayer", "C:/Program Files/Nox/bin/Nox.exe", "🟣"},
        {"LDPlayer 9", "C:/LDPlayer/LDPlayer9/dnplayer.exe", "🟡"},
        {"LDPlayer 4", "C:/LDPlayer/LDPlayer4.0/dnplayer.exe", "🟡"},
        {"MuMu Player", "C:/Program Files/MuMu/emulator/nemu/vmonitor/bin/nemu.exe", "🟠"},
        {"MuMu Player 12", "C:/Program Files/Netease/MuMuPlayer-12.0/shell/MuMuPlayer.exe", "🟠"},
        {"MEmu", "C:/Program Files/Microvirt/MEmu/MEmu.exe", "🟢"}
    };

    for (const auto& def : definitions) {
        if (QFileInfo::exists(def.path)) {
            m_installedEmulators.append({def.name, def.path, def.icon});
            qDebug() << "[Emulator] Found installed:" << def.name << "at" << def.path;
        }
    }

    emit installedEmulatorsChanged();
}

void NeoController::launchEmulator(const QString& path)
{
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        qWarning() << "[Emulator] Cannot launch, invalid path:" << path;
        return;
    }

    qDebug() << "[Emulator] Launching:" << path;
    QProcess::startDetached(path, QStringList());
}

QVariantList NeoController::installedEmulators() const
{
    QVariantList list;
    for (const auto& emu : m_installedEmulators) {
        QVariantMap map;
        map["name"] = emu.name;
        map["path"] = emu.path;
        map["icon"] = emu.icon;
        list.append(map);
    }
    return list;
}

// ==========================
// Crosshair Detection Aim Assist
// ==========================
bool NeoController::aimAssistActive() const
{
    return m_crosshairDetector ? m_crosshairDetector->aimAssistActive() : false;
}

bool NeoController::crosshairDetectionEnabled() const
{
    return m_crosshairDetector ? m_crosshairDetector->enabled() : false;
}

void NeoController::setCrosshairDetectionEnabled(bool enabled)
{
    if (!m_crosshairDetector) {
        m_crosshairDetector = new NeoZ::CrosshairDetector(this);
        
        // Setup ADB path and device
        m_crosshairDetector->setAdbPath(getAdbPath());
        m_crosshairDetector->setDeviceId(m_selectedDevice);
        
        // Connect aim assist state change to Y sensitivity adjustment
        connect(m_crosshairDetector, &NeoZ::CrosshairDetector::aimAssistStateChanged, 
                this, [this](bool active) {
            qDebug() << "[NeoController] Aim assist:" << (active ? "ACTIVE (reducing Y)" : "INACTIVE");
            emit aimAssistStateChanged();
            
            // When aim assist active, reduce Y multiplier by alpha
            if (active && m_crosshairDetector) {
                double alpha = m_crosshairDetector->yReductionAlpha();
                double adjustedY = m_yMultiplier * (1.0 - alpha);
                InputHookManager::instance().setMultipliers(m_xMultiplier, adjustedY);
            } else {
                InputHookManager::instance().setMultipliers(m_xMultiplier, m_yMultiplier);
            }
        });
    }
    
    // Update ADB path and device before starting
    m_crosshairDetector->setAdbPath(getAdbPath());
    m_crosshairDetector->setDeviceId(m_selectedDevice);
    m_crosshairDetector->setEnabled(enabled);
    
    emit aimAssistStateChanged();
}

double NeoController::aimAssistYReduction() const
{
    return m_crosshairDetector ? m_crosshairDetector->yReductionAlpha() : 0.2;
}

void NeoController::setAimAssistYReduction(double alpha)
{
    if (m_crosshairDetector) {
        m_crosshairDetector->setYReductionAlpha(alpha);
        emit aimAssistStateChanged();
    }
}

// ==========================
// Snapshot/Rollback
// ==========================
void NeoController::takeSnapshot()
{
    m_snapshot.xMultiplier = m_xMultiplier;
    m_snapshot.yMultiplier = m_yMultiplier;
    m_snapshot.slowZone = m_slowZone;
    m_snapshot.smoothing = m_smoothing;
    m_snapshot.mouseDpi = m_mouseDpi;
    m_hasSnapshot = true;
    emit snapshotChanged();
    qDebug() << "[NeoController] Snapshot taken: X=" << m_xMultiplier << "Y=" << m_yMultiplier;
}

void NeoController::rollback()
{
    if (!m_hasSnapshot) return;
    
    m_xMultiplier = m_snapshot.xMultiplier;
    m_yMultiplier = m_snapshot.yMultiplier;
    m_slowZone = m_snapshot.slowZone;
    m_smoothing = m_snapshot.smoothing;
    m_mouseDpi = m_snapshot.mouseDpi;
    
    scheduleSave();
    
    // Update pipeline with restored values
    if (auto* pipeline = InputHookManager::instance().pipeline()) {
        pipeline->setAxisMultiplierX(m_xMultiplier);
        pipeline->setAxisMultiplierY(m_yMultiplier);
        pipeline->setSmoothingMs(static_cast<double>(m_smoothing));
        pipeline->setMouseDpi(m_mouseDpi);
    }
    
    emit sensitivityChanged();
    qDebug() << "[NeoController] Rollback to: X=" << m_xMultiplier << "Y=" << m_yMultiplier;
}

// ==========================
// Velocity Curve
// ==========================
int NeoController::velocityCurvePreset() const
{
    return m_velocityCurve ? static_cast<int>(m_velocityCurve->preset()) : 0;
}

void NeoController::setVelocityCurvePreset(int preset)
{
    if (!m_velocityCurve) {
        m_velocityCurve = new NeoZ::VelocityCurve(this);
    }
    m_velocityCurve->setPreset(static_cast<NeoZ::VelocityCurve::CurvePreset>(preset));
    emit velocityCurveChanged();
}

double NeoController::velocityLowThreshold() const
{
    return m_velocityCurve ? m_velocityCurve->lowThreshold() : 0.5;
}

void NeoController::setVelocityLowThreshold(double v)
{
    if (!m_velocityCurve) {
        m_velocityCurve = new NeoZ::VelocityCurve(this);
    }
    m_velocityCurve->setLowThreshold(v);
    emit velocityCurveChanged();
    
    if (auto* pipeline = InputHookManager::instance().pipeline()) {
        if (auto* curve = pipeline->velocityCurve()) {
            curve->setLowThreshold(v);
        }
    }
}

double NeoController::velocityHighThreshold() const
{
    return m_velocityCurve ? m_velocityCurve->highThreshold() : 5.0;
}

void NeoController::setVelocityHighThreshold(double v)
{
    if (!m_velocityCurve) {
        m_velocityCurve = new NeoZ::VelocityCurve(this);
    }
    m_velocityCurve->setHighThreshold(v);
    emit velocityCurveChanged();
    
    if (auto* pipeline = InputHookManager::instance().pipeline()) {
        if (auto* curve = pipeline->velocityCurve()) {
            curve->setHighThreshold(v);
        }
    }
}

double NeoController::velocityLowMultiplier() const
{
    return m_velocityCurve ? m_velocityCurve->lowMultiplier() : 0.8;
}

void NeoController::setVelocityLowMultiplier(double v)
{
    if (!m_velocityCurve) {
        m_velocityCurve = new NeoZ::VelocityCurve(this);
    }
    m_velocityCurve->setLowMultiplier(v);
    emit velocityCurveChanged();
    
    if (auto* pipeline = InputHookManager::instance().pipeline()) {
        if (auto* curve = pipeline->velocityCurve()) {
            curve->setLowMultiplier(v);
        }
    }
}

double NeoController::velocityHighMultiplier() const
{
    return m_velocityCurve ? m_velocityCurve->highMultiplier() : 1.2;
}

void NeoController::setVelocityHighMultiplier(double v)
{
    if (!m_velocityCurve) {
        m_velocityCurve = new NeoZ::VelocityCurve(this);
    }
    m_velocityCurve->setHighMultiplier(v);
    emit velocityCurveChanged();
    
    if (auto* pipeline = InputHookManager::instance().pipeline()) {
        if (auto* curve = pipeline->velocityCurve()) {
            curve->setHighMultiplier(v);
        }
    }
}

// ==========================
// Sensitivity Setters
// ==========================
void NeoController::setSmoothing(int value)
{
    if (m_smoothing == value) return;
    m_smoothing = value;
    
    // Forward to SensitivityPipeline
    if (auto* pipeline = InputHookManager::instance().pipeline()) {
        pipeline->setSmoothingMs(static_cast<double>(value));
    }
    
    scheduleSave();
    emit sensitivityChanged();
    qDebug() << "[NeoController] Smoothing set to:" << value << "ms";
}
