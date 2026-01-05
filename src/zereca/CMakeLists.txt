# Zereca - Adaptive Performance Control Plane
# Part of NeoZ

cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Core)

# Define the Zereca library
set(ZERECA_SOURCES
    # Types
    types/ZerecaTypes.cpp
    types/ContextHash.cpp
    
    # Core (System A - Enforcement)
    core/TargetState.cpp
    core/StateReconciler.cpp
    core/FlightRecorder.cpp
    core/EmergencyRollback.cpp
    core/TelemetryReader.cpp
    
    # Arbiter (System C - Safety)
    arbiter/ProbationLedger.cpp
    arbiter/OptimizationArbiter.cpp
    arbiter/OutcomeClassifier.cpp
    
    # Policy (System B - Learning)
    policy/EmulatorDetector.cpp
    policy/ObservationPhase.cpp
    policy/HypothesisEngine.cpp
    policy/ShadowMode.cpp
    
    # Controller (QML Bridge)
    ZerecaController.cpp
)

set(ZERECA_HEADERS
    # Types
    types/ZerecaTypes.h
    types/ContextHash.h
    
    # Core (System A)
    core/TargetState.h
    core/StateReconciler.h
    core/FlightRecorder.h
    core/EmergencyRollback.h
    core/TelemetryReader.h
    
    # Arbiter (System C)
    arbiter/ProbationLedger.h
    arbiter/OptimizationArbiter.h
    arbiter/OutcomeClassifier.h
    
    # Policy (System B)
    policy/EmulatorDetector.h
    policy/ObservationPhase.h
    policy/HypothesisEngine.h
    policy/ShadowMode.h
)

add_library(zereca STATIC ${ZERECA_SOURCES} ${ZERECA_HEADERS})

target_include_directories(zereca PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(zereca PUBLIC
    Qt6::Core
)

# Windows-specific libraries for System A enforcement
if(WIN32)
    target_link_libraries(zereca PRIVATE
        PowrProf      # Power management
        pdh           # Performance counters
        dxgi          # GPU info
        psapi         # Process enumeration
    )
endif()

# Enable automoc for Qt integration
set_target_properties(zereca PROPERTIES
    AUTOMOC ON
)

message(STATUS "Zereca module configured")
message(STATUS "  - System A (Enforcement): 5 components")
message(STATUS "  - System B (Policy): 4 components")
message(STATUS "  - System C (Arbiter): 3 components")
