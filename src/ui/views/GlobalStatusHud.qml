import QtQuick
import QtQuick.Layouts
import NeoZ
import "../components"
import "../style"

// Global Status HUD - Fully theme-based for plugin/skin support
Rectangle {
    id: root
    Layout.fillWidth: true
    Layout.preferredHeight: 75
    color: "transparent"

    // Signal emitted only when ADB tile is clicked
    signal adbClicked

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 3
        anchors.bottomMargin: 3
        spacing: 8

        // === EMULATOR TILE ===
        StatusTile {
            label: "Emulator:"
            value: Backend.emulatorStatus === "Running" ? "Bluestacks · " + Backend.resolution : "Stopped"
            subtext: Backend.freeFireRunning ? "Free Fire: Running" : ""
            isActive: Backend.emulatorStatus === "Running"
            activeColor: Style.hudStatusEmulator
            iconText: "🖥️"

            onClicked: {
                Backend.identifyEmulators();
                root.adbClicked();
            }
        }

        // === ADB TILE ===
        StatusTile {
            label: "ADB:"
            value: Backend.adbStatus
            subtext: Backend.adbStatus === "Connected" ? "emulator-5554" : "Click to scan"
            isActive: Backend.adbStatus === "Connected"
            activeColor: Style.hudStatusAdb
            iconText: "🔌"

            onClicked: {
                root.adbClicked();
            }
        }

        // === INPUT TILE ===
        StatusTile {
            label: "Input:"
            value: "Hook: " + (Backend.inputHookActive ? "Active" : "Inactive")
            subtext: Backend.inputHookActive ? "Click to stop" : "Click to start"
            isActive: Backend.inputHookActive
            activeColor: Style.hudStatusInput
            iconText: "🎮"

            onClicked: {
                Backend.toggleInputHook();
            }
        }

        // === DISPLAY TILE ===
        StatusTile {
            label: "Display:"
            value: "Display1 · " + Backend.displayRefreshRate
            subtext: ""
            isActive: true
            activeColor: Style.hudStatusDisplay
            iconText: "🖵"

            onClicked: {
                console.log("Display tile clicked - functionality TBD");
            }
        }

        // === SCRIPT RUNNER TILE ===
        StatusTile {
            label: "Script Runner:"
            value: Backend.scriptStatus
            subtext: "Last: Success"
            isActive: Backend.scriptStatus === "Running"
            activeColor: Style.hudStatusScript
            iconText: "📜"

            onClicked: {
                console.log("Script Runner tile clicked - functionality TBD");
            }
        }

        // === AI ADVISOR TILE ===
        StatusTile {
            label: "AI Advisor:"
            value: Backend.aiStatus
            subtext: ""
            isActive: Backend.aiStatus === "Online"
            activeColor: Style.hudStatusAi
            iconText: "🧠"

            onClicked: {
                console.log("AI Advisor tile clicked - functionality TBD");
            }
        }
    }
}
