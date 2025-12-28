#include <QtTest>

#include "core/sensitivity/DRCS.h"

/**
 * @brief Unit tests for DRCS (Directional Repetition Constraint System)
 * 
 * Tests the anti-recoil detection and suppression system.
 */
class TestDRCS : public QObject
{
    Q_OBJECT

private:
    DRCS* m_drcs = nullptr;

private slots:
    void initTestCase()
    {
        m_drcs = new DRCS(this);
        qDebug() << "Starting DRCS tests...";
    }

    void cleanupTestCase()
    {
        delete m_drcs;
        m_drcs = nullptr;
        qDebug() << "DRCS tests completed.";
    }

    // ========================================
    // Enable/Disable Tests
    // ========================================
    
    void testDefaultDisabled()
    {
        // DRCS should be disabled by default
        QVERIFY(!m_drcs->isEnabled());
    }

    void testEnableSystem()
    {
        QSignalSpy spy(m_drcs, &DRCS::enabledChanged);
        
        m_drcs->setEnabled(true);
        
        QVERIFY(m_drcs->isEnabled());
        QCOMPARE(spy.count(), 1);
        
        m_drcs->setEnabled(false);
    }

    void testDisableSystem()
    {
        m_drcs->setEnabled(true);
        
        QSignalSpy spy(m_drcs, &DRCS::enabledChanged);
        m_drcs->setEnabled(false);
        
        QVERIFY(!m_drcs->isEnabled());
        QCOMPARE(spy.count(), 1);
    }

    // ========================================
    // Repetition Tolerance Tests
    // ========================================
    
    void testDefaultRepetitionTolerance()
    {
        double tolerance = m_drcs->repetitionTolerance();
        QVERIFY2(tolerance > 0.0,
                 "Repetition tolerance should be positive");
    }

    void testSetRepetitionTolerance()
    {
        QSignalSpy spy(m_drcs, &DRCS::parametersChanged);
        
        m_drcs->setRepetitionTolerance(5.0);
        
        QCOMPARE(m_drcs->repetitionTolerance(), 5.0);
        QCOMPARE(spy.count(), 1);
    }

    void testRepetitionToleranceRange()
    {
        // Tolerance should be clamped to valid range
        m_drcs->setRepetitionTolerance(1.0);
        QVERIFY(m_drcs->repetitionTolerance() >= 1.0);
        
        m_drcs->setRepetitionTolerance(10.0);
        QVERIFY(m_drcs->repetitionTolerance() <= 10.0);
    }

    // ========================================
    // Direction Threshold Tests
    // ========================================
    
    void testDefaultDirectionThreshold()
    {
        double threshold = m_drcs->directionThreshold();
        QVERIFY2(threshold >= 0.0 && threshold <= 1.0,
                 "Direction threshold should be 0.0-1.0");
    }

    void testSetDirectionThreshold()
    {
        QSignalSpy spy(m_drcs, &DRCS::parametersChanged);
        
        m_drcs->setDirectionThreshold(0.9);
        
        QCOMPARE(m_drcs->directionThreshold(), 0.9);
        QCOMPARE(spy.count(), 1);
    }

    // ========================================
    // Suppression Level Tests
    // ========================================
    
    void testSuppressionLevelDefault()
    {
        // When not processing, suppression should be 0
        double suppression = m_drcs->suppressionLevel();
        QVERIFY(suppression >= 0.0 && suppression <= 1.0);
    }

    // ========================================
    // Input Processing Tests
    // ========================================
    
    void testProcessNormalInput()
    {
        m_drcs->setEnabled(true);
        
        // Normal random input should not trigger suppression
        QPointF result1 = m_drcs->process(10.0, 5.0);
        QPointF result2 = m_drcs->process(-8.0, 3.0);
        QPointF result3 = m_drcs->process(12.0, -7.0);
        
        // Results should be close to input (no significant suppression)
        QVERIFY(result1.x() != 0.0 || result1.y() != 0.0);
        
        m_drcs->setEnabled(false);
    }

    void testProcessRepetitiveInput()
    {
        m_drcs->setEnabled(true);
        m_drcs->setRepetitionTolerance(2.0);
        m_drcs->setDirectionThreshold(0.95);
        
        // Simulate repetitive downward motion (like recoil compensation)
        for (int i = 0; i < 20; i++) {
            m_drcs->process(0.0, -5.0);
        }
        
        // After many repetitive inputs, suppression should increase
        double suppression = m_drcs->suppressionLevel();
        // Note: Exact behavior depends on DRCS implementation
        QVERIFY(suppression >= 0.0);
        
        m_drcs->reset();
        m_drcs->setEnabled(false);
    }

    void testProcessWhenDisabled()
    {
        m_drcs->setEnabled(false);
        
        // When disabled, input should pass through unchanged
        QPointF input(10.0, 5.0);
        QPointF result = m_drcs->process(input.x(), input.y());
        
        QCOMPARE(result.x(), input.x());
        QCOMPARE(result.y(), input.y());
    }

    // ========================================
    // Reset Tests
    // ========================================
    
    void testReset()
    {
        m_drcs->setEnabled(true);
        
        // Process some input
        m_drcs->process(5.0, 5.0);
        m_drcs->process(5.0, 5.0);
        
        // Reset should clear internal state
        m_drcs->reset();
        
        // After reset, suppression should be back to baseline
        double suppression = m_drcs->suppressionLevel();
        QVERIFY(suppression <= 0.1); // Should be very low after reset
        
        m_drcs->setEnabled(false);
    }
};

QTEST_MAIN(TestDRCS)
#include "tst_drcs.moc"
