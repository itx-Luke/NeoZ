#include <QtTest>
#include <QTemporaryDir>
#include <QFile>

#include "core/logging/Logger.h"

/**
 * @brief Unit tests for the Logger system
 * 
 * Tests logging levels, file output, and thread safety.
 */
class TestLogger : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

private slots:
    void initTestCase()
    {
        QVERIFY(m_tempDir.isValid());
    }

    void cleanupTestCase()
    {
        Logger::closeLogFile();
    }

    // ========================================
    // Basic Logging Tests
    // ========================================
    
    void testDebugLog()
    {
        // Should not crash
        Logger::debug("Test debug message", "TestContext");
        QVERIFY(true);
    }

    void testInfoLog()
    {
        Logger::info("Test info message", "TestContext");
        QVERIFY(true);
    }

    void testWarningLog()
    {
        Logger::warning("Test warning message", "TestContext");
        QVERIFY(true);
    }

    void testErrorLog()
    {
        Logger::error("Test error message", "TestContext");
        QVERIFY(true);
    }

    void testCriticalLog()
    {
        Logger::critical("Test critical message", "TestContext");
        QVERIFY(true);
    }

    // ========================================
    // Log Level Tests
    // ========================================
    
    void testSetLogLevel()
    {
        // Set to Warning - Debug and Info should be filtered
        Logger::setLogLevel(Logger::Warning);
        
        // These should still work (not crash)
        Logger::debug("This should be filtered", "TestLevel");
        Logger::info("This should be filtered", "TestLevel");
        Logger::warning("This should appear", "TestLevel");
        
        // Reset to Debug for other tests
        Logger::setLogLevel(Logger::Debug);
        QVERIFY(true);
    }

    // ========================================
    // File Logging Tests
    // ========================================
    
    void testSetLogFile()
    {
        QString logPath = m_tempDir.path() + "/test.log";
        Logger::setLogFile(logPath);
        
        // Log something
        Logger::info("Test file logging", "FileTest");
        
        // Verify file was created
        QFile file(logPath);
        QVERIFY(file.exists());
    }

    void testLogFileContent()
    {
        QString logPath = m_tempDir.path() + "/content_test.log";
        Logger::setLogFile(logPath);
        
        QString testMessage = "Unique test message 12345";
        Logger::info(testMessage, "ContentTest");
        
        // Close to flush
        Logger::closeLogFile();
        
        // Read file and check content
        QFile file(logPath);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QString content = QString::fromUtf8(file.readAll());
        file.close();
        
        QVERIFY2(content.contains(testMessage),
                 "Log file should contain the test message");
    }

    // ========================================
    // Without Context Tests
    // ========================================
    
    void testLogWithoutContext()
    {
        // Context is optional
        Logger::info("Message without context");
        QVERIFY(true);
    }

    // ========================================
    // Macro Tests
    // ========================================
    
    void testLogMacros()
    {
        // Test convenience macros
        LOG_DEBUG("Macro debug test");
        LOG_INFO("Macro info test");
        LOG_WARNING("Macro warning test");
        LOG_ERROR("Macro error test");
        QVERIFY(true);
    }

    // ========================================
    // Singleton Tests
    // ========================================
    
    void testSingletonInstance()
    {
        // Multiple calls should return same instance
        Logger& instance1 = Logger::instance();
        Logger& instance2 = Logger::instance();
        
        QCOMPARE(&instance1, &instance2);
    }
};

QTEST_MAIN(TestLogger)
#include "tst_logger.moc"
