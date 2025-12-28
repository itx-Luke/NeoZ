# NeoZ ADB Service - Separate executable for ADB operations
# Communicates with Core via TCP on port 5557

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(ADB_SERVICE_SOURCES
    main.cpp
    AdbService.h
    AdbService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/adb/AdbConnection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/adb/AdbConnection.cpp
)

qt_add_executable(NeoZ_AdbService
    ${ADB_SERVICE_SOURCES}
)

target_include_directories(NeoZ_AdbService PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
)

target_link_libraries(NeoZ_AdbService PRIVATE
    Qt6::Core
    Qt6::Network
)

# Install alongside main executable
install(TARGETS NeoZ_AdbService
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
