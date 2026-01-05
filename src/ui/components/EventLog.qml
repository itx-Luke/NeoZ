import QtQuick
import QtQuick.Layouts
import "../style"

// Terminal-style Event Log with colored entries
Rectangle {
    id: root

    property var logEntries: []

    color: CyberpunkColors.deepSpace
    radius: 12
    border.width: 1
    border.color: CyberpunkColors.glassBorder

    // Add entry function
    function addEntry(type, message) {
        var entry = {
            "type": type // "success", "warning", "error", "info"
            ,
            "message": message,
            "time": Qt.formatTime(new Date(), "hh:mm:ss")
        };
        logEntries.push(entry);
        logEntriesChanged();
        listView.positionViewAtEnd();
    }

    function getTypeColor(type) {
        switch (type) {
        case "success":
            return CyberpunkColors.matrixGreen;
        case "warning":
            return CyberpunkColors.warningOrange;
        case "error":
            return CyberpunkColors.dangerRed;
        case "info":
        default:
            return CyberpunkColors.neonBlue;
        }
    }

    function getTypeIcon(type) {
        switch (type) {
        case "success":
            return "✓";
        case "warning":
            return "⚠";
        case "error":
            return "✗";
        case "info":
        default:
            return "ℹ";
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "📋"
                font.pixelSize: 14
            }

            Text {
                text: "EVENT LOG"
                color: CyberpunkColors.dimGray
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 2
            }

            Item {
                Layout.fillWidth: true
            }

            Text {
                text: logEntries.length + " entries"
                color: CyberpunkColors.dimGray
                font.pixelSize: 10
            }
        }

        // Divider
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: CyberpunkColors.glassBorder
        }

        // Log list
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: logEntries

            delegate: Rectangle {
                width: listView.width
                height: 28
                radius: 4
                color: Qt.rgba(1, 1, 1, 0.03)

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    // Timestamp
                    Text {
                        text: modelData.time
                        color: CyberpunkColors.dimGray
                        font.pixelSize: 10
                        font.family: "Consolas"
                    }

                    // Type icon
                    Text {
                        text: root.getTypeIcon(modelData.type)
                        color: root.getTypeColor(modelData.type)
                        font.pixelSize: 12
                    }

                    // Message
                    Text {
                        Layout.fillWidth: true
                        text: modelData.message
                        color: root.getTypeColor(modelData.type)
                        font.pixelSize: 11
                        font.family: "Consolas"
                        elide: Text.ElideRight
                    }
                }
            }

            // Auto-scroll behavior
            Behavior on contentY {
                NumberAnimation {
                    duration: 200
                }
            }
        }

        // Empty state
        Text {
            visible: logEntries.length === 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: "Waiting for events..."
            color: CyberpunkColors.dimGray
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    // Sample entries for visual demo
    Component.onCompleted: {
        addEntry("success", "System optimization complete");
        addEntry("info", "Scanning for emulators...");
        addEntry("warning", "High memory usage detected");
        addEntry("success", "BlueStacks priority set to High");
        addEntry("info", "Network optimization applied");
    }
}
