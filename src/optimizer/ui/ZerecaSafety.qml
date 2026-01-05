import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

/**
 * ZerecaSafety - Safety arbiter and probation ledger
 */
Rectangle {
    color: "transparent"

    readonly property color accentWarning: "#FF8C00"
    readonly property color accentDanger: "#FF4444"
    readonly property color accentSuccess: "#00FF41"
    readonly property color bgDark: Qt.rgba(0.02, 0.02, 0.06, 0.95)

    RowLayout {
        anchors.fill: parent
        spacing: 16

        // === Rollback Status ===
        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            radius: 16
            color: Zereca && Zereca.rollbackActive ? Qt.rgba(1, 0.1, 0.1, 0.15) : bgDark
            border.width: 1
            border.color: Zereca && Zereca.rollbackActive ? accentDanger : Qt.rgba(1, 0.55, 0, 0.25)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "SYSTEM PROTECTION"
                    color: Zereca && Zereca.rollbackActive ? accentDanger : accentWarning
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 1.5
                }

                // Shield icon
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 100
                    height: 100
                    radius: 50
                    color: "transparent"
                    border.width: 3
                    border.color: Zereca && Zereca.rollbackActive ? accentDanger : accentSuccess

                    Text {
                        anchors.centerIn: parent
                        text: Zereca && Zereca.rollbackActive ? "⚠" : "🛡"
                        font.pixelSize: 40
                    }

                    SequentialAnimation on scale {
                        loops: Animation.Infinite
                        running: Zereca && Zereca.rollbackActive
                        NumberAnimation {
                            to: 1.1
                            duration: 500
                        }
                        NumberAnimation {
                            to: 1.0
                            duration: 500
                        }
                    }
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: Zereca && Zereca.rollbackActive ? "ROLLBACK ACTIVE" : "Protected"
                    color: Zereca && Zereca.rollbackActive ? accentDanger : accentSuccess
                    font.pixelSize: 14
                    font.weight: Font.Bold
                }

                // Acknowledge button (when in rollback)
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    visible: Zereca && Zereca.rollbackActive
                    width: 160
                    height: 36
                    radius: 8
                    color: Qt.rgba(1, 0.27, 0.27, 0.3)
                    border.width: 1
                    border.color: accentDanger

                    Text {
                        anchors.centerIn: parent
                        text: "Acknowledge"
                        color: "#FFFFFF"
                        font.pixelSize: 12
                        font.weight: Font.Bold
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (Zereca)
                            Zereca.acknowledgeRollback()
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                // Drift counter
                Rectangle {
                    Layout.fillWidth: true
                    height: 48
                    radius: 8
                    color: Qt.rgba(0, 0, 0, 0.3)

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12

                        Text {
                            text: "⚠"
                            font.pixelSize: 16
                        }
                        Text {
                            text: "Drift Events"
                            color: "#888888"
                            font.pixelSize: 11
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Text {
                            text: Zereca ? Zereca.driftCount : 0
                            color: Zereca && Zereca.driftCount > 0 ? accentWarning : "#FFFFFF"
                            font.pixelSize: 16
                            font.weight: Font.Bold
                        }
                    }
                }
            }
        }

        // === Probation Ledger ===
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 16
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(1, 0.27, 0.27, 0.25)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                RowLayout {
                    Text {
                        text: "PROBATION LEDGER"
                        color: accentDanger
                        font.pixelSize: 11
                        font.weight: Font.Bold
                        font.letterSpacing: 1.5
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Rectangle {
                        width: 32
                        height: 20
                        radius: 10
                        color: Qt.rgba(1, 0.27, 0.27, 0.3)
                        Text {
                            anchors.centerIn: parent
                            text: Zereca ? Zereca.probationCount : 0
                            color: "#FFFFFF"
                            font.pixelSize: 10
                            font.weight: Font.Bold
                        }
                    }
                }

                // Placeholder ledger entries
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 6

                    model: Zereca ? Zereca.probationCount : 0

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 48
                        radius: 6
                        color: Qt.rgba(1, 0.27, 0.27, 0.1)
                        border.width: 1
                        border.color: Qt.rgba(1, 0.27, 0.27, 0.2)

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10

                            Text {
                                text: "🚫"
                                font.pixelSize: 14
                            }

                            Column {
                                Layout.fillWidth: true
                                Text {
                                    text: "Configuration #" + (index + 1)
                                    color: "#FFFFFF"
                                    font.pixelSize: 11
                                }
                                Text {
                                    text: "Severity: Medium"
                                    color: accentWarning
                                    font.pixelSize: 9
                                }
                            }

                            Text {
                                text: "BLOCKED"
                                color: accentDanger
                                font.pixelSize: 9
                                font.weight: Font.Bold
                            }
                        }
                    }

                    // Empty state
                    Text {
                        anchors.centerIn: parent
                        visible: parent.count === 0
                        text: "No failed configurations"
                        color: "#666666"
                        font.pixelSize: 12
                    }
                }

                // Clear probation button
                Rectangle {
                    Layout.fillWidth: true
                    height: 36
                    radius: 8
                    color: Qt.rgba(1, 0.27, 0.27, 0.1)
                    border.width: 1
                    border.color: Qt.rgba(1, 0.27, 0.27, 0.3)
                    opacity: Zereca && Zereca.probationCount > 0 ? 1.0 : 0.5

                    Text {
                        anchors.centerIn: parent
                        text: "⚠ Clear All Probation (Manual Reset)"
                        color: accentDanger
                        font.pixelSize: 10
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: Zereca && Zereca.probationCount > 0
                        onClicked: if (Zereca)
                            Zereca.clearProbation()
                    }
                }
            }
        }
    }
}
