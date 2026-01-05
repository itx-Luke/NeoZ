/**
 * @file main.cpp
 * @brief NeoZ Optimizer - Standalone QML GUI application.
 * 
 * This runs as a separate process from the main NeoZ application.
 * Launched when user clicks "Optimize Now" button.
 * 
 * Usage:
 *   NeoZ_Optimizer.exe
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDebug>
#include "OptimizerBackend.h"
#include "ZerecaBridgeAdapter.h"
#include "../zereca/ZerecaController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("NeoZ_Optimizer");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("NeoZ");
    
    // Set modern style
    QQuickStyle::setStyle("Basic");
    
    qDebug() << "========================================";
    qDebug() << "NeoZ Optimizer v2.0 + Zereca Control Plane";
    qDebug() << "========================================";
    
    // Create backends
    OptimizerBackend backend;
    Zereca::ZerecaController zerecaController;
    ZerecaBridgeAdapter bridge(&backend);
    
    QQmlApplicationEngine engine;
    
    // Register backends as singletons for QML access
    qmlRegisterSingletonInstance("NeoZOptimizer", 1, 0, "Optimizer", &backend);
    qmlRegisterSingletonInstance("NeoZOptimizer", 1, 0, "Zereca", &zerecaController);
    qmlRegisterSingletonInstance("NeoZOptimizer", 1, 0, "Bridge", &bridge);
    
    const QUrl url(QStringLiteral("qrc:/qt/qml/NeoZOptimizer/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl) {
                             qCritical() << "Failed to load QML!";
                             QCoreApplication::exit(-1);
                         }
                     }, Qt::QueuedConnection);
    
    engine.load(url);
    
    qDebug() << "Optimizer UI loaded successfully";
    qDebug() << "Zereca control plane initialized";
    
    return app.exec();
}

