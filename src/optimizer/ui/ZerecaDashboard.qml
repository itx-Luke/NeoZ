import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

/**
 * ZerecaDashboard - Main status overview
 */
Rectangle {
    id: dashboardSection
    color: "transparent"

    readonly property color accentPrimary: "#00D4FF"
    readonly property color accentSecondary: "#9D00FF"
    readonly property color accentSuccess: "#00FF41"
    readonly property color bgDark: Qt.rgba(0.02, 0.02, 0.06, 0.95)

    RowLayout {
        anchors.fill: parent
        spacing: 16

        // === Left: Emulator Status ===
        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            radius: 16
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(0, 0.8, 1, 0.25)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "EMULATOR STATUS"
                    color: accentPrimary
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 1.5
                }

                // Emulator icon/orb
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 100
                    height: 100
                    radius: 50
                    color: "transparent"
                    border.width: 3
                    border.color: Zereca && Zereca.emulatorConfidence >= 0.75 ? accentSuccess : Zereca && Zereca.emulatorConfidence > 0 ? "#FF8C00" : Qt.rgba(1, 1, 1, 0.2)

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 8
                        radius: width / 2
                        gradient: Gradient {
                            GradientStop {
                                position: 0.0
                                color: Qt.rgba(0, 0.8, 1, 0.2)
                            }
                            GradientStop {
                                position: 1.0
                                color: Qt.rgba(0.6, 0, 1, 0.1)
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: Zereca && Zereca.emulatorConfidence > 0 ? Math.round(Zereca.emulatorConfidence * 100) + "%" : "—"
                        color: parent.border.color
                        font.pixelSize: 24
                        font.weight: Font.Bold
                    }

                    SequentialAnimation on scale {
                        loops: Animation.Infinite
                        running: Zereca && Zereca.emulatorConfidence > 0
                        NumberAnimation {
                            to: 1.03
                            duration: 1500
                            easing.type: Easing.InOutSine
                        }
                        NumberAnimation {
                            to: 1.0
                            duration: 1500
                            easing.type: Easing.InOutSine
                        }
                    }
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: Zereca && Zereca.emulatorName ? Zereca.emulatorName : "No Emulator Detected"
                    color: "#FFFFFF"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: Zereca && Zereca.emulatorConfidence >= 0.75 ? "✓ Ready for optimization" : "Scanning..."
                    color: Zereca && Zereca.emulatorConfidence >= 0.75 ? accentSuccess : "#888888"
                    font.pixelSize: 11
                }

                Item {
                    Layout.fillHeight: true
                }

                // Supported emulators list
                Text {
                    text: "SUPPORTED"
                    color: "#666666"
                    font.pixelSize: 9
                    font.weight: Font.Bold
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 6

                    Repeater {
                        model: ["Bluestacks", "LDPlayer", "Nox", "MEmu", "SmartGaGa"]

                        Rectangle {
                            width: emulatorLabel.width + 12
                            height: 20
                            radius: 4
                            color: Qt.rgba(1, 1, 1, 0.05)
                            border.width: 1
                            border.color: Qt.rgba(1, 1, 1, 0.1)

                            Text {
                                id: emulatorLabel
                                anchors.centerIn: parent
                                text: modelData
                                color: "#888888"
                                font.pixelSize: 9
                            }
                        }
                    }
                }
            }
        }

        // === Center: Observation Progress ===
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 16
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(0.6, 0, 1, 0.25)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "OBSERVATION PHASE"
                    color: accentSecondary
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 1.5
                }

                // Progress ring
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 160

                    Rectangle {
                        anchors.centerIn: parent
                        width: 140
                        height: 140
                        radius: 70
                        color: "transparent"
                        border.width: 8
                        border.color: Qt.rgba(1, 1, 1, 0.1)

                        // Progress arc (simplified as filled segment)
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 4
                            radius: width / 2
                            color: "transparent"
                            border.width: 6
                            border.color: accentSecondary
                            opacity: Zereca ? Zereca.observationProgress : 0

                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 300
                                }
                            }
                        }

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: Zereca ? Math.round(Zereca.observationProgress * 100) + "%" : "0%"
                                color: "#FFFFFF"
                                font.pixelSize: 28
                                font.weight: Font.Bold
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: Zereca && Zereca.mode === "OBSERVING" ? "Observing..." : Zereca && Zereca.observationProgress >= 1 ? "Complete" : "Waiting"
                                color: "#888888"
                                font.pixelSize: 10
                            }
                        }
                    }
                }

                // Baseline metrics preview
                Rectangle {
                    Layout.fillWidth: true
                    height: 80
                    radius: 10
                    color: Qt.rgba(0, 0, 0, 0.3)
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.08)

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 16

                        BaselineMetric {
                            label: "Baseline FPS"
                            value: Zereca ? Zereca.fps.toFixed(1) : "--"
                            unit: ""
                        }

                        Rectangle {
                            width: 1
                            Layout.fillHeight: true
                            color: Qt.rgba(1, 1, 1, 0.1)
                        }

                        BaselineMetric {
                            label: "Variance"
                            value: Zereca ? Zereca.fpsVariance.toFixed(2) : "--"
                            unit: "σ"
                        }

                        Rectangle {
                            width: 1
                            Layout.fillHeight: true
                            color: Qt.rgba(1, 1, 1, 0.1)
                        }

                        BaselineMetric {
                            label: "CPU Load"
                            value: Zereca ? Zereca.cpuUsage.toFixed(0) : "--"
                            unit: "%"
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }

        // === Right: Quick Stats ===
        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            radius: 16
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(0, 1, 0.26, 0.25)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Text {
                    text: "OPTIMIZATION STATS"
                    color: accentSuccess
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 1.5
                }

                // Stat cards
                StatCard {
                    Layout.fillWidth: true
                    title: "Trials Completed"
                    value: Zereca ? Zereca.trialsCompleted : 0
                    icon: "🧪"
                }

                StatCard {
                    Layout.fillWidth: true
                    title: "Optimizations Applied"
                    value: Zereca ? Zereca.optimizationsApplied : 0
                    icon: "⚡"
                    highlight: true
                }

                StatCard {
                    Layout.fillWidth: true
                    title: "Hypotheses Pending"
                    value: Zereca ? Zereca.hypothesesCount : 0
                    icon: "💡"
                }

                StatCard {
                    Layout.fillWidth: true
                    title: "Drift Events"
                    value: Zereca ? Zereca.driftCount : 0
                    icon: "⚠"
                    warning: (Zereca && Zereca.driftCount > 0)
                }

                StatCard {
                    Layout.fillWidth: true
                    title: "Probation Entries"
                    value: Zereca ? Zereca.probationCount : 0
                    icon: "🚫"
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    // === Helper Components ===
    component BaselineMetric: Column {
        property string label
        property string value
        property string unit

        Layout.fillWidth: true
        spacing: 4

        Text {
            text: label
            color: "#666666"
            font.pixelSize: 9
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            Text {
                text: value
                color: "#FFFFFF"
                font.pixelSize: 20
                font.weight: Font.Bold
            }
            Text {
                text: unit
                color: "#888888"
                font.pixelSize: 12
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2
            }
        }
    }

    component StatCard: Rectangle {
        property string title
        property int value
        property string icon
        property bool highlight: false
        property bool warning: false

        height: 52
        radius: 8
        color: highlight ? Qt.rgba(0, 1, 0.26, 0.1) : warning ? Qt.rgba(1, 0.55, 0, 0.1) : Qt.rgba(0, 0, 0, 0.3)
        border.width: 1
        border.color: highlight ? Qt.rgba(0, 1, 0.26, 0.3) : warning ? Qt.rgba(1, 0.55, 0, 0.3) : Qt.rgba(1, 1, 1, 0.08)

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            Text {
                text: icon
                font.pixelSize: 18
            }

            Column {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: title
                    color: "#888888"
                    font.pixelSize: 9
                }

                Text {
                    text: value
                    color: highlight ? accentSuccess : warning ? "#FF8C00" : "#FFFFFF"
                    font.pixelSize: 16
                    font.weight: Font.Bold
                }
            }
        }
    }
}
