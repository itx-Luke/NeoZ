#include "InputHook.h"
#include <QDebug>
#include <cmath>

// Global instance pointer for the static callback
InputHookManager* g_hookInstance = nullptr;

InputHookManager& InputHookManager::instance()
{
    static InputHookManager instance;
    return instance;
}

InputHookManager::InputHookManager(QObject *parent) : QObject(parent)
{
    g_hookInstance = this;
    m_pipeline = new NeoZ::SensitivityPipeline(this);
    
    // Default settings
    m_pipeline->setInputAuthorityEnabled(true); // We control the input
    m_pipeline->setSafeZoneClampEnabled(true);
}

InputHookManager::~InputHookManager()
{
    stopHook();
}

void InputHookManager::setMultipliers(double x, double y)
{
    if (m_pipeline) {
        m_pipeline->setAxisMultiplierX(x);
        m_pipeline->setAxisMultiplierY(y);
        qDebug() << "[InputHook] Multipliers set via Pipeline: X=" << x << "Y=" << y;
    }
}

void InputHookManager::setSmoothingMs(double ms)
{
    if (m_pipeline) {
        m_pipeline->setSmoothingMs(ms);
        qDebug() << "[InputHook] Smoothing set via Pipeline:" << ms << "ms";
    }
}

void InputHookManager::setVelocityCurve(double lowThresh, double highThresh, double lowMult, double highMult)
{
    if (m_pipeline && m_pipeline->velocityCurve()) {
        auto* curve = m_pipeline->velocityCurve();
        curve->setLowThreshold(lowThresh);
        curve->setHighThreshold(highThresh);
        curve->setLowMultiplier(lowMult);
        curve->setHighMultiplier(highMult);
        qDebug() << "[InputHook] Velocity Curve updated via Pipeline";
    }
}

#ifdef Q_OS_WIN

void InputHookManager::startHook()
{
    if (m_hook) return;

    m_lastX = 0;
    m_lastY = 0;
    m_firstMove = true;
    m_active = true;

    m_hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), 0);
    
    if (m_hook) {
        qDebug() << "[InputHook] Mouse Hook Installed - Pipeline ACTIVE";
    } else {
        qWarning() << "[InputHook] Failed to install Mouse Hook. Error:" << GetLastError();
        m_active = false;
    }
}

void InputHookManager::stopHook()
{
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
        m_active = false;
        qDebug() << "[InputHook] Mouse Hook Removed";
    }
}

// THE CRITICAL INPUT LOOP - DELEGATES TO SENSITIVITY PIPELINE
LRESULT CALLBACK InputHookManager::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_MOUSEMOVE && g_hookInstance && g_hookInstance->m_active && g_hookInstance->m_pipeline) {
        MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
        
        // Skip if this is an injected event (to avoid infinite loop)
        if (pMouseStruct->flags & LLMHF_INJECTED) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        
        int currentX = pMouseStruct->pt.x;
        int currentY = pMouseStruct->pt.y;
        
        // First move - just store position
        if (g_hookInstance->m_firstMove) {
            g_hookInstance->m_lastX = currentX;
            g_hookInstance->m_lastY = currentY;
            g_hookInstance->m_firstMove = false;
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        
        // Calculate raw delta
        int deltaX = currentX - g_hookInstance->m_lastX;
        int deltaY = currentY - g_hookInstance->m_lastY;
        g_hookInstance->m_lastX = currentX;
        g_hookInstance->m_lastY = currentY;
        
        if (deltaX == 0 && deltaY == 0) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        
        // Prepare input state for pipeline
        NeoZ::InputState rawInput = NeoZ::InputState::fromRawDelta(
            static_cast<double>(deltaX),
            static_cast<double>(deltaY)
        );
        
        // Process through pipeline
        NeoZ::InputState processed = g_hookInstance->m_pipeline->process(rawInput);
        
        // Calculate extra movement to inject
        // The pipeline returns the DESIRED total movement.
        // We subtract the PHYSICAL movement (deltaX) to find what we need to ADD.
        int extraX = static_cast<int>(std::round(processed.deltaX)) - deltaX;
        int extraY = static_cast<int>(std::round(processed.deltaY)) - deltaY;
        
        if (extraX != 0 || extraY != 0) {
            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.dx = extraX;
            input.mi.dy = extraY;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(INPUT));
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

#else

// Stub implementations for non-Windows platforms
void InputHookManager::startHook()
{
    qWarning() << "[InputHook] Mouse hook not supported on this platform";
}

void InputHookManager::stopHook()
{
    qDebug() << "[InputHook] No hook to stop on this platform";
}

#endif