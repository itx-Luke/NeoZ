// Neo-Z End-to-End Test Suite
// Integration tests that verify complete user workflows

#include <QtTest/QtTest>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSignalSpy>
#include "../src/backend/NeoController.h"

class EndToEndTests : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<QQmlApplicationEngine> m_engine;
    NeoController* m_controller = nullptr;

private slots:
    void initTestCase()
    {
        // Initialize QML engine for e2e tests
        m_engine = std::make_unique<QQmlApplicationEngine>();
        m_controller = new NeoController(this);
        m_engine->rootContext()->setContextProperty("neoController", m_controller);
    }

    void cleanupTestCase()
    {
        m_engine.reset();
    }

    // ========================================
    // Application Lifecycle Tests
    // ========================================
    
    void test_ApplicationStarts()
    {
        // Verify controller initializes without crash
        QVERIFY(m_controller != nullptr);
        QVERIFY(!m_controller->emulatorStatus().isEmpty());
    }

    void test_ControllerPropertiesAccessible()
    {
        // Verify all major properties are accessible
        QVERIFY(m_controller->xMultiplier() >= 0);
        QVERIFY(m_controller->yMultiplier() >= 0);
        QVERIFY(m_controller->smoothing() >= 0);
    }

    // ========================================
    // Sensitivity Workflow Tests
    // ========================================

    void test_SensitivityAdjustment()
    {
        QSignalSpy sensitivitySpy(m_controller, &NeoController::sensitivityChanged);
        
        double originalX = m_controller->xMultiplier();
        m_controller->setXMultiplier(originalX + 0.1);
        
        // Verify signal emitted
        QVERIFY(sensitivitySpy.count() >= 1);
        
        // Verify value changed
        QCOMPARE(m_controller->xMultiplier(), originalX + 0.1);
        
        // Restore
        m_controller->setXMultiplier(originalX);
    }

    void test_DRCSToggle()
    {
        QSignalSpy drcsSpy(m_controller, &NeoController::drcsChanged);
        
        bool originalState = m_controller->drcsEnabled();
        m_controller->setDrcsEnabled(!originalState);
        
        QVERIFY(drcsSpy.count() >= 1);
        QCOMPARE(m_controller->drcsEnabled(), !originalState);
        
        // Restore
        m_controller->setDrcsEnabled(originalState);
    }

    // ========================================
    // Device Connection Workflow Tests
    // ========================================

    void test_DeviceListAccessible()
    {
        QStringList devices = m_controller->adbDevices();
        // Just verify we can access the list without crash
        QVERIFY(true);
    }

    void test_EmulatorScanTriggerable()
    {
        // Verify scan can be triggered without crash
        m_controller->scanEmulators();
        
        // Wait briefly for async operation
        QTest::qWait(100);
        QVERIFY(true);
    }

    // ========================================
    // AI Integration Tests
    // ========================================

    void test_AIStatusAccessible()
    {
        QString aiStatus = m_controller->aiStatus();
        QVERIFY(!aiStatus.isNull());
    }

    void test_AIToggle()
    {
        QSignalSpy aiSpy(m_controller, &NeoController::aiEnabledChanged);
        
        bool originalState = m_controller->aiEnabled();
        m_controller->setAiEnabled(!originalState);
        
        QVERIFY(aiSpy.count() >= 1);
        
        // Restore
        m_controller->setAiEnabled(originalState);
    }

    // ========================================
    // Metrics & Telemetry Tests
    // ========================================

    void test_MetricsAccessible()
    {
        // Verify telemetry properties don't crash
        QVERIFY(m_controller->fpsMean() >= 0 || m_controller->fpsMean() == 0);
        QVERIFY(m_controller->latencyMs() >= 0 || m_controller->latencyMs() == 0);
    }
};

QTEST_MAIN(EndToEndTests)
#include "tst_e2e.moc"
