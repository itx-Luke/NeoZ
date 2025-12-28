# Neo-Z Unit Tests
cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Test Quick Gui Network)

enable_testing()

# Include main project source directory
set(PROJECT_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Common include directories for all tests
set(TEST_INCLUDE_DIRS
    ${PROJECT_SRC_DIR}
    ${PROJECT_SRC_DIR}/backend
    ${PROJECT_SRC_DIR}/core
)

# Common source files needed by tests
set(COMMON_SOURCES
    ${PROJECT_SRC_DIR}/core/logging/Logger.h
    ${PROJECT_SRC_DIR}/core/logging/Logger.cpp
    ${PROJECT_SRC_DIR}/core/sensitivity/DRCS.h
    ${PROJECT_SRC_DIR}/core/sensitivity/DRCS.cpp
    ${PROJECT_SRC_DIR}/core/sensitivity/VelocityCurve.h
    ${PROJECT_SRC_DIR}/core/sensitivity/VelocityCurve.cpp
    ${PROJECT_SRC_DIR}/core/sensitivity/SensitivityCalculator.h
    ${PROJECT_SRC_DIR}/core/sensitivity/SensitivityCalculator.cpp
)

# ========================================
# Test: NeoController Integration Tests
# ========================================
qt_add_executable(tst_neocontroller
    tst_neocontroller.cpp
    ${PROJECT_SRC_DIR}/backend/NeoController.h
    ${PROJECT_SRC_DIR}/backend/NeoController.cpp
    ${PROJECT_SRC_DIR}/core/ai/AiAdvisor.h
    ${PROJECT_SRC_DIR}/core/ai/AiAdvisor.cpp
    ${PROJECT_SRC_DIR}/core/ai/GeminiClient.h
    ${PROJECT_SRC_DIR}/core/ai/GeminiClient.cpp
    ${PROJECT_SRC_DIR}/core/input/InputHook.h
    ${PROJECT_SRC_DIR}/core/input/InputHook.cpp
    ${PROJECT_SRC_DIR}/core/input/WindowsInputReader.h
    ${PROJECT_SRC_DIR}/core/input/WindowsInputReader.cpp
    ${PROJECT_SRC_DIR}/core/input/LogitechHID.h
    ${PROJECT_SRC_DIR}/core/input/LogitechHID.cpp
    ${COMMON_SOURCES}
)

target_include_directories(tst_neocontroller PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(tst_neocontroller PRIVATE
    Qt6::Test
    Qt6::Quick
    Qt6::Gui
    Qt6::Network
)

if(WIN32)
    target_link_libraries(tst_neocontroller PRIVATE hidapi)
endif()

add_test(NAME tst_neocontroller COMMAND tst_neocontroller)

# ========================================
# Test: Logger Unit Tests
# ========================================
qt_add_executable(tst_logger
    tst_logger.cpp
    ${PROJECT_SRC_DIR}/core/logging/Logger.h
    ${PROJECT_SRC_DIR}/core/logging/Logger.cpp
)

target_include_directories(tst_logger PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(tst_logger PRIVATE Qt6::Test Qt6::Core)

add_test(NAME tst_logger COMMAND tst_logger)

# ========================================
# Test: Sensitivity Pipeline Tests
# ========================================
qt_add_executable(tst_sensitivity
    tst_sensitivity.cpp
    ${PROJECT_SRC_DIR}/core/sensitivity/VelocityCurve.h
    ${PROJECT_SRC_DIR}/core/sensitivity/VelocityCurve.cpp
    ${PROJECT_SRC_DIR}/core/sensitivity/SensitivityCalculator.h
    ${PROJECT_SRC_DIR}/core/sensitivity/SensitivityCalculator.cpp
)

target_include_directories(tst_sensitivity PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(tst_sensitivity PRIVATE Qt6::Test Qt6::Core)

add_test(NAME tst_sensitivity COMMAND tst_sensitivity)

# ========================================
# Test: DRCS Unit Tests
# ========================================
qt_add_executable(tst_drcs
    tst_drcs.cpp
    ${PROJECT_SRC_DIR}/core/sensitivity/DRCS.h
    ${PROJECT_SRC_DIR}/core/sensitivity/DRCS.cpp
)

target_include_directories(tst_drcs PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(tst_drcs PRIVATE Qt6::Test Qt6::Core)

add_test(NAME tst_drcs COMMAND tst_drcs)

# ========================================
# Summary
# ========================================
message(STATUS "Neo-Z Tests configured:")
message(STATUS "  - tst_neocontroller (Integration)")
message(STATUS "  - tst_logger (Unit)")
message(STATUS "  - tst_sensitivity (Unit)")
message(STATUS "  - tst_drcs (Unit)")
