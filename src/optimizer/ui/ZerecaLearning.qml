import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

/**
 * ZerecaLearning - Learning engine visualization
 */
Rectangle {
    color: "transparent"

    readonly property color accentSecondary: "#9D00FF"
    readonly property color accentSuccess: "#00FF41"
    readonly property color bgDark: Qt.rgba(0.02, 0.02, 0.06, 0.95)

    RowLayout {
        anchors.fill: parent
        spacing: 16

        // === Hypothesis Queue ===
        Rectangle {
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            radius: 16
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(0.6, 0, 1, 0.25)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                RowLayout {
                    Text {
                        text: "HYPOTHESIS QUEUE"
                        color: accentSecondary
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
                        color: Qt.rgba(0.6, 0, 1, 0.3)
                        Text {
                            anchors.centerIn: parent
                            text: Zereca ? Zereca.hypothesesCount : 0
                            color: "#FFFFFF"
                            font.pixelSize: 10
                            font.weight: Font.Bold
                        }
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 8

                    model: 5 // Placeholder - would be hypothesisList

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 56
                        radius: 8
                        color: Qt.rgba(0, 0, 0, 0.3)
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.08)

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10

                            Rectangle {
                                width: 36
                                height: 36
                                radius: 6
                                color: Qt.rgba(0.6, 0, 1, 0.2)

                                Text {
                                    anchors.centerIn: parent
                                    text: index + 1
                                    color: accentSecondary
                                    font.pixelSize: 14
                                    font.weight: Font.Bold
                                }
                            }

                            Column {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: ["Priority Boost", "CPU Affinity", "IO Priority", "Timer 1ms", "Gold Cores"][index]
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                }
                                Text {
                                    text: "Expected: +" + (3 + index) + "% FPS"
                                    color: "#888888"
                                    font.pixelSize: 10
                                }
                            }

                            Text {
                                text: (70 + index * 5) + "%"
                                color: accentSuccess
                                font.pixelSize: 11
                                font.weight: Font.Bold
                            }
                        }
                    }
                }

                // Reset button
                Rectangle {
                    Layout.fillWidth: true
                    height: 36
                    radius: 8
                    color: Qt.rgba(1, 1, 1, 0.05)
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.1)

                    Text {
                        anchors.centerIn: parent
                        text: "🔄 Reset Learning"
                        color: "#888888"
                        font.pixelSize: 11
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (Zereca)
                            Zereca.resetLearning()
                    }
                }
            }
        }

        // === Trial Progress ===
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 16
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(0, 1, 0.26, 0.25)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "SHADOW TRIAL"
                    color: accentSuccess
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 1.5
                }

                // Trial visualization
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Column {
                        anchors.centerIn: parent
                        spacing: 20

                        // Before/After comparison
                        Row {
                            spacing: 40
                            anchors.horizontalCenter: parent.horizontalCenter

                            Column {
                                spacing: 8
                                Text {
                                    text: "BEFORE"
                                    color: "#666666"
                                    font.pixelSize: 10
                                    font.weight: Font.Bold
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                                Rectangle {
                                    width: 80
                                    height: 80
                                    radius: 40
                                    color: Qt.rgba(1, 1, 1, 0.05)
                                    border.width: 2
                                    border.color: Qt.rgba(1, 1, 1, 0.2)

                                    Text {
                                        anchors.centerIn: parent
                                        text: Zereca ? Zereca.fps.toFixed(0) : "--"
                                        color: "#AAAAAA"
                                        font.pixelSize: 24
                                        font.weight: Font.Bold
                                    }
                                }
                            }

                            Text {
                                text: "→"
                                color: accentSecondary
                                font.pixelSize: 32
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Column {
                                spacing: 8
                                Text {
                                    text: "AFTER"
                                    color: "#666666"
                                    font.pixelSize: 10
                                    font.weight: Font.Bold
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                                Rectangle {
                                    width: 80
                                    height: 80
                                    radius: 40
                                    color: Qt.rgba(0, 1, 0.26, 0.1)
                                    border.width: 2
                                    border.color: accentSuccess

                                    Text {
                                        anchors.centerIn: parent
                                        text: "--"
                                        color: accentSuccess
                                        font.pixelSize: 24
                                        font.weight: Font.Bold
                                    }
                                }
                            }
                        }

                        // Delta indicator
                        Rectangle {
                            width: 120
                            height: 40
                            radius: 8
                            color: Qt.rgba(0, 1, 0.26, 0.15)
                            border.width: 1
                            border.color: accentSuccess
                            anchors.horizontalCenter: parent.horizontalCenter

                            Text {
                                anchors.centerIn: parent
                                text: "+0.0%"
                                color: accentSuccess
                                font.pixelSize: 18
                                font.weight: Font.Bold
                            }
                        }

                        Text {
                            text: "Waiting for trial..."
                            color: "#666666"
                            font.pixelSize: 11
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }
    }
}
