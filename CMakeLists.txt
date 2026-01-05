cmake_minimum_required(VERSION 3.16...3.30)

project(Neo-Z VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC requires /Zc:__cplusplus to correctly report C++ standard version
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

# Added 'Gui' component here for Windows API/Screen detection features
# Added 'Network' component for Gemini API HTTP requests

# --- FALLBACK FOR IDE/MANUAL BUILDS ---
# If CMAKE_PREFIX_PATH is not set (e.g. by presets), add the known Qt6 path.
if(NOT DEFINED CMAKE_PREFIX_PATH)
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.10.1/mingw_64")
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Gui Network WebView Multimedia)

# --- POLICIES ---
# Fixes warnings about resource prefixes and QML subdirectories
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)
# ----------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Fetch hidapi for Logitech HID++ support ---
# Set policy version minimum to handle hidapi's older CMakeLists
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version" FORCE)
include(FetchContent)
FetchContent_Declare(
    hidapi
    GIT_REPOSITORY https://github.com/libusb/hidapi.git
    GIT_TAG        hidapi-0.14.0
    EXCLUDE_FROM_ALL  # Prevent hidapi from being the default build target
)
FetchContent_MakeAvailable(hidapi)

# Ensure hidapi targets don't interfere with default build
# Note: 'hidapi' is an ALIAS target, so only set properties on real targets
if(TARGET hidapi_winapi)
    set_target_properties(hidapi_winapi PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
if(TARGET hidapi_hidraw)
    set_target_properties(hidapi_hidraw PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

# --- C++ SOURCES ---
set(PROJECT_SOURCES
    src/main.cpp
    src/backend/NeoController.h
    src/backend/NeoController.cpp
    src/core/adb/AdbConnector.h
    src/core/adb/AdbConnector.cpp
    src/core/ai/GeminiClient.h
    src/core/ai/GeminiClient.cpp
    src/core/ai/AiAdvisor.h
    src/core/ai/AiAdvisor.cpp
    
    # Input Pipeline Layer
    src/core/input/InputState.h
    src/core/input/InputHook.h
    src/core/input/InputHook.cpp
    src/core/input/WindowsInputReader.h
    src/core/input/WindowsInputReader.cpp
    
    # Sensitivity Pipeline Layer
    src/core/sensitivity/VelocityCurve.h
    src/core/sensitivity/VelocityCurve.cpp
    src/core/sensitivity/HostNormalizer.h
    src/core/sensitivity/HostNormalizer.cpp
    src/core/sensitivity/EmulatorTranslator.h
    src/core/sensitivity/EmulatorTranslator.cpp
    src/core/sensitivity/SensitivityCalculator.h
    src/core/sensitivity/SensitivityCalculator.cpp
    src/core/sensitivity/SensitivityPipeline.h
    src/core/sensitivity/SensitivityPipeline.cpp
    
    # Logitech HID++ DPI Control
    src/core/input/LogitechHID.h
    src/core/input/LogitechHID.cpp
    
    # DRCS - Directional Repetition Constraint System
    src/core/sensitivity/DRCS.h
    src/core/sensitivity/DRCS.cpp
    
    # Logging System
    src/core/logging/Logger.h
    src/core/logging/Logger.cpp
    
    # Crosshair Detection (Aim Assist State)
    src/core/aim/CrosshairDetector.h
    src/core/aim/CrosshairDetector.cpp
    
    # High-Performance ADB Connection
    src/core/adb/AdbConnection.h
    src/core/adb/AdbConnection.cpp
    
    # Fast Configuration System
    src/core/config/FastConfig.h
    src/core/config/FastConfig.cpp
    
    # High-Performance Utilities (header-only)
    src/core/perf/FastConf.hpp
    src/core/perf/RealTimeSensitivityAI.hpp
    
    # IPC System (Core side)
    src/core/ipc/IpcServer.h
    src/core/ipc/IpcServer.cpp
    
    # IPC System (UI side) - included here for single-exe build
    src/ui/ipc/IpcClient.h
    src/ui/ipc/IpcClient.cpp
    
    # Manager Classes (refactored from NeoController)
    src/core/managers/InputManager.h
    src/core/managers/InputManager.cpp
    src/core/managers/SensitivityManager.h
    src/core/managers/SensitivityManager.cpp
    src/core/managers/AiManager.h
    src/core/managers/AiManager.cpp
    src/core/managers/DeviceManager.h
    src/core/managers/DeviceManager.cpp
    
    # Service Locator (Dependency Injection)
    src/core/ServiceLocator.h
    src/core/Services.h
)

# --- Windows Manifest for UAC and High Priority ---
if(WIN32)
    set(WIN_MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/src/app/Neo-Z.manifest)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${WIN_MANIFEST})
endif()

# --- ADB Service (separate executable) ---
add_subdirectory(src/adb_service)

# --- Zereca Control Plane ---
add_subdirectory(src/zereca)

qt_add_executable(appNeo-Z
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# --- Make appNeo-Z the default build target ---
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT appNeo-Z)
# For non-VS generators, set as ALL target default
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_custom_target(build_all ALL DEPENDS appNeo-Z)

# --- Windows-specific libraries ---
if(WIN32)
    target_link_libraries(appNeo-Z PRIVATE winmm advapi32 setupapi)
endif()

# --- QML MODULE ---
# Mark Style.qml and ThemeManager.qml as Singletons BEFORE adding to module
set_source_files_properties(src/ui/style/Style.qml PROPERTIES QT_QML_SINGLETON_TYPE true)
set_source_files_properties(src/ui/style/ThemeManager.qml PROPERTIES QT_QML_SINGLETON_TYPE true)
set_source_files_properties(src/ui/style/CyberpunkColors.qml PROPERTIES QT_QML_SINGLETON_TYPE true)
# TEMPORARILY DISABLED - debugging crash
# set_source_files_properties(src/ui/singletons/AppSettings.qml PROPERTIES QT_QML_SINGLETON_TYPE true)
# set_source_files_properties(src/ui/singletons/AppState.qml PROPERTIES QT_QML_SINGLETON_TYPE true)

qt_add_qml_module(appNeo-Z
    URI "NeoZ"
    VERSION 1.0
    QML_FILES
        # --- Root ---
        src/ui/Main.qml

        # --- Singletons ---
        src/ui/style/Style.qml
        src/ui/style/ThemeManager.qml
        # TEMPORARILY DISABLED - debugging crash
        # src/ui/singletons/AppSettings.qml
        # src/ui/singletons/AppState.qml

        # --- Components ---
        src/ui/components/GlassPanel.qml
        src/ui/components/AnimatedBackground.qml
        src/ui/components/NeonButton.qml
        src/ui/components/GlassComboBox.qml
        src/ui/components/NeonSlider.qml
        src/ui/components/NeonToggle.qml
        src/ui/components/StatusPill.qml
        src/ui/components/StatusTile.qml
        src/ui/components/RadarChart.qml
        src/ui/components/IconLabel.qml
        src/ui/components/EmulatorScanDialog.qml
        src/ui/components/NeoLoader.qml
        src/ui/components/GalaxyBackground.qml
        src/ui/components/GlassSlider.qml
        src/ui/components/RadialSlider.qml
        src/ui/components/MonitorOptimizationWindow.qml
        
        # --- Cyberpunk Optimization Components ---
        src/ui/components/HealthRing.qml
        src/ui/components/ThreatRadar.qml
        src/ui/components/EventLog.qml
        src/ui/components/GlowButton.qml
        src/ui/components/ProfileCard.qml
        src/ui/components/OptimizationTile.qml
        src/ui/components/OrbitingOrb.qml
        src/ui/style/CyberpunkColors.qml

        # --- Core NeoZ Theme-Aware Components ---
        src/ui/components/core/NeoZText.qml
        src/ui/components/core/NeoZButton.qml
        src/ui/components/core/NeoZSlider.qml
        src/ui/components/core/NeoZToggle.qml
        src/ui/components/core/NeoZCheckbox.qml
        src/ui/components/core/NeoZPanel.qml
        src/ui/components/core/NeoZProgressBar.qml
        src/ui/components/core/NeoZCard.qml
        src/ui/components/core/NeoZComboBox.qml

        # --- Views (Screens) ---
        src/ui/views/GlobalStatusHud.qml
        src/ui/views/Sidebar.qml
        src/ui/views/DashboardView.qml

        src/ui/views/SettingsView.qml
        src/ui/views/HistoryView.qml
        src/ui/views/ScriptRunnerView.qml
        src/ui/views/AdvancedAiView.qml
    
    RESOURCES
        assets/adb.svg
        assets/ai.svg
        assets/brain.svg
        assets/dashboard.svg
        assets/display.svg
        assets/emulator.svg
        assets/history.svg
        assets/input.svg
        assets/script.svg
        assets/sensitivity.svg
        assets/settings.svg
        assets/frost_background.mp4

        QML_FILES src/ui/views/SensitivityView.qml
)

# --- LIBRARIES ---
target_link_libraries(appNeo-Z
    PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Gui     # Required for backend screen resolution checks
    Qt6::Network # Required for Gemini API HTTP requests
    Qt6::WebView # Required for Spline 3D embedding
    Qt6::Multimedia # Required for video background
    hidapi       # Required for Logitech HID++ DPI control
    zereca       # Zereca adaptive performance control plane
)

include(GNUInstallDirs)
install(TARGETS appNeo-Z
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(appNeo-Z)

# --- TESTS ---
option(BUILD_TESTING "Build unit tests" OFF)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# --- OPTIMIZER (Separate Application) ---
add_subdirectory(src/optimizer)

