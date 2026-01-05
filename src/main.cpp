#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWebView>
#include <QStandardPaths>

#include "backend/NeoController.h"
#include "core/logging/Logger.h"
#include "core/Services.h"
#include "core/config/FastConfig.h"
#include <QQuickStyle>

#ifdef Q_OS_WIN
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif

/**
 * @brief Set process to high priority for smooth input processing
 * 
 * This is critical for:
 * - Accurate mouse input capture
 * - Consistent sensitivity application
 * - Minimal input latency
 */
void setupHighPriorityAccess()
{
#ifdef Q_OS_WIN
    // Set process priority to HIGH (not REALTIME to avoid system instability)
    HANDLE hProcess = GetCurrentProcess();
    if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS)) {
        Logger::info("Process priority set to HIGH_PRIORITY_CLASS", "System");
    } else {
        Logger::warning("Failed to set high priority - running as normal priority", "System");
    }
    
    // Request 1ms timer resolution for accurate timing
    // This helps with input polling and smooth operation
    MMRESULT result = timeBeginPeriod(1);
    if (result == TIMERR_NOERROR) {
        Logger::info("Timer resolution set to 1ms", "System");
    } else {
        Logger::warning("Failed to set 1ms timer resolution", "System");
    }
    
    // Log if running as administrator
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    if (isAdmin) {
        Logger::info("Running with Administrator privileges", "System");
    } else {
        Logger::info("Running with standard user privileges", "System");
    }
#endif
}

/**
 * @brief Cleanup high priority settings on exit
 */
void cleanupHighPriorityAccess()
{
#ifdef Q_OS_WIN
    // Restore default timer resolution
    timeEndPeriod(1);
    Logger::info("Timer resolution restored", "System");
#endif
}

int main(int argc, char *argv[])
{
    // Initialize QtWebView (must be called before QGuiApplication)
    QtWebView::initialize();
    
    QGuiApplication app(argc, argv);
    
    // Set application metadata
    app.setOrganizationName("NeoZ");
    app.setApplicationName("Neo-Z");
    app.setApplicationVersion("0.1");
    
    // Initialize logging system
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/neo-z.log";
    Logger::setLogFile(logPath);
#ifdef QT_DEBUG
    Logger::setLogLevel(Logger::Debug);
#else
    Logger::setLogLevel(Logger::Info);
#endif
    Logger::info("Neo-Z starting up", "Main");
    Logger::info(QString("Log file: %1").arg(logPath), "Main");
    
    // Setup high priority access for smooth input processing
    setupHighPriorityAccess();
    
    // Cleanup on exit
    QObject::connect(&app, &QCoreApplication::aboutToQuit, []() {
        cleanupHighPriorityAccess();
        Logger::info("Neo-Z shutting down", "Main");
        Logger::closeLogFile();
    });

    QQuickStyle::setStyle("Basic");
    Logger::info("QuickStyle set to Basic", "Main");
    
    // ========== Initialize FastConfig V3 ==========
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/neo-z.ini";
    NeoZ::initGlobalConfig(configPath);
    
    // Enable V3 features for production
    if (NeoZ::globalConfig()) {
        NeoZ::globalConfig()->setFlushThreshold(100);     // Snapshot every 100 writes
        NeoZ::globalConfig()->setFlushDelay(500);         // 500ms debounce before disk write
        NeoZ::globalConfig()->setCrashSafeWrites(true);   // Atomic write (temp + rename)
        NeoZ::globalConfig()->setBackupEnabled(true);     // Create .bak file
    }
    Logger::info(QString("FastConfig V3 initialized: %1").arg(configPath), "Main");
    
    // Initialize all core services (managers)
    Logger::info("Initializing services...", "Main");
    NeoZ::Services::initialize(&app);
    Logger::info("Services initialized", "Main");
    
    // Cleanup services and config on exit
    QObject::connect(&app, &QCoreApplication::aboutToQuit, []() {
        NeoZ::Services::shutdown();
        NeoZ::destroyGlobalConfig();  // Flush and cleanup FastConfig
    });
    
    // Register C++ backend types for QML access
    Logger::info("Registering NeoController...", "Main");
    qmlRegisterSingletonInstance("NeoZ", 1, 0, "Backend", new NeoController(&app));
    Logger::info("NeoController registered", "Main");
    
    QQmlApplicationEngine engine;
    Logger::info("QQmlApplicationEngine created", "Main");
    
    // Load the main QML file from the module
    const QUrl url(QStringLiteral("qrc:/qt/qml/NeoZ/src/ui/Main.qml"));
    Logger::info(QString("Loading QML from: %1").arg(url.toString()), "Main");
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { 
            Logger::error("QML object creation FAILED!", "Main");
            QCoreApplication::exit(-1); 
        },
        Qt::QueuedConnection);
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [](QObject *obj, const QUrl &objUrl) {
            if (!obj) {
                Logger::error("QML object is NULL after creation!", "Main");
            } else {
                Logger::info("QML object created successfully", "Main");
            }
        },
        Qt::QueuedConnection);
    
    engine.load(url);
    
    if (engine.rootObjects().isEmpty()) {
        Logger::error("No root objects after engine.load() - QML loading failed!", "Main");
        return -1;
    }
    
    Logger::info("UI loaded successfully", "Main");

    return app.exec();
}
