#ifndef ZERECA_CONTEXT_HASH_H
#define ZERECA_CONTEXT_HASH_H

#include "ZerecaTypes.h"
#include <QString>

namespace Zereca {

/**
 * @brief Utility class for generating and managing system context.
 * 
 * The context hash uniquely identifies the current system environment.
 * Probation entries are scoped to their context - a configuration that
 * failed under one context may be retried after a context shift.
 */
class ContextHash
{
public:
    /**
     * @brief Capture the current system context.
     * @return SystemContext populated with current GPU driver, OS build, etc.
     */
    static SystemContext capture();
    
    /**
     * @brief Get the GPU driver version.
     * @return Driver version as uint64 (packed major.minor.build)
     */
    static uint64_t getGpuDriverVersion();
    
    /**
     * @brief Get the Windows OS build number.
     * @return OS build (e.g., 22631 for Win11 23H2)
     */
    static uint64_t getOsBuild();
    
    /**
     * @brief Get the BIOS/UEFI version.
     * @return BIOS version hash
     */
    static uint64_t getBiosVersion();
    
    /**
     * @brief Hash an executable file.
     * @param exePath Path to the executable
     * @return SHA256 hash of the first 64KB of the file
     */
    static uint64_t hashExecutable(const QString& exePath);
};

} // namespace Zereca

#endif // ZERECA_CONTEXT_HASH_H
