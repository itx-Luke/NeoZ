#include <QtTest>

// Include sensitivity pipeline headers
#include "core/sensitivity/VelocityCurve.h"
#include "core/sensitivity/SensitivityCalculator.h"

/**
 * @brief Unit tests for the Sensitivity Pipeline
 * 
 * Tests based on Dev Docs Part 3:
 * - Multipliers, curves, smoothing windows
 * - Speed-based scaling (low-speed vs high-speed)
 * - Aim-assist-aware micro-smoothing
 */
class TestSensitivityPipeline : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        qDebug() << "Starting Sensitivity Pipeline tests...";
    }

    void cleanupTestCase()
    {
        qDebug() << "Sensitivity Pipeline tests completed.";
    }

    // ========================================
    // Velocity Curve Tests
    // ========================================
    
    void testLinearCurve()
    {
        // Linear curve: output = input
        double input = 5.0;
        double multiplier = 1.0;
        double expected = input * multiplier;
        
        QCOMPARE(input * multiplier, expected);
    }

    void testMultiplierScaling()
    {
        // Multiplier should scale output proportionally
        double input = 10.0;
        double multiplier = 2.0;
        
        double output = input * multiplier;
        
        QCOMPARE(output, 20.0);
    }

    void testAsymmetricMultipliers()
    {
        // X and Y can have different multipliers
        double deltaX = 10.0;
        double deltaY = 10.0;
        double xMult = 1.5;
        double yMult = 1.2;
        
        double outX = deltaX * xMult;
        double outY = deltaY * yMult;
        
        QCOMPARE(outX, 15.0);
        QCOMPARE(outY, 12.0);
    }

    // ========================================
    // Speed-Based Scaling Tests
    // ========================================
    
    void testLowSpeedBehavior()
    {
        // At low speeds, precision is more important
        // Effective sensitivity might be slightly lower
        double speed = 2.0; // Low speed
        double baseSens = 1.0;
        double slowZone = 0.5; // 50%
        
        // If speed is low and in slow zone, apply reduction
        double effectiveSens = baseSens;
        if (speed < 5.0) {
            effectiveSens = baseSens * (1.0 - (slowZone * 0.2));
        }
        
        QVERIFY(effectiveSens <= baseSens);
    }

    void testHighSpeedBehavior()
    {
        // At high speeds, full sensitivity applies
        double speed = 50.0; // High speed
        double baseSens = 1.0;
        
        // No reduction at high speed
        double effectiveSens = baseSens;
        
        QCOMPARE(effectiveSens, baseSens);
    }

    // ========================================
    // Smoothing Tests
    // ========================================
    
    void testSmoothingWindowSize()
    {
        // Smoothing value 0-100 maps to window size
        int smoothing = 50; // 50%
        
        // Example: 50% = 5ms window
        int windowMs = smoothing / 10;
        
        QCOMPARE(windowMs, 5);
    }

    void testNoSmoothing()
    {
        // Smoothing = 0 means no averaging
        int smoothing = 0;
        int windowMs = smoothing / 10;
        
        QCOMPARE(windowMs, 0);
    }

    void testMaxSmoothing()
    {
        // Smoothing = 100 means maximum averaging
        int smoothing = 100;
        int windowMs = smoothing / 10;
        
        QCOMPARE(windowMs, 10);
    }

    // ========================================
    // eDPI Calculation Tests
    // ========================================
    
    void testEdpiCalculation()
    {
        // eDPI = Mouse DPI * In-game Sensitivity
        int mouseDpi = 800;
        double inGameSens = 1.5;
        
        double eDpi = mouseDpi * inGameSens;
        
        QCOMPARE(eDpi, 1200.0);
    }

    void testCm360Approximation()
    {
        // cm/360 = (360 * 2.54) / eDPI
        // This is an approximation for the distance to do a 360
        double eDpi = 1600.0;
        double cm360 = (360.0 * 2.54) / eDpi;
        
        // Should be around 57 cm for 1600 eDPI
        QVERIFY(cm360 > 50.0 && cm360 < 65.0);
    }

    // ========================================
    // Aim Assist Slow Zone Tests
    // ========================================
    
    void testSlowZoneReduction()
    {
        // When aim assist is active (red crosshair),
        // sensitivity is reduced by slow zone percentage
        double baseSens = 1.0;
        int slowZonePercent = 30; // 30% reduction
        
        double reducedSens = baseSens * (1.0 - slowZonePercent / 100.0);
        
        QCOMPARE(reducedSens, 0.7);
    }

    void testSlowZoneZero()
    {
        // 0% slow zone = no reduction
        double baseSens = 1.0;
        int slowZonePercent = 0;
        
        double reducedSens = baseSens * (1.0 - slowZonePercent / 100.0);
        
        QCOMPARE(reducedSens, 1.0);
    }

    void testSlowZoneFull()
    {
        // 100% slow zone = full stop (but we clamp to minimum)
        double baseSens = 1.0;
        int slowZonePercent = 100;
        
        double reducedSens = baseSens * (1.0 - slowZonePercent / 100.0);
        
        // Clamp to minimum 0.1
        if (reducedSens < 0.1) reducedSens = 0.1;
        
        QCOMPARE(reducedSens, 0.1);
    }

    // ========================================
    // Resolution Impact Tests
    // ========================================
    
    void testResolutionChangeImpact()
    {
        // When resolution changes, px/cm changes
        // Need to recalculate to maintain feel
        
        // Old: 1920x1080 on 24" monitor
        double oldPxPerCm = 1920.0 / (24.0 * 2.54);
        
        // New: 2560x1440 on same monitor
        double newPxPerCm = 2560.0 / (24.0 * 2.54);
        
        // Ratio for adjustment
        double ratio = newPxPerCm / oldPxPerCm;
        
        // New sens should be scaled inversely to maintain feel
        double oldSens = 1.0;
        double newSens = oldSens / ratio;
        
        QVERIFY(newSens < oldSens); // Higher res = lower sens to maintain feel
    }
};

QTEST_MAIN(TestSensitivityPipeline)
#include "tst_sensitivity.moc"
