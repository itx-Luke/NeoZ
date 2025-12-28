import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NeoZ
import "../components"

RowLayout {
    id: root
    spacing: 0

    property int selectedVersion: 0

    // Mock version data
    property var versions: [
        {
            id: "v000007",
            timestamp: "2025-12-09 17:00",
            source: "AI Recommendation",
            cm360: "31.8 cm",
            isCurrent: true,
            isFavorite: true
        },
        {
            id: "v000006",
            timestamp: "2025-12-09 16:45",
            source: "Manual",
            cm360: "34.6 cm",
            isCurrent: false,
            isFavorite: false
        },
        {
            id: "v000005",
            timestamp: "2025-12-09 16:30",
            source: "Rollback",
            cm360: "33.2 cm",
            isCurrent: false,
            isFavorite: true
        },
        {
            id: "v000004",
            timestamp: "2025-12-09 16:15",
            source: "Impact",
            cm360: "35.0 cm",
            isCurrent: false,
            isFavorite: false
        },
        {
            id: "v000003",
            timestamp: "2025-12-09 16:00",
            source: "Default",
            cm360: "36.8 cm",
            isCurrent: false,
            isFavorite: false
        }
    ]

    // === LEFT: VERSION TIMELINE ===
    GlassPanel {
        Layout.preferredWidth: 380
        Layout.fillHeight: true
        Layout.margins: 20
        Layout.rightMargin: 10
        glassOpacity: 0.3

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                text: "Version Timeline"
                color: Style.textPrimary
                font.pixelSize: 18
                font.bold: true
            }

            // Timeline
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: 0

                    Repeater {
                        model: root.versions.length
                        delegate: Item {
                            Layout.fillWidth: true
                            height: 100

                            // Timeline line
                            Rectangle {
                                width: 2
                                height: parent.height
                                x: 15
                                color: Style.primary
                                opacity: 0.5
                            }

                            // Timeline dot
                            Rectangle {
                                width: index === root.selectedVersion ? 16 : 10
                                height: width
                                radius: width / 2
                                x: 15 - width / 2 + 1
                                y: 20
                                color: index === root.selectedVersion ? Style.primary : Style.surfaceHighlight
                                border.color: Style.primary
                                border.width: index === root.selectedVersion ? 2 : 0

                                Behavior on width {
                                    NumberAnimation {
                                        duration: 150
                                    }
                                }
                            }

                            // Version card
                            GlassPanel {
                                anchors.left: parent.left
                                anchors.leftMargin: 40
                                anchors.right: parent.right
                                anchors.rightMargin: 10
                                height: 85
                                y: 5
                                glassOpacity: index === root.selectedVersion ? 0.5 : 0.2
                                glowEnabled: root.versions[index].isCurrent
                                glowColor: Style.primary

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 4

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Text {
                                            text: "Version " + root.versions[index].id
                                            color: root.versions[index].isCurrent ? Style.primary : Style.textPrimary
                                            font.bold: true
                                        }

                                        // Source badge
                                        Rectangle {
                                            visible: root.versions[index].source !== "Default" && root.versions[index].source !== "Manual"
                                            height: 18
                                            width: badgeText.width + 12
                                            radius: 4
                                            color: {
                                                if (root.versions[index].source === "AI Recommendation")
                                                    return "#4D00E5FF";
                                                if (root.versions[index].source === "Rollback")
                                                    return "#4DFF4081";
                                                if (root.versions[index].source === "Impact")
                                                    return "#4DFFA500";
                                                return "transparent";
                                            }
                                            border.color: {
                                                if (root.versions[index].source === "AI Recommendation")
                                                    return Style.primary;
                                                if (root.versions[index].source === "Rollback")
                                                    return Style.secondary;
                                                if (root.versions[index].source === "Impact")
                                                    return "#FFA500";
                                                return "transparent";
                                            }
                                            border.width: 1

                                            Text {
                                                id: badgeText
                                                anchors.centerIn: parent
                                                text: root.versions[index].source
                                                color: parent.border.color
                                                font.pixelSize: 9
                                            }
                                        }

                                        Text {
                                            visible: root.versions[index].source === "Manual"
                                            text: "Manual"
                                            color: Style.textSecondary
                                            font.pixelSize: 11
                                        }

                                        Item {
                                            Layout.fillWidth: true
                                        }

                                        // Favorite star
                                        Text {
                                            text: root.versions[index].isFavorite ? "★" : "☆"
                                            color: root.versions[index].isFavorite ? "#FFD700" : Style.textSecondary
                                            font.pixelSize: 16
                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    root.versions[index].isFavorite = !root.versions[index].isFavorite;
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.versions[index].isCurrent ? "Current: " + root.versions[index].timestamp : "Timestamp: " + root.versions[index].timestamp
                                        color: Style.textSecondary
                                        font.pixelSize: 10
                                    }

                                    Text {
                                        text: "cm/360: " + root.versions[index].cm360
                                        color: Style.primary
                                        font.pixelSize: 11
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.selectedVersion = index
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // === RIGHT: VERSION DETAILS ===
    GlassPanel {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 20
        Layout.leftMargin: 10
        glassOpacity: 0.3

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            // Header row
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Version Details"
                    color: Style.textSecondary
                    font.pixelSize: 12
                }

                Item {
                    Layout.fillWidth: true
                }

                NeonButton {
                    text: "↩ Undo Last Change"
                    accentColor: Style.secondary
                }
            }

            // Version header
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Version " + root.versions[root.selectedVersion].id + (root.versions[root.selectedVersion].isCurrent ? " (Current)" : "")
                    color: Style.primary
                    font.pixelSize: 20
                    font.bold: true
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: root.versions[root.selectedVersion].isFavorite ? "★" : "☆"
                    color: root.versions[root.selectedVersion].isFavorite ? "#FFD700" : Style.textSecondary
                    font.pixelSize: 24
                }
            }

            Text {
                text: "Source: " + root.versions[root.selectedVersion].source + " · Reason: Resolution change adaptation"
                color: Style.textSecondary
                font.pixelSize: 11
            }

            // Parameters and Metrics
            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                // Parameters
                GlassPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    glassOpacity: 0.2

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 10

                        Text {
                            text: "Parameters"
                            color: Style.textPrimary
                            font.bold: true
                        }

                        GridLayout {
                            columns: 2
                            rowSpacing: 8
                            columnSpacing: 20
                            Layout.fillWidth: true

                            Text {
                                text: "DPI"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "800"
                                color: Style.textPrimary
                                font.bold: true
                            }

                            Text {
                                text: "X Multiplier"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "1.12"
                                color: Style.textPrimary
                                font.bold: true
                            }

                            Text {
                                text: "Y Multiplier"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "1.15"
                                color: Style.textPrimary
                                font.bold: true
                            }

                            Text {
                                text: "Curve"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "FF_OneTap_v2"
                                color: Style.textPrimary
                                font.bold: true
                            }

                            Text {
                                text: "Slow-zone %"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "15%"
                                color: Style.textPrimary
                                font.bold: true
                            }

                            Text {
                                text: "Smoothing"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "90 ms"
                                color: Style.textPrimary
                                font.bold: true
                            }
                        }
                    }
                }

                // Derived Metrics
                GlassPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    glassOpacity: 0.2

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 10

                        Text {
                            text: "Derived Metrics"
                            color: Style.textPrimary
                            font.bold: true
                        }

                        GridLayout {
                            columns: 2
                            rowSpacing: 8
                            columnSpacing: 20
                            Layout.fillWidth: true

                            Text {
                                text: "eDPI X"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "896"
                                color: Style.textPrimary
                                font.bold: true
                            }

                            Text {
                                text: "px/cm"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "362.2"
                                color: Style.textPrimary
                                font.bold: true
                            }

                            Text {
                                text: "cm/360 (est)"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "31.8 cm"
                                color: Style.textPrimary
                                font.bold: true
                            }
                        }
                    }
                }
            }

            // Change since previous
            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                glassOpacity: 0.2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Change since previous version"
                            color: Style.textPrimary
                            font.bold: true
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Text {
                            text: "Compact diff"
                            color: Style.textSecondary
                            font.pixelSize: 11
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 30

                        Row {
                            spacing: 8
                            Text {
                                text: "↗"
                                color: Style.success
                                font.bold: true
                            }
                            Text {
                                text: "X Multiplier:"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "1.00 → 1.12"
                                color: Style.textPrimary
                            }
                            Text {
                                text: "(+12%)"
                                color: Style.success
                            }
                        }

                        Row {
                            spacing: 8
                            Text {
                                text: "↘"
                                color: Style.danger
                                font.bold: true
                            }
                            Text {
                                text: "cm/360:"
                                color: Style.textSecondary
                            }
                            Text {
                                text: "34.6 → 31.8"
                                color: Style.textPrimary
                            }
                            Text {
                                text: "(-8%)"
                                color: Style.danger
                            }
                        }
                    }
                }
            }

            // Action buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 15

                NeonButton {
                    text: "Rollback to this version"
                    accentColor: Style.primary
                    Layout.preferredWidth: 200
                }

                NeonButton {
                    text: "Copy as Preset"
                    accentColor: Style.textSecondary
                    Layout.preferredWidth: 150
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }
}
