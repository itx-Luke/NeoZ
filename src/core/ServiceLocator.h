#ifndef NEOZ_SERVICELOCATOR_H
#define NEOZ_SERVICELOCATOR_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QMutex>
#include <typeinfo>
#include <memory>

namespace NeoZ {

/**
 * @brief Service Locator for dependency injection.
 * 
 * Provides a centralized registry for service instances, allowing
 * loose coupling between components.
 * 
 * Usage:
 *   // Registration (at startup)
 *   ServiceLocator::provide<InputManager>(new InputManager());
 *   ServiceLocator::provide<DeviceManager>(new DeviceManager());
 *   
 *   // Retrieval (anywhere in code)
 *   auto* input = ServiceLocator::get<InputManager>();
 *   auto* device = ServiceLocator::get<DeviceManager>();
 *   
 *   // With custom key
 *   ServiceLocator::provide("primary_ai", new AiManager());
 *   auto* ai = ServiceLocator::get<AiManager>("primary_ai");
 */
class ServiceLocator
{
public:
    // ========== TYPE-BASED REGISTRATION (Recommended) ==========
    
    /**
     * @brief Register a service using its type as the key
     * @tparam T Service type (must inherit from QObject)
     * @param service Service instance to register
     */
    template<typename T>
    static void provide(T* service)
    {
        static_assert(std::is_base_of_v<QObject, T>, "Service must inherit from QObject");
        QMutexLocker locker(&mutex());
        services()[typeName<T>()] = service;
    }
    
    /**
     * @brief Retrieve a service by its type
     * @tparam T Service type
     * @return Pointer to the service, or nullptr if not registered
     */
    template<typename T>
    static T* get()
    {
        QMutexLocker locker(&mutex());
        QObject* obj = services().value(typeName<T>(), nullptr);
        return qobject_cast<T*>(obj);
    }
    
    /**
     * @brief Check if a service is registered
     * @tparam T Service type
     * @return true if service is registered
     */
    template<typename T>
    static bool has()
    {
        QMutexLocker locker(&mutex());
        return services().contains(typeName<T>());
    }
    
    /**
     * @brief Remove a service from the registry
     * @tparam T Service type
     */
    template<typename T>
    static void remove()
    {
        QMutexLocker locker(&mutex());
        services().remove(typeName<T>());
    }
    
    // ========== STRING-KEY REGISTRATION (Flexible) ==========
    
    /**
     * @brief Register a service with a custom string key
     * @param key Unique identifier for the service
     * @param service Service instance
     */
    static void provide(const QString& key, QObject* service)
    {
        QMutexLocker locker(&mutex());
        services()[key] = service;
    }
    
    /**
     * @brief Retrieve a service by string key
     * @tparam T Expected service type
     * @param key Service key
     * @return Pointer to the service, or nullptr if not found
     */
    template<typename T>
    static T* get(const QString& key)
    {
        QMutexLocker locker(&mutex());
        QObject* obj = services().value(key, nullptr);
        return qobject_cast<T*>(obj);
    }
    
    /**
     * @brief Check if a service with the given key exists
     * @param key Service key
     * @return true if registered
     */
    static bool has(const QString& key)
    {
        QMutexLocker locker(&mutex());
        return services().contains(key);
    }
    
    /**
     * @brief Remove a service by key
     * @param key Service key
     */
    static void remove(const QString& key)
    {
        QMutexLocker locker(&mutex());
        services().remove(key);
    }
    
    // ========== LIFECYCLE ==========
    
    /**
     * @brief Clear all registered services
     * 
     * Note: Does NOT delete the service objects. Caller is responsible
     * for managing service lifetimes.
     */
    static void clear()
    {
        QMutexLocker locker(&mutex());
        services().clear();
    }
    
    /**
     * @brief Get all registered service keys
     * @return List of service keys
     */
    static QStringList keys()
    {
        QMutexLocker locker(&mutex());
        return services().keys();
    }
    
private:
    ServiceLocator() = delete; // Static-only class
    
    template<typename T>
    static QString typeName()
    {
        return QString::fromLatin1(typeid(T).name());
    }
    
    static QHash<QString, QObject*>& services()
    {
        static QHash<QString, QObject*> s_services;
        return s_services;
    }
    
    static QMutex& mutex()
    {
        static QMutex s_mutex;
        return s_mutex;
    }
};

// ========== CONVENIENCE MACROS ==========

// Register a service (typically in main.cpp or Core initialization)
#define NEOZ_PROVIDE_SERVICE(Type, Instance) \
    NeoZ::ServiceLocator::provide<Type>(Instance)

// Get a service (anywhere in code)
#define NEOZ_GET_SERVICE(Type) \
    NeoZ::ServiceLocator::get<Type>()

// Check if service exists
#define NEOZ_HAS_SERVICE(Type) \
    NeoZ::ServiceLocator::has<Type>()

} // namespace NeoZ

#endif // NEOZ_SERVICELOCATOR_H
