import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

/**
 * ZerecaLogs - Event log viewer
 */
Rectangle {
    color: Qt.rgba(0.02, 0.02, 0.06, 0.95)
    radius: 16
    border.width: 1
    border.color: Qt.rgba(0, 0.8, 1, 0.25)

    readonly property color accentPrimary: "#00D4FF"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Text {
                text: "EVENT LOG"
                color: accentPrimary
                font.pixelSize: 11
                font.weight: Font.Bold
                font.letterSpacing: 1.5
            }

            Item {
                Layout.fillWidth: true
            }

            // Filter buttons
            Row {
                spacing: 6

                Repeater {
                    model: ["ALL", "INFO", "WARNING", "ERROR"]

                    Rectangle {
                        width: filterLabel.width + 16
                        height: 22
                        radius: 4
                        color: index === 0 ? Qt.rgba(0, 0.8, 1, 0.2) : Qt.rgba(1, 1, 1, 0.05)
                        border.width: 1
                        border.color: index === 0 ? accentPrimary : Qt.rgba(1, 1, 1, 0.1)

                        Text {
                            id: filterLabel
                            anchors.centerIn: parent
                            text: modelData
                            color: index === 0 ? accentPrimary : "#888888"
                            font.pixelSize: 9
                            font.weight: Font.Bold
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }
            }
        }

        // Log list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4

            model: Zereca ? Zereca.eventLog : []

            delegate: Rectangle {
                width: ListView.view.width
                height: 32
                radius: 4
                color: {
                    var level = modelData.level || "INFO";
                    switch (level) {
                    case "CRITICAL":
                        return Qt.rgba(1, 0.27, 0.27, 0.15);
                    case "WARNING":
                        return Qt.rgba(1, 0.55, 0, 0.1);
                    case "SUCCESS":
                        return Qt.rgba(0, 1, 0.26, 0.1);
                    default:
                        return Qt.rgba(0, 0, 0, 0.3);
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 10

                    // Timestamp
                    Text {
                        text: modelData.timestamp || "--:--:--"
                        color: "#666666"
                        font.pixelSize: 10
                        font.family: "Consolas"
                    }

                    // Level badge
                    Rectangle {
                        width: 60
                        height: 18
                        radius: 3
                        color: {
                            var level = modelData.level || "INFO";
                            switch (level) {
                            case "CRITICAL":
                                return Qt.rgba(1, 0.27, 0.27, 0.4);
                            case "WARNING":
                                return Qt.rgba(1, 0.55, 0, 0.3);
                            case "SUCCESS":
                                return Qt.rgba(0, 1, 0.26, 0.3);
                            default:
                                return Qt.rgba(0, 0.8, 1, 0.2);
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: modelData.level || "INFO"
                            color: {
                                var level = modelData.level || "INFO";
                                switch (level) {
                                case "CRITICAL":
                                    return "#FF4444";
                                case "WARNING":
                                    return "#FF8C00";
                                case "SUCCESS":
                                    return "#00FF41";
                                default:
                                    return accentPrimary;
                                }
                            }
                            font.pixelSize: 8
                            font.weight: Font.Bold
                        }
                    }

                    // Message
                    Text {
                        Layout.fillWidth: true
                        text: modelData.message || ""
                        color: "#CCCCCC"
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                }
            }

            // Empty state
            Text {
                anchors.centerIn: parent
                visible: parent.count === 0
                text: "No events yet. Start Zereca to see activity."
                color: "#666666"
                font.pixelSize: 12
            }
        }
    }
}
