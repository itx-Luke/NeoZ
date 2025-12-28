#ifndef INPUTHOOKMANAGER_H
#define INPUTHOOKMANAGER_H

#include <QObject>
#include "../sensitivity/SensitivityPipeline.h"

// Include Windows headers AFTER Qt/STL to avoid template conflicts
#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class InputHookManager : public QObject
{
    Q_OBJECT
public:
    static InputHookManager& instance(); // Singleton pattern for the static callback

    void startHook();
    void stopHook();
    bool isHookActive() const { return m_hook != nullptr && m_active; }

    // Access to the pipeline for configuration
    NeoZ::SensitivityPipeline* pipeline() const { return m_pipeline; }

    // Legacy setters (forwarded to pipeline for compatibility)
    void setMultipliers(double x, double y);
    void setSmoothingMs(double ms);
    void setVelocityCurve(double lowThresh, double highThresh, double lowMult, double highMult);

signals:
    void mouseEventDetected(int dx, int dy); // For analytics UI

private:
    InputHookManager(QObject *parent = nullptr);
    ~InputHookManager();

    // The static callback required by Windows
#ifdef Q_OS_WIN
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    HHOOK m_hook = nullptr;
#else
    void* m_hook = nullptr;
#endif

    NeoZ::SensitivityPipeline* m_pipeline = nullptr;
    
    // Position tracking for delta calculation
    int m_lastX = 0;
    int m_lastY = 0;
    bool m_firstMove = true;
    bool m_active = false;
};

#endif // INPUTHOOKMANAGER_H