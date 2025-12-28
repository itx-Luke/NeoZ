/**
 * @file main.cpp
 * @brief NeoZ ADB Service - Standalone executable for ADB operations.
 * 
 * This runs as a separate process from the main NeoZ application.
 * Core communicates with this service via TCP on port 5557.
 * 
 * Usage:
 *   NeoZ_AdbService.exe [--port 5557] [--adb-path /path/to/adb]
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "AdbService.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("NeoZ_AdbService");
    app.setApplicationVersion("1.0");
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("NeoZ ADB Service - Handles ADB communication for NeoZ");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption portOption(
        {"p", "port"},
        "TCP port to listen on (default: 5557)",
        "port",
        "5557"
    );
    parser.addOption(portOption);
    
    QCommandLineOption adbPathOption(
        {"a", "adb-path"},
        "Path to ADB executable (default: uses PATH)",
        "path",
        "adb"
    );
    parser.addOption(adbPathOption);
    
    parser.process(app);
    
    quint16 port = parser.value(portOption).toUShort();
    QString adbPath = parser.value(adbPathOption);
    
    // Create and start the service
    NeoZ::AdbService service;
    service.setAdbPath(adbPath);
    
    QObject::connect(&service, &NeoZ::AdbService::error, [](const QString& error) {
        qCritical() << "[AdbService] Error:" << error;
    });
    
    QObject::connect(&service, &NeoZ::AdbService::requestReceived, 
                     [](const QString& type, const QString& deviceId) {
        qDebug() << "[AdbService] Request:" << type << "device:" << deviceId;
    });
    
    if (!service.start(port)) {
        qCritical() << "Failed to start ADB service on port" << port;
        return 1;
    }
    
    qDebug() << "========================================";
    qDebug() << "NeoZ ADB Service v1.0";
    qDebug() << "Listening on port:" << port;
    qDebug() << "ADB path:" << adbPath;
    qDebug() << "========================================";
    qDebug() << "Press Ctrl+C to stop.";
    
    return app.exec();
}
