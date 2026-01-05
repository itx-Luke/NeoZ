import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import NeoZ
import "views"
import "components"
import "style"

Window {
    id: mainWindow
    width: 1350
    height: 800
    minimumWidth: 1350
    maximumWidth: 1350
    minimumHeight: 800
    maximumHeight: 800
    visible: true
    title: "Neo-Z — Emulator Sensitivity Control Suite"
    color: Style.mainBgColor  // Theme-reactive background

    // --- LOADER STATE ---
    property bool loaderComplete: false
    property bool elementsRevealed: false

    // Theme changes are handled by ThemeManager.setTheme()
    // Components animate their own transitions via Behavior on color/opacity

    // --- NEO LOADER (Startup Splash) ---
    NeoLoader {
        id: neoLoader
        anchors.fill: parent
        z: 1000
        visible: !loaderComplete

        onStartReveal: {
            // Trigger UI element animations
            elementsRevealed = true;
            sidebarRevealAnim.start();
            hudRevealAnim.start();
            contentRevealAnim.start();
        }

        onFinished: {
            loaderComplete = true;
            console.log("NEO-Z: System initialized.");
        }
    }

    // --- MAIN CONTENT (loads behind the loader) ---
    Item {
        id: mainContent
        anchors.fill: parent
        visible: true  // Always visible, elements animate in

        AnimatedBackground {
            anchors.fill: parent
            z: -1
            // Only show video after boot (NeoLoader has its own paused video)
            isBootComplete: loaderComplete
        }

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // --- SIDEBAR (slides in from left) ---
            Sidebar {
                id: sidebar
                Layout.fillHeight: true

                // Start off-screen
                opacity: 0
                transform: Translate {
                    id: sidebarTranslate
                    x: -sidebar.width
                }

                ParallelAnimation {
                    id: sidebarRevealAnim

                    NumberAnimation {
                        target: sidebar
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 500
                        easing.type: Easing.OutCubic
                    }
                    NumberAnimation {
                        target: sidebarTranslate
                        property: "x"
                        from: -sidebar.width
                        to: 0
                        duration: 600
                        easing.type: Easing.OutCubic
                    }
                }
            }

            // --- CONTENT AREA ---
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // Global Status HUD (slides down from top)
                GlobalStatusHud {
                    id: statusHud
                    Layout.fillWidth: true
                    Layout.margins: 20
                    Layout.bottomMargin: 0

                    // Start above screen
                    opacity: 0
                    transform: Translate {
                        id: hudTranslate
                        y: -80
                    }

                    onAdbClicked: emulatorScanDialog.show()

                    ParallelAnimation {
                        id: hudRevealAnim

                        NumberAnimation {
                            target: statusHud
                            property: "opacity"
                            from: 0
                            to: 1
                            duration: 450
                            easing.type: Easing.OutCubic
                        }
                        NumberAnimation {
                            target: hudTranslate
                            property: "y"
                            from: -80
                            to: 0
                            duration: 550
                            easing.type: Easing.OutCubic
                        }
                    }
                }

                // --- NAVIGATION STACK (fades in and scales) ---
                StackLayout {
                    id: contentStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: sidebar.currentIndex

                    // Start smaller and transparent
                    opacity: 0
                    transform: Scale {
                        id: contentScale
                        origin.x: contentStack.width / 2
                        origin.y: contentStack.height / 2
                        xScale: 0.95
                        yScale: 0.95
                    }

                    ParallelAnimation {
                        id: contentRevealAnim

                        NumberAnimation {
                            target: contentStack
                            property: "opacity"
                            from: 0
                            to: 1
                            duration: 600
                            easing.type: Easing.OutCubic
                        }
                        NumberAnimation {
                            target: contentScale
                            property: "xScale"
                            from: 0.95
                            to: 1.0
                            duration: 700
                            easing.type: Easing.OutCubic
                        }
                        NumberAnimation {
                            target: contentScale
                            property: "yScale"
                            from: 0.95
                            to: 1.0
                            duration: 700
                            easing.type: Easing.OutCubic
                        }
                    }

                    // Index 0: Dashboard
                    DashboardView {
                        id: dashboardView
                        onOpenMonitorOptimization: monitorOptWindow.open()
                        onOpenOptimizePopup: Backend.launchOptimizer()
                    }

                    // Index 1: Sensitivity
                    SensitivityView {}

                    // Index 2: History / Rollback
                    HistoryView {}

                    // Index 3: Script Runner
                    ScriptRunnerView {}

                    // Index 4: Advanced / AI
                    AdvancedAiView {}

                    // Index 5: Settings
                    SettingsView {}
                }
            }
        }
    }

    // --- EMULATOR SCAN DIALOG (Glass Overlay) ---
    EmulatorScanDialog {
        id: emulatorScanDialog
        anchors.fill: parent

        onDeviceSelected: function (deviceId) {
            console.log("Selected device:", deviceId);
            Backend.scanForDevices();
        }

        onClosed: {
            console.log("Scan dialog closed");
        }
    }

    // Keyboard shortcut to open scanner
    Shortcut {
        sequence: "Ctrl+D"
        onActivated: emulatorScanDialog.show()
    }

    // --- MONITOR OPTIMIZATION WINDOW ---
    MonitorOptimizationWindow {
        id: monitorOptWindow
        anchors.fill: parent
    }

    // Keyboard shortcut to open monitor optimization
    Shortcut {
        sequence: "Ctrl+M"
        onActivated: monitorOptWindow.open()
    }

    // ===== THEME TRANSITION OVERLAY =====
    Rectangle {
        id: themeTransitionOverlay
        anchors.fill: parent
        z: 9999
        color: "black"
        opacity: 0
        visible: opacity > 0

        // Block all clicks during transition
        MouseArea {
            anchors.fill: parent
            enabled: parent.opacity > 0
            onClicked: {} // Absorb click
        }

        // Fade in animation (to black)
        NumberAnimation {
            id: fadeInAnim
            target: themeTransitionOverlay
            property: "opacity"
            from: 0
            to: 1
            duration: 300
            easing.type: Easing.InOutQuad
            onFinished: {
                // Apply theme while screen is black
                ThemeManager.applyPendingTheme();
                // Start fade out after short pause
                fadeOutTimer.start();
            }
        }

        // Short pause before fade out
        Timer {
            id: fadeOutTimer
            interval: 150
            onTriggered: fadeOutAnim.start()
        }

        // Fade out animation (reveal new theme)
        NumberAnimation {
            id: fadeOutAnim
            target: themeTransitionOverlay
            property: "opacity"
            from: 1
            to: 0
            duration: 400
            easing.type: Easing.InOutQuad
            onFinished: {
                ThemeManager.completeTransition();
            }
        }
    }

    // ===== Listen for Theme Transition Requests =====
    Connections {
        target: ThemeManager
        function onThemeTransitionRequested(newTheme) {
            console.log("[Main] Theme transition requested:", newTheme);
            fadeInAnim.start();
        }
    }
}
