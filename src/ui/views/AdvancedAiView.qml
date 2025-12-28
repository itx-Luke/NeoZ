import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NeoZ
import "../components"

ScrollView {
    id: root
    clip: true
    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    contentWidth: availableWidth

    // Helper for status display
    readonly property color statusColor: {
        if (Backend.aiStatus.indexOf("Online") !== -1)
            return Style.success;
        if (Backend.aiStatus.indexOf("Analyzing") !== -1)
            return Style.primary;
        return Style.warning;
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // === ROW 1: AI Status + Manual Trigger ===
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            // AI Advisor Status Panel
            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                glassOpacity: 0.3

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Text {
                        text: "AI Advisor Status"
                        color: Style.textSecondary
                        font.pixelSize: 12
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20

                        // Left: Big Title + Status
                        Column {
                            spacing: 8

                            Text {
                                text: "AI Advisor"
                                color: Style.textPrimary
                                font.pixelSize: 28
                                font.bold: true
                            }

                            Rectangle {
                                width: 100
                                height: 26
                                radius: 13
                                color: Qt.rgba(statusColor.r, statusColor.g, statusColor.b, 0.2)
                                border.color: statusColor
                                border.width: 1

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 6

                                    // Loading spinner when processing
                                    BusyIndicator {
                                        width: 14
                                        height: 14
                                        running: Backend.aiProcessing
                                        visible: Backend.aiProcessing
                                    }

                                    Text {
                                        text: Backend.aiStatus
                                        color: statusColor
                                        font.pixelSize: 11
                                        font.bold: true
                                    }
                                }
                            }

                            Text {
                                text: Backend.lastRecommendationSummary || "No recent tuning"
                                color: Style.textSecondary
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                                width: 280
                            }
                            Text {
                                text: Backend.hasRecommendation ? "Confidence: " + Backend.recommendationConfidence.toFixed(2) + " · Severity: " + Backend.recommendationSeverity : "No pending recommendation"
                                color: Style.textSecondary
                                font.pixelSize: 11
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        // Right: Toggle + Slider
                        Column {
                            spacing: 15

                            RowLayout {
                                spacing: 10
                                Text {
                                    text: "Enable AI Advisor"
                                    color: Style.textSecondary
                                }
                                NeonToggle {
                                    checked: Backend.aiEnabled
                                    activeColor: Style.primary
                                    onCheckedChanged: Backend.aiEnabled = checked
                                }
                            }

                            Column {
                                spacing: 5
                                Text {
                                    text: "Aggressiveness"
                                    color: Style.textSecondary
                                    font.pixelSize: 12
                                }
                                Slider {
                                    width: 180
                                    from: 0
                                    to: 2
                                    value: 1
                                    stepSize: 1
                                }
                                RowLayout {
                                    width: 180
                                    Text {
                                        text: "Conservative"
                                        color: Style.textSecondary
                                        font.pixelSize: 10
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        text: "Balanced"
                                        color: Style.primary
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        text: "Aggressive"
                                        color: Style.textSecondary
                                        font.pixelSize: 10
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Manual Tuning Trigger Panel
            GlassPanel {
                Layout.preferredWidth: 320
                Layout.preferredHeight: 220
                glassOpacity: 0.3

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Text {
                        text: "Manual Tuning Trigger"
                        color: Style.textSecondary
                        font.pixelSize: 12
                    }

                    Text {
                        text: "Analyze My Sensitivity"
                        color: Style.textPrimary
                        font.pixelSize: 18
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    NeonButton {
                        text: Backend.aiProcessing ? "Analyzing..." : "Run Analysis Now"
                        accentColor: Style.primary
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        enabled: !Backend.aiProcessing && Backend.aiEnabled
                        onClicked: Backend.runAiAnalysis()
                    }

                    Text {
                        text: "Uses your system config and session data\nto suggest optimal sensitivity."
                        color: Style.textSecondary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }

                    // Pending recommendation
                    Rectangle {
                        Layout.fillWidth: true
                        height: 50
                        radius: 8
                        color: Backend.hasRecommendation ? "#1A00E5FF" : "#0DFFFFFF"
                        border.color: Backend.hasRecommendation ? Style.primary : Style.surfaceHighlight
                        border.width: 1
                        visible: Backend.hasRecommendation || Backend.lastRecommendationSummary

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 8

                            Text {
                                text: Backend.hasRecommendation ? "💡" : "📋"
                                font.pixelSize: 14
                            }
                            Column {
                                Layout.fillWidth: true
                                Text {
                                    text: Backend.hasRecommendation ? "Pending recommendation" : "Last recommendation"
                                    color: Style.textSecondary
                                    font.pixelSize: 10
                                }
                                Text {
                                    text: Backend.hasRecommendation ? "X=" + Backend.recommendedX.toFixed(2) + ", Y=" + Backend.recommendedY.toFixed(2) : Backend.lastRecommendationSummary || "-"
                                    color: Backend.hasRecommendation ? Style.primary : Style.textSecondary
                                    font.pixelSize: 10
                                    font.bold: Backend.hasRecommendation
                                }
                            }

                            // Accept/Decline buttons when recommendation pending
                            Row {
                                spacing: 4
                                visible: Backend.hasRecommendation

                                Rectangle {
                                    width: 28
                                    height: 28
                                    radius: 4
                                    color: "#3300E676"
                                    border.color: Style.success
                                    Text {
                                        anchors.centerIn: parent
                                        text: "✓"
                                        color: Style.success
                                        font.bold: true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: Backend.acceptRecommendation()
                                    }
                                }

                                Rectangle {
                                    width: 28
                                    height: 28
                                    radius: 4
                                    color: "#33FF1744"
                                    border.color: Style.danger
                                    Text {
                                        anchors.centerIn: parent
                                        text: "✕"
                                        color: Style.danger
                                        font.bold: true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: Backend.declineRecommendation()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // === ROW 2: Session Analytics + Plugins ===
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            // Session Analytics Panel
            GlassPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 350
                glassOpacity: 0.3

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    RowLayout {
                        Layout.fillWidth: true
                        Image {
                            source: "qrc:/qt/qml/NeoZ/assets/brain.svg"
                            sourceSize.width: 24
                            sourceSize.height: 24
                        }
                        Text {
                            text: "Session Analytics"
                            color: Style.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                        }
                    }

                    Text {
                        text: "Current Session (" + Backend.resolution + ")"
                        color: Style.textSecondary
                        font.pixelSize: 12
                    }

                    // Chart placeholder
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Qt.rgba(0, 0, 0, 0.2)
                        radius: 8
                        border.color: Style.surfaceHighlight
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "📈 Session Analytics Chart\n(Overshoot Rate & Red-zone Stabilize Time)"
                            color: Style.textSecondary
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    // Stats row - now with real data
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 15

                        // Stat card 1 - FPS
                        GlassPanel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 60
                            glassOpacity: 0.2

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                Text {
                                    text: "🖥️"
                                    font.pixelSize: 20
                                }
                                Column {
                                    Text {
                                        text: "FPS mean"
                                        color: Style.textSecondary
                                        font.pixelSize: 10
                                    }
                                    Text {
                                        text: Backend.fpsMean > 0 ? Backend.fpsMean.toFixed(1) + " · std: " + Backend.fpsStdDev.toFixed(1) : "N/A"
                                        color: Backend.fpsMean >= 50 ? Style.success : (Backend.fpsMean > 0 ? Style.warning : Style.textPrimary)
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        // Stat card 2 - Resolution
                        GlassPanel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 60
                            glassOpacity: 0.2

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                Text {
                                    text: "📱"
                                    font.pixelSize: 20
                                }
                                Column {
                                    Text {
                                        text: "Emulator Resolution"
                                        color: Style.textSecondary
                                        font.pixelSize: 10
                                    }
                                    Text {
                                        text: Backend.mobileRes !== "-" ? Backend.mobileRes : "Not Connected"
                                        color: Style.textPrimary
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        // Stat card 3 - Current Sensitivity
                        GlassPanel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 60
                            glassOpacity: 0.2

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                Text {
                                    text: "🎯"
                                    font.pixelSize: 20
                                }
                                Column {
                                    Text {
                                        text: "Current Sensitivity"
                                        color: Style.textSecondary
                                        font.pixelSize: 10
                                    }
                                    Text {
                                        text: "X: " + Backend.xMultiplier.toFixed(2) + " · Y: " + Backend.yMultiplier.toFixed(2)
                                        color: Style.primary
                                        font.bold: true
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Plugins & Extensions Panel
            GlassPanel {
                Layout.preferredWidth: 350
                Layout.fillHeight: true
                glassOpacity: 0.3

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Text {
                        text: "Plugins & Extensions"
                        color: Style.textPrimary
                        font.pixelSize: 14
                        font.bold: true
                    }

                    // Table header
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Name"
                            color: Style.textSecondary
                            font.pixelSize: 11
                            Layout.preferredWidth: 140
                        }
                        Text {
                            text: "Status"
                            color: Style.textSecondary
                            font.pixelSize: 11
                            Layout.preferredWidth: 70
                        }
                        Text {
                            text: "Tag"
                            color: Style.textSecondary
                            font.pixelSize: 11
                            Layout.fillWidth: true
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Style.surfaceHighlight
                    }

                    // Plugin list
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 5
                        model: ListModel {
                            ListElement {
                                name: "Gemini AI Advisor"
                                status: "Enabled"
                                tag: "AI / Analysis"
                            }
                            ListElement {
                                name: "FPS Monitor"
                                status: "Enabled"
                                tag: "ADB / Metrics"
                            }
                            ListElement {
                                name: "Resolution Tracker"
                                status: "Enabled"
                                tag: "Core / Utility"
                            }
                            ListElement {
                                name: "Heuristic Engine"
                                status: "Enabled"
                                tag: "AI / Fallback"
                            }
                        }
                        delegate: RowLayout {
                            width: parent.width
                            height: 35
                            Text {
                                text: name
                                color: Style.textPrimary
                                font.pixelSize: 12
                                Layout.preferredWidth: 140
                            }
                            Text {
                                text: status
                                color: status === "Enabled" ? Style.success : Style.textSecondary
                                font.pixelSize: 11
                                Layout.preferredWidth: 70
                            }
                            Text {
                                text: tag
                                color: Style.textSecondary
                                font.pixelSize: 11
                                Layout.fillWidth: true
                            }
                            Text {
                                text: "ⓘ"
                                color: Style.textSecondary
                                font.pixelSize: 14
                            }
                        }
                    }

                    // Buttons
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        NeonButton {
                            text: "Open Plugins Folder"
                            accentColor: Style.secondary
                            Layout.fillWidth: true
                        }
                        NeonButton {
                            text: "Reload Plugins"
                            accentColor: Style.textSecondary
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
