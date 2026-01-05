import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import NeoZ
import "../components"

ScrollView {
    id: root
    clip: true
    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    contentWidth: availableWidth

    // Signal to open Monitor Optimization Window
    signal openMonitorOptimization

    // Signal to open Optimize Popup
    signal openOptimizePopup

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 18

        // ============================================================
        // HEADER: Current Sensitivity Overview + Preset Tabs
        // ============================================================
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Current Sensitivity Overview"
                color: Style.textPrimary
                font.pixelSize: 22
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            // Preset tabs
            RowLayout {
                spacing: 0

                Rectangle {
                    width: 70
                    height: 28
                    radius: 4
                    color: Qt.rgba(1, 1, 1, 0.08)
                    Text {
                        anchors.centerIn: parent
                        text: "Training"
                        color: Style.textSecondary
                        font.pixelSize: 11
                    }
                }
                Rectangle {
                    width: 70
                    height: 28
                    radius: 4
                    color: Style.primary
                    Text {
                        anchors.centerIn: parent
                        text: "Ranked"
                        color: Style.background
                        font.pixelSize: 11
                        font.bold: true
                    }
                }
                Rectangle {
                    width: 85
                    height: 28
                    radius: 4
                    color: Qt.rgba(1, 1, 1, 0.08)
                    Text {
                        anchors.centerIn: parent
                        text: "Sniper Preset"
                        color: Style.textSecondary
                        font.pixelSize: 11
                    }
                }
            }
        }

        // ============================================================
        // MAIN ROW: Sensitivity Cards + Radar Chart
        // ============================================================
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            // LEFT COLUMN: Multiplier Cards + Derived Metrics
            ColumnLayout {
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width * 0.65 // Removed to fix recursive layout loop
                spacing: 15

                // === BIG MULTIPLIER CARDS ===
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15

                    // X Multiplier
                    GlassPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 110
                        glowEnabled: true
                        glowColor: Style.primary
                        glassOpacity: 0.5

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                text: "X Multiplier:"
                                color: Style.textSecondary
                                font.pixelSize: 11
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: Backend.xMultiplier.toFixed(2)
                                color: Style.primary
                                font.pixelSize: 42
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }

                    // Y Multiplier
                    GlassPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 110
                        glowEnabled: true
                        glowColor: Style.primary
                        glassOpacity: 0.5

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                text: "Y Multiplier:"
                                color: Style.textSecondary
                                font.pixelSize: 11
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: Backend.yMultiplier.toFixed(2)
                                color: Style.primary
                                font.pixelSize: 42
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }

                    // DPI
                    GlassPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 110
                        glowEnabled: true
                        glowColor: Style.primary
                        glassOpacity: 0.5

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                text: "DPI:"
                                color: Style.textSecondary
                                font.pixelSize: 11
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "800"
                                color: Style.primary
                                font.pixelSize: 42
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }

                // === DERIVED METRICS ROW ===
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15

                    GlassPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 55
                        glassOpacity: 0.3

                        Column {
                            anchors.centerIn: parent
                            spacing: 2

                            Text {
                                text: "px/cm:"
                                color: Style.textSecondary
                                font.pixelSize: 11
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "362.2"
                                color: Style.textPrimary
                                font.pixelSize: 18
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }

                    GlassPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 55
                        glassOpacity: 0.3

                        Column {
                            anchors.centerIn: parent
                            spacing: 2

                            Text {
                                text: "cm/360 (est):"
                                color: Style.textSecondary
                                font.pixelSize: 11
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "31.8 cm"
                                color: Style.textPrimary
                                font.pixelSize: 18
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }
            }

            // RIGHT COLUMN: Radar Chart
            GlassPanel {
                Layout.preferredWidth: 280
                Layout.preferredHeight: 230
                glassOpacity: 0.25

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10

                    Text {
                        text: "Stability vs Flick Control vs Micro-Adjust"
                        color: Style.textPrimary
                        font.pixelSize: 11
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    RadarChart {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignCenter
                    }
                }
            }
        }

        // ============================================================
        // BOTTOM ROW: Environment Summary + Recent Activity
        // ============================================================
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 400
            spacing: 20

            // === ENVIRONMENT SUMMARY ===
            GlassPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                glassOpacity: 0.3

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 12

                    Text {
                        text: "Environment Summary"
                        color: Style.textPrimary
                        font.pixelSize: 14
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 15

                        // Host Display Panel
                        GlassPanel {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            glassOpacity: 0.2

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                RowLayout {
                                    spacing: 8
                                    Rectangle {
                                        width: 24
                                        height: 24
                                        radius: 4
                                        color: Qt.rgba(0.2, 0.6, 1, 0.2)
                                        border.color: "#5599ff"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "🖥️"
                                            font.pixelSize: 12
                                        }
                                    }
                                    Text {
                                        text: "Host Display"
                                        color: Style.textPrimary
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                }

                                Text {
                                    text: Backend.resolution
                                    color: Style.textSecondary
                                    font.pixelSize: 12
                                }

                                Item {
                                    Layout.fillHeight: true
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 28
                                    radius: 6
                                    color: Qt.rgba(0.2, 0.6, 1, 0.15)
                                    border.color: "#5599ff"
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: "Open Resolution Manager"
                                        color: "#5599ff"
                                        font.pixelSize: 11
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: root.openMonitorOptimization()
                                    }
                                }
                            }
                        }

                        // Emulator State Panel
                        GlassPanel {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            glassOpacity: 0.2

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                RowLayout {
                                    spacing: 8
                                    Rectangle {
                                        width: 24
                                        height: 24
                                        radius: 4
                                        color: Qt.rgba(0.2, 0.6, 1, 0.2)
                                        border.color: "#5599ff"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "📱"
                                            font.pixelSize: 12
                                        }
                                    }
                                    Text {
                                        text: Backend.adbStatus === "Connected" ? Backend.selectedDevice : "Emulator State"
                                        color: Style.textPrimary
                                        font.pixelSize: 12
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                }

                                Text {
                                    text: "wm size: " + Backend.mobileRes + " · density: " + Backend.mobileDpi
                                    color: Style.textSecondary
                                    font.pixelSize: 11
                                }

                                Text {
                                    text: "Free Fire: " + (Backend.freeFireRunning ? "Foreground" : "Not Running")
                                    color: Backend.freeFireRunning ? Style.success : Style.textSecondary
                                    font.pixelSize: 11
                                    font.bold: Backend.freeFireRunning
                                }

                                Item {
                                    Layout.fillHeight: true
                                }
                            }
                        }
                    }
                }
            }

            // === RECENT ACTIVITY & RECOMMENDATIONS ===
            GlassPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                glassOpacity: 0.3

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 15

                    // Activity Log
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8

                        // NEW Simple Optimize Button
                        Rectangle {
                            id: optimizeBtn
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            Layout.bottomMargin: 16
                            radius: 10
                            color: "#9D00FF"

                            Text {
                                anchors.centerIn: parent
                                text: "⚡ OPTIMIZE NOW"
                                color: "#FFFFFF"
                                font.pixelSize: 14
                                font.bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    console.log(">>> OPTIMIZE BUTTON CLICKED <<<");
                                    root.openOptimizePopup();
                                }
                            }
                        }

                        Text {
                            text: "Recent Activity & Recommendations"
                            color: Style.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                        }

                        // Activity items
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            // Activity 1
                            RowLayout {
                                spacing: 8
                                Text {
                                    text: "🔄"
                                    font.pixelSize: 12
                                }
                                Text {
                                    text: "16:10 — Resolution changed\nfrom 1920×1080 to 1600×900"
                                    color: Style.textSecondary
                                    font.pixelSize: 10
                                    lineHeight: 1.2
                                }
                            }

                            // Activity 2
                            RowLayout {
                                spacing: 8
                                Text {
                                    text: "⚠️"
                                    font.pixelSize: 12
                                }
                                Text {
                                    text: "16:12 — AI suggested sensitivity\ntweak (Declined)"
                                    color: Style.textSecondary
                                    font.pixelSize: 10
                                    lineHeight: 1.2
                                }
                            }

                            // Activity 3
                            RowLayout {
                                spacing: 8
                                Text {
                                    text: "📜"
                                    font.pixelSize: 12
                                }
                                Text {
                                    text: "16:25 — Script 'ff_optimize.sh'\ncompleted"
                                    color: Style.textSecondary
                                    font.pixelSize: 10
                                    lineHeight: 1.2
                                }
                            }
                        }

                        // Spacer to push content to top
                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }
            }
        }

        Item {
            Layout.preferredHeight: 20
        }
    }
}
