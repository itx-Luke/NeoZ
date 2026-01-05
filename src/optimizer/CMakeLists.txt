# NeoZ Optimizer - Standalone QML GUI application
# Launched from Core via Backend.launchOptimizer()

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2)

# Create the executable
qt_add_executable(NeoZ_Optimizer
    main.cpp
    OptimizerBackend.h
    OptimizerBackend.cpp
    ZerecaBridgeAdapter.h
    ZerecaBridgeAdapter.cpp
)

# Add QML module to the executable
qt_add_qml_module(NeoZ_Optimizer
    URI NeoZOptimizer
    VERSION 1.0
    QML_FILES
        Main.qml
        # Zereca UI Components
        ui/ZerecaView.qml
        ui/ZerecaDashboard.qml
        ui/ZerecaLearning.qml
        ui/ZerecaSafety.qml
        ui/ZerecaLogs.qml
        ui/ZerecaUIPanel.qml
)

target_link_libraries(NeoZ_Optimizer PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    zereca  # Zereca control plane library
)

# Link Windows-specific libraries for system optimization APIs
if(WIN32)
    target_link_libraries(NeoZ_Optimizer PRIVATE
        psapi      # Process Status API
        pdh        # Performance Data Helper
        powrprof   # Power Profile API
    )
endif()

# Set WIN32_EXECUTABLE to hide console window on Windows
# Output to same directory as main app
set_target_properties(NeoZ_Optimizer PROPERTIES
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Install alongside main executable
install(TARGETS NeoZ_Optimizer
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


