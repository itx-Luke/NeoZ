import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
import NeoZ
import "../components"

Item {
    id: root

    property bool isDragging: false
    property bool recentlyRan: false
    property color hue1: "#4d21fc"
    property color hue2: "#00a2ff"
    property string pendingScriptPath: ""
    property string pendingScriptName: ""

    Timer {
        id: glowFadeTimer
        interval: 600000
        onTriggered: recentlyRan = false
    }

    Connections {
        target: Backend
        function onScriptFinished(jobId, exitCode) {
            recentlyRan = true;
            glowFadeTimer.restart();
        }
    }

    // File Dialog for browsing .sh files
    FileDialog {
        id: fileDialog
        title: "Select Script File"
        nameFilters: ["Shell Scripts (*.sh)", "All Files (*)"]
        onAccepted: {
            var filePath = selectedFile.toString();
            if (filePath.startsWith("file:///"))
                filePath = filePath.substring(8);
            showConfirmation(filePath);
        }
    }

    function showConfirmation(scriptPath) {
        pendingScriptPath = scriptPath;
        var parts = scriptPath.replace(/\\/g, "/").split("/");
        var fileName = parts[parts.length - 1];
        pendingScriptName = fileName.replace(/\.sh$/i, "");
        confirmToast.opacity = 0;
        confirmToast.visible = true;
        confirmToast.opacity = 1;
    }

    function executeScript() {
        if (pendingScriptPath) {
            Backend.runScript(pendingScriptPath);
            dropStatus.text = "Running: " + pendingScriptName;
            dropStatus.color = "#00E676";
        }
        confirmToast.opacity = 0;
    }

    function declineScript() {
        pendingScriptPath = "";
        pendingScriptName = "";
        confirmToast.opacity = 0;
    }

    // Backdrop
    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        visible: confirmToast.visible
        z: 999
        MouseArea {
            anchors.fill: parent
            onClicked: declineScript()
        }
    }

    // Glass Confirmation Toast - Dark Transparent
    Rectangle {
        id: confirmToast
        anchors.centerIn: parent
        width: 400
        height: 240
        radius: 20
        color: "#E6080810"
        border.color: Qt.alpha(hue2, 0.5)
        border.width: 1
        visible: false
        opacity: 0
        z: 1000

        Behavior on opacity {
            NumberAnimation {
                duration: 400
                easing.type: Easing.OutCubic
            }
        }

        onOpacityChanged: if (opacity === 0)
            visible = false

        // Subtle gradient overlay
        Rectangle {
            anchors.fill: parent
            radius: 20
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: Qt.alpha(hue1, 0.1)
                }
                GradientStop {
                    position: 0.5
                    color: "transparent"
                }
                GradientStop {
                    position: 1.0
                    color: Qt.alpha(hue2, 0.1)
                }
            }
        }

        // Animated border
        Rectangle {
            anchors.fill: parent
            radius: 20
            color: "transparent"
            border.width: 1
            border.color: hue2

            SequentialAnimation on border.color {
                loops: Animation.Infinite
                ColorAnimation {
                    to: hue1
                    duration: 2000
                    easing.type: Easing.InOutSine
                }
                ColorAnimation {
                    to: hue2
                    duration: 2000
                    easing.type: Easing.InOutSine
                }
            }
        }

        // Top shine
        Rectangle {
            width: parent.width * 0.5
            height: 1
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
                GradientStop {
                    position: 0.5
                    color: Qt.alpha("#FFFFFF", 0.6)
                }
                GradientStop {
                    position: 1.0
                    color: "transparent"
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 28
            spacing: 12

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "RUN SCRIPT"
                font.pixelSize: 12
                font.letterSpacing: 2
                color: "#888"
            }

            Item {
                Layout.preferredHeight: 8
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "You're about to run"
                font.pixelSize: 13
                color: "#AAA"
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                text: pendingScriptName
                font.pixelSize: 22
                font.bold: true
                color: hue2
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideMiddle
            }

            Item {
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20

                Rectangle {
                    width: 110
                    height: 40
                    radius: 10
                    color: "#2A2A2A"
                    border.color: "#444"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Decline"
                        font.pixelSize: 13
                        color: "#AAA"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: declineScript()
                        onPressed: parent.color = "#222"
                        onReleased: parent.color = "#2A2A2A"
                    }
                }

                Rectangle {
                    width: 110
                    height: 40
                    radius: 10
                    color: hue2
                    border.color: Qt.lighter(hue2, 1.2)
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Accept"
                        font.pixelSize: 13
                        font.bold: true
                        color: "#000"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: executeScript()
                        onPressed: parent.color = Qt.darker(hue2, 1.2)
                        onReleased: parent.color = hue2
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // TOP STATUS BAR
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            radius: 12
            color: "#CC08090d"
            border.color: Qt.alpha(hue2, 0.3)
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 24

                Row {
                    spacing: 8
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: Backend.adbDevices.length > 0 ? "#00E676" : "#666"
                    }
                    Text {
                        text: Backend.adbDevices.length + " Device(s)"
                        font.pixelSize: 12
                        color: "#FFF"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Rectangle {
                    width: 1
                    height: 24
                    color: Qt.alpha(hue2, 0.2)
                }

                Row {
                    spacing: 8
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: Backend.adbStatus === "Connected" ? "#00E676" : "#FF1744"
                    }
                    Text {
                        text: Backend.adbStatus === "Connected" ? "ADB Online" : "ADB Offline"
                        font.pixelSize: 12
                        color: "#FFF"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Rectangle {
                    width: 90
                    height: 30
                    radius: 8
                    color: recentlyRan ? Qt.alpha(hue2, 0.2) : "#1A1A1A"
                    border.color: recentlyRan ? hue2 : "#333"
                    Text {
                        anchors.centerIn: parent
                        text: recentlyRan ? "Completed" : "Ready"
                        font.pixelSize: 11
                        color: recentlyRan ? hue2 : "#888"
                    }
                }
            }
        }

        // MAIN CONTENT
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // LEFT: DROP ZONE
            Item {
                Layout.preferredWidth: 380
                Layout.fillHeight: true

                Rectangle {
                    anchors.fill: parent
                    radius: 16
                    color: "#CC08090d"
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 16
                    color: "transparent"
                    border.width: 2
                    border.color: hue1

                    SequentialAnimation on border.color {
                        loops: Animation.Infinite
                        ColorAnimation {
                            to: hue2
                            duration: 3000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: hue1
                            duration: 3000
                            easing.type: Easing.InOutSine
                        }
                    }

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation {
                            to: 0.3
                            duration: 2000
                            easing.type: Easing.InOutSine
                        }
                        NumberAnimation {
                            to: 1.0
                            duration: 2000
                            easing.type: Easing.InOutSine
                        }
                    }
                }

                DropArea {
                    anchors.fill: parent
                    onEntered: function (drag) {
                        isDragging = true;
                        drag.accepted = true;
                    }
                    onExited: isDragging = false
                    onDropped: function (drop) {
                        isDragging = false;
                        if (drop.urls.length > 0) {
                            var filePath = drop.urls[0].toString();
                            if (filePath.startsWith("file:///"))
                                filePath = filePath.substring(8);
                            if (filePath.toLowerCase().endsWith(".sh")) {
                                showConfirmation(filePath);
                            } else {
                                dropStatus.text = "Only .sh files";
                                dropStatus.color = "#FF1744";
                            }
                        }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 16

                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        width: 70
                        height: 70
                        radius: 35
                        color: "transparent"
                        border.color: isDragging ? hue2 : "#555"
                        border.width: 2
                        Text {
                            anchors.centerIn: parent
                            text: "+"
                            font.pixelSize: 30
                            color: isDragging ? hue2 : "#AAA"
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "DROP SCRIPT"
                        font.pixelSize: 14
                        font.letterSpacing: 2
                        color: isDragging ? hue2 : "#FFF"
                    }

                    Text {
                        id: dropStatus
                        Layout.alignment: Qt.AlignHCenter
                        text: "Drag .sh files here"
                        font.pixelSize: 11
                        color: "#888"
                    }

                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        width: 90
                        height: 26
                        radius: 6
                        color: Qt.alpha(hue1, 0.2)
                        border.color: Qt.alpha(hue1, 0.5)
                        Text {
                            anchors.centerIn: parent
                            text: "Browse"
                            font.pixelSize: 11
                            color: "#FFF"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: fileDialog.open()
                        }
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 16
                    height: 3
                    radius: 2
                    color: "#1A1A1A"
                    visible: Backend.scriptRunning

                    Rectangle {
                        height: parent.height
                        radius: 2
                        color: hue2
                        width: parent.width * 0.25
                        SequentialAnimation on x {
                            running: Backend.scriptRunning
                            loops: Animation.Infinite
                            NumberAnimation {
                                from: 0
                                to: 260
                                duration: 1000
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                from: 260
                                to: 0
                                duration: 1000
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
            }

            // RIGHT: HISTORY + TERMINAL
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 140
                    radius: 12
                    color: "#CC08090d"
                    border.color: Qt.alpha(hue1, 0.2)

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        RowLayout {
                            Text {
                                text: "HISTORY"
                                font.pixelSize: 10
                                font.letterSpacing: 1
                                color: "#888"
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            Text {
                                text: "Clear"
                                font.pixelSize: 10
                                color: "#666"
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: Backend.clearJobs()
                                }
                            }
                        }

                        ListView {
                            id: historyList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: Backend.scriptJobs
                            spacing: 2

                            delegate: Rectangle {
                                width: historyList.width
                                height: 26
                                radius: 4
                                color: ma.containsMouse ? "#1A1A1A" : "transparent"
                                MouseArea {
                                    id: ma
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: Backend.viewJobLogs(modelData.id)
                                }
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    Text {
                                        Layout.preferredWidth: 120
                                        text: modelData.script || "-"
                                        font.pixelSize: 10
                                        color: "#FFF"
                                        elide: Text.ElideMiddle
                                    }
                                    Rectangle {
                                        width: 50
                                        height: 16
                                        radius: 8
                                        color: modelData.status === "Success" ? Qt.alpha("#00E676", 0.15) : modelData.status === "Failed" ? Qt.alpha("#FF1744", 0.15) : "#1A1A1A"
                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData.status || "-"
                                            font.pixelSize: 8
                                            color: modelData.status === "Success" ? "#00E676" : modelData.status === "Failed" ? "#FF1744" : "#888"
                                        }
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        text: modelData.started || "-"
                                        font.pixelSize: 9
                                        color: "#666"
                                    }
                                }
                            }
                            Text {
                                anchors.centerIn: parent
                                visible: historyList.count === 0
                                text: "No jobs"
                                font.pixelSize: 11
                                color: "#666"
                            }
                        }
                    }
                }

                // TERMINAL
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 12
                    color: "#0A0A0A"
                    border.color: Qt.alpha(hue2, 0.3)

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        Rectangle {
                            Layout.fillWidth: true
                            height: 32
                            color: "#151515"
                            radius: 12
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 12
                                color: parent.color
                            }
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                Text {
                                    text: "TERMINAL"
                                    font.pixelSize: 10
                                    font.letterSpacing: 1
                                    color: "#888"
                                }
                                Row {
                                    visible: Backend.scriptRunning
                                    spacing: 6
                                    Rectangle {
                                        width: 6
                                        height: 6
                                        radius: 3
                                        color: "#00E676"

                                        SequentialAnimation on opacity {
                                            running: Backend.scriptRunning
                                            loops: Animation.Infinite
                                            NumberAnimation {
                                                to: 0.3
                                                duration: 500
                                            }
                                            NumberAnimation {
                                                to: 1
                                                duration: 500
                                            }
                                        }
                                    }
                                    Text {
                                        text: "LIVE"
                                        font.pixelSize: 9
                                        color: "#00E676"
                                    }
                                }
                                Item {
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            TextArea {
                                readOnly: true
                                text: Backend.currentScriptLog || "Neo-Z Terminal v1.0\nType a command below and press Enter.\n\n"
                                color: "#00E676"
                                font.family: "Consolas"
                                font.pixelSize: 12
                                wrapMode: TextEdit.Wrap
                                background: null
                                leftPadding: 12
                                rightPadding: 12
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: "#0D0D0D"
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 8
                                Text {
                                    text: ">"
                                    font.family: "Consolas"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: hue2
                                }
                                TextInput {
                                    id: commandInput
                                    Layout.fillWidth: true
                                    color: "#FFF"
                                    font.family: "Consolas"
                                    font.pixelSize: 12
                                    clip: true
                                    Text {
                                        anchors.fill: parent
                                        text: "Enter command..."
                                        color: "#444"
                                        font.family: "Consolas"
                                        font.pixelSize: 12
                                        visible: !commandInput.text && !commandInput.activeFocus
                                    }
                                    onAccepted: {
                                        if (text.trim()) {
                                            Backend.runAdbCommand(text.trim());
                                            text = "";
                                        }
                                    }
                                }
                                Rectangle {
                                    width: 50
                                    height: 26
                                    radius: 4
                                    color: commandInput.text ? hue2 : "#333"
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Run"
                                        font.pixelSize: 10
                                        font.bold: true
                                        color: commandInput.text ? "#000" : "#666"
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: commandInput.text.length > 0
                                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                        onClicked: commandInput.accepted()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
