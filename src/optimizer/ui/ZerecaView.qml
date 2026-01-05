import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

/**
 * ZerecaView - Dedicated Zereca Control Plane UI
 *
 * This view is accessed via the "Optimize Now" button and contains:
 * - Dedicated navbar for Zereca sections
 * - Status dashboard
 * - Emulator detection panel
 * - Learning progress visualization
 * - Event log
 */
Item {
    id: zerecaView

    // Current navbar selection
    property int currentSection: 0
    property var sectionNames: ["Dashboard", "Learning", "Safety", "Logs"]

    // Gradient colors
    readonly property color accentPrimary: "#00D4FF"
    readonly property color accentSecondary: "#9D00FF"
    readonly property color accentSuccess: "#00FF41"
    readonly property color accentWarning: "#FF8C00"
    readonly property color accentDanger: "#FF4444"
    readonly property color bgDark: Qt.rgba(0.02, 0.02, 0.06, 0.95)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // ========== ZERECA HEADER ==========
        Rectangle {
            Layout.fillWidth: true
            height: 48
            radius: 12
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(0, 0.8, 1, 0.3)

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 16

                // Zereca badge
                Rectangle {
                    width: 80
                    height: 28
                    radius: 6
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0.0
                            color: accentSecondary
                        }
                        GradientStop {
                            position: 1.0
                            color: accentPrimary
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "ZERECA"
                        color: "#FFFFFF"
                        font.pixelSize: 11
                        font.weight: Font.Bold
                        font.letterSpacing: 2
                    }
                }

                // Status text
                Text {
                    text: Zereca ? Zereca.status : "Initializing..."
                    color: "#AAAAAA"
                    font.pixelSize: 12
                }

                Item {
                    Layout.fillWidth: true
                }

                // Mode indicator
                Rectangle {
                    width: modeText.width + 20
                    height: 24
                    radius: 12
                    color: {
                        if (!Zereca)
                            return Qt.rgba(0.5, 0.5, 0.5, 0.3);
                        switch (Zereca.mode) {
                        case "SCANNING":
                            return Qt.rgba(0, 0.8, 1, 0.2);
                        case "OBSERVING":
                            return Qt.rgba(1, 0.55, 0, 0.2);
                        case "LEARNING":
                            return Qt.rgba(0.6, 0, 1, 0.2);
                        case "TESTING":
                            return Qt.rgba(0.6, 0, 1, 0.3);
                        case "MONITORING":
                            return Qt.rgba(0, 1, 0.26, 0.2);
                        case "ROLLBACK":
                            return Qt.rgba(1, 0.27, 0.27, 0.3);
                        default:
                            return Qt.rgba(0.5, 0.5, 0.5, 0.2);
                        }
                    }
                    border.width: 1
                    border.color: {
                        if (!Zereca)
                            return Qt.rgba(0.5, 0.5, 0.5, 0.5);
                        switch (Zereca.mode) {
                        case "SCANNING":
                            return accentPrimary;
                        case "OBSERVING":
                            return accentWarning;
                        case "LEARNING":
                        case "TESTING":
                            return accentSecondary;
                        case "MONITORING":
                            return accentSuccess;
                        case "ROLLBACK":
                            return accentDanger;
                        default:
                            return Qt.rgba(0.5, 0.5, 0.5, 0.5);
                        }
                    }

                    Text {
                        id: modeText
                        anchors.centerIn: parent
                        text: Zereca ? Zereca.mode : "STANDBY"
                        color: parent.border.color
                        font.pixelSize: 10
                        font.weight: Font.Bold
                        font.letterSpacing: 1
                    }
                }

                // Admin badge
                Rectangle {
                    width: 60
                    height: 24
                    radius: 12
                    color: Zereca && Zereca.adminMode ? Qt.rgba(0, 1, 0.26, 0.15) : Qt.rgba(1, 0.55, 0, 0.15)
                    border.width: 1
                    border.color: Zereca && Zereca.adminMode ? accentSuccess : accentWarning

                    Text {
                        anchors.centerIn: parent
                        text: Zereca && Zereca.adminMode ? "ADMIN" : "USER"
                        color: Zereca && Zereca.adminMode ? accentSuccess : accentWarning
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }
                }
            }
        }

        // ========== NAVBAR ==========
        Rectangle {
            Layout.fillWidth: true
            height: 44
            radius: 10
            color: Qt.rgba(0.05, 0.05, 0.1, 0.8)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.08)

            RowLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                Repeater {
                    model: sectionNames

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 8
                        color: currentSection === index ? Qt.rgba(0.6, 0, 1, 0.3) : navHover.containsMouse ? Qt.rgba(1, 1, 1, 0.05) : "transparent"
                        border.width: currentSection === index ? 1 : 0
                        border.color: accentSecondary

                        Behavior on color {
                            ColorAnimation {
                                duration: 150
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            color: currentSection === index ? "#FFFFFF" : "#888888"
                            font.pixelSize: 12
                            font.weight: currentSection === index ? Font.Bold : Font.Normal

                            Behavior on color {
                                ColorAnimation {
                                    duration: 150
                                }
                            }
                        }

                        MouseArea {
                            id: navHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: currentSection = index
                        }
                    }
                }
            }
        }

        // ========== CONTENT AREA ==========
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: currentSection

            // === SECTION 0: Dashboard ===
            ZerecaDashboard {
                id: dashboardSection
            }

            // === SECTION 1: Learning ===
            ZerecaLearning {
                id: learningSection
            }

            // === SECTION 2: Safety ===
            ZerecaSafety {
                id: safetySection
            }

            // === SECTION 3: Logs ===
            ZerecaLogs {
                id: logsSection
            }
        }

        // ========== BOTTOM CONTROL BAR ==========
        Rectangle {
            Layout.fillWidth: true
            height: 52
            radius: 12
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(0.6, 0, 1, 0.25)

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 12

                // Start/Stop button
                Rectangle {
                    Layout.preferredWidth: 140
                    Layout.fillHeight: true
                    radius: 8
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0.0
                            color: Zereca && Zereca.running ? accentDanger : accentPrimary
                        }
                        GradientStop {
                            position: 1.0
                            color: Zereca && Zereca.running ? "#FF8800" : accentSecondary
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: Zereca && Zereca.running ? "⏹ STOP" : "▶ START"
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.weight: Font.Bold
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (Zereca) {
                                if (Zereca.running) {
                                    Zereca.stop();
                                } else {
                                    Zereca.start();
                                }
                            }
                        }
                    }
                }

                // Metrics display
                Row {
                    spacing: 20

                    MetricPill {
                        label: "FPS"
                        value: Zereca ? Zereca.fps.toFixed(1) : "--"
                        color: accentSuccess
                    }

                    MetricPill {
                        label: "CPU"
                        value: Zereca ? Zereca.cpuUsage.toFixed(0) + "%" : "--"
                        color: accentPrimary
                    }

                    MetricPill {
                        label: "MEM"
                        value: Zereca ? (Zereca.memoryPressure * 100).toFixed(0) + "%" : "--"
                        color: accentSecondary
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // Emulator confidence meter
                Rectangle {
                    width: 150
                    height: 32
                    radius: 6
                    color: Qt.rgba(0, 0, 0, 0.4)
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.1)

                    Column {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            text: Zereca && Zereca.emulatorName ? Zereca.emulatorName : "No Emulator"
                            color: "#AAAAAA"
                            font.pixelSize: 9
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Rectangle {
                            width: 130
                            height: 4
                            radius: 2
                            color: Qt.rgba(1, 1, 1, 0.1)

                            Rectangle {
                                width: parent.width * (Zereca ? Zereca.emulatorConfidence : 0)
                                height: parent.height
                                radius: 2
                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop {
                                        position: 0.0
                                        color: Zereca && Zereca.emulatorConfidence >= 0.75 ? accentSuccess : accentWarning
                                    }
                                    GradientStop {
                                        position: 1.0
                                        color: Zereca && Zereca.emulatorConfidence >= 0.75 ? "#00FFAA" : "#FFAA00"
                                    }
                                }

                                Behavior on width {
                                    NumberAnimation {
                                        duration: 300
                                    }
                                }
                            }
                        }
                    }
                }

                // Stats
                Row {
                    spacing: 12

                    StatBadge {
                        icon: "⚡"
                        value: Zereca ? Zereca.optimizationsApplied : 0
                        tooltip: "Optimizations Applied"
                    }

                    StatBadge {
                        icon: "🧪"
                        value: Zereca ? Zereca.trialsCompleted : 0
                        tooltip: "Trials Completed"
                    }

                    StatBadge {
                        icon: "⚠"
                        value: Zereca ? Zereca.driftCount : 0
                        tooltip: "Drift Events"
                        warning: true
                    }
                }
            }
        }
    }

    // ========== HELPER COMPONENTS ==========

    component MetricPill: Rectangle {
        property string label
        property string value
        property color color

        width: 70
        height: 32
        radius: 6
        color: Qt.rgba(0, 0, 0, 0.3)
        border.width: 1
        border.color: Qt.rgba(color.r, color.g, color.b, 0.3)

        Column {
            anchors.centerIn: parent
            spacing: 1

            Text {
                text: value
                color: parent.parent.color
                font.pixelSize: 12
                font.weight: Font.Bold
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: label
                color: Qt.rgba(1, 1, 1, 0.5)
                font.pixelSize: 8
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    component StatBadge: Rectangle {
        property string icon
        property int value
        property string tooltip
        property bool warning: false

        width: 36
        height: 32
        radius: 6
        color: warning && value > 0 ? Qt.rgba(1, 0.55, 0, 0.15) : Qt.rgba(0, 0, 0, 0.3)
        border.width: 1
        border.color: warning && value > 0 ? accentWarning : Qt.rgba(1, 1, 1, 0.1)

        Column {
            anchors.centerIn: parent
            spacing: 0

            Text {
                text: icon
                font.pixelSize: 10
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: value
                color: warning && value > 0 ? accentWarning : "#AAAAAA"
                font.pixelSize: 10
                font.weight: Font.Bold
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        ToolTip.visible: badgeHover.containsMouse
        ToolTip.text: tooltip
        ToolTip.delay: 500

        MouseArea {
            id: badgeHover
            anchors.fill: parent
            hoverEnabled: true
        }
    }
}
