#include "InputManager.h"
#include <QDebug>
#include <cmath>

namespace NeoZ {

InputManager::InputManager(QObject* parent)
    : QObject(parent)
{
    // Connect to pipeline signals when hook is active
    if (auto* pipe = pipeline()) {
        connect(pipe, &SensitivityPipeline::inputProcessed, 
                this, &InputManager::onInputProcessed, Qt::UniqueConnection);
    }
}

InputManager::~InputManager()
{
    stopHook();
}

void InputManager::startHook()
{
    auto& hookManager = InputHookManager::instance();
    
    if (hookManager.isHookActive()) {
        qDebug() << "[InputManager] Hook already active";
        return;
    }
    
    hookManager.startHook();
    
    // Connect to pipeline
    if (auto* pipe = hookManager.pipeline()) {
        connect(pipe, &SensitivityPipeline::inputProcessed, 
                this, &InputManager::onInputProcessed, Qt::UniqueConnection);
    }
    
    m_status = "Active";
    emit hookStateChanged();
    emit statusChanged();
    qDebug() << "[InputManager] Hook started";
}

void InputManager::stopHook()
{
    auto& hookManager = InputHookManager::instance();
    
    if (!hookManager.isHookActive()) {
        return;
    }
    
    hookManager.stopHook();
    m_status = "Idle";
    emit hookStateChanged();
    emit statusChanged();
    qDebug() << "[InputManager] Hook stopped";
}

void InputManager::toggleHook()
{
    if (isHookActive()) {
        stopHook();
    } else {
        startHook();
    }
}

bool InputManager::isHookActive() const
{
    return InputHookManager::instance().isHookActive();
}

SensitivityPipeline* InputManager::pipeline() const
{
    return InputHookManager::instance().pipeline();
}

void InputManager::setAxisMultiplierX(double value)
{
    if (auto* pipe = pipeline()) {
        pipe->setAxisMultiplierX(value);
    }
}

void InputManager::setAxisMultiplierY(double value)
{
    if (auto* pipe = pipeline()) {
        pipe->setAxisMultiplierY(value);
    }
}

void InputManager::setSmoothingMs(double ms)
{
    if (auto* pipe = pipeline()) {
        pipe->setSmoothingMs(ms);
    }
}

void InputManager::setMouseDpi(int dpi)
{
    if (auto* pipe = pipeline()) {
        pipe->setMouseDpi(dpi);
    }
}

void InputManager::onInputProcessed(const InputState& input)
{
    // Update telemetry
    m_mouseVelocity = input.velocity;
    m_mouseAngleDegrees = std::atan2(input.deltaY, input.deltaX) * 180.0 / M_PI;
    
    // Track latency
    m_latencyTracker.add(input.velocity > 0);
    
    emit telemetryChanged();
    emit inputProcessed(input.deltaX, input.deltaY, input.velocity);
}

} // namespace NeoZ
