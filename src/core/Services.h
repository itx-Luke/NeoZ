#ifndef NEOZ_SERVICES_H
#define NEOZ_SERVICES_H

#include "ServiceLocator.h"
#include "managers/InputManager.h"
#include "managers/SensitivityManager.h"
#include "managers/AiManager.h"
#include "managers/DeviceManager.h"
#include "ipc/IpcServer.h"
#include "config/FastConfig.h"

namespace NeoZ {

/**
 * @brief Typed service accessors for common services.
 * 
 * Provides convenient, typed access to registered services
 * without needing to use ServiceLocator directly.
 * 
 * Usage:
 *   auto* input = Services::input();
 *   auto* device = Services::device();
 *   auto* sensitivity = Services::sensitivity();
 */
class Services
{
public:
    // ========== MANAGER ACCESSORS ==========
    
    static InputManager* input()
    {
        return ServiceLocator::get<InputManager>();
    }
    
    static SensitivityManager* sensitivity()
    {
        return ServiceLocator::get<SensitivityManager>();
    }
    
    static AiManager* ai()
    {
        return ServiceLocator::get<AiManager>();
    }
    
    static DeviceManager* device()
    {
        return ServiceLocator::get<DeviceManager>();
    }
    
    // ========== INFRASTRUCTURE ACCESSORS ==========
    
    static IpcServer* ipcServer()
    {
        return ServiceLocator::get<IpcServer>();
    }
    
    static FastConfig* config()
    {
        return globalConfig();
    }
    
    // ========== INITIALIZATION ==========
    
    /**
     * @brief Initialize all core services
     * @param parent Parent QObject for memory management
     * 
     * Call this once at application startup (typically in main.cpp)
     */
    static void initialize(QObject* parent = nullptr)
    {
        // Create and register all managers
        ServiceLocator::provide<InputManager>(new InputManager(parent));
        ServiceLocator::provide<SensitivityManager>(new SensitivityManager(parent));
        ServiceLocator::provide<AiManager>(new AiManager(parent));
        ServiceLocator::provide<DeviceManager>(new DeviceManager(parent));
        ServiceLocator::provide<IpcServer>(new IpcServer(parent));
    }
    
    /**
     * @brief Shutdown all services
     * 
     * Call this at application exit. Services will be deleted
     * by their parent QObject if one was provided.
     */
    static void shutdown()
    {
        // Clear the registry (objects owned by parent)
        ServiceLocator::clear();
    }
    
private:
    Services() = delete; // Static-only class
};

} // namespace NeoZ

#endif // NEOZ_SERVICES_H
