import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NeoZ
import "../components"

// Glass popup dialog for emulator scanning and selection
Rectangle {
    id: root
    anchors.fill: parent
    color: "#B3000000"
    visible: false
    z: 1000

    property string tempSelectedDevice: ""
    property bool isScanning: false
    property string statusText: "Ready to scan"

    signal deviceSelected(string deviceId)
    signal closed

    // Show the dialog
    function show() {
        visible = true;
        fadeIn.start();
        tempSelectedDevice = Backend.selectedDevice;
        startScan();
    }

    // Hide the dialog
    function hide() {
        fadeOut.start();
    }

    // Start scanning
    function startScan() {
        root.isScanning = true;
        root.statusText = "Scanning for emulators...";
        Backend.scanForDevices();
        scanTimer.start();
    }

    Timer {
        id: scanTimer
        interval: 2000
        onTriggered: {
            root.isScanning = false;
            root.updateStatusText();
        }
    }

    function updateStatusText() {
        if (Backend.adbDevices.length > 0) {
            root.statusText = Backend.adbDevices.length + " device(s) found";
        } else if (Backend.adbStatus === "No ADB") {
            root.statusText = "ADB not found. Install Android SDK.";
        } else {
            root.statusText = "No emulators found.";
        }
    }

    Connections {
        target: Backend
        function onDevicesChanged() {
            if (!root.isScanning) {
                root.updateStatusText();
            }
        }
    }

    NumberAnimation on opacity {
        id: fadeIn
        from: 0
        to: 1
        duration: 200
        running: false
    }

    NumberAnimation on opacity {
        id: fadeOut
        from: 1
        to: 0
        duration: 200
        running: false
        onFinished: {
            root.visible = false;
            root.closed();
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.hide()
    }

    // Main dialog panel
    GlassPanel {
        id: dialogPanel
        width: Math.min(500, parent.width - 60)
        height: Math.min(480, parent.height - 60)
        anchors.centerIn: parent
        glassOpacity: 1.0
        color: "#121212" // Black Matte Clay look
        glowEnabled: true
        glowColor: Style.primary

        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 25
            spacing: 20

            // Header
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Emulator Scanner"
                    color: Style.textPrimary
                    font.pixelSize: 20
                    font.bold: true
                }

                Item {
                    Layout.fillWidth: true
                }

                // ADB Status Badge
                Rectangle {
                    width: adbStatusText.width + 20
                    height: 24
                    radius: 12
                    color: Backend.adbStatus === "Connected" ? Qt.rgba(0, 230 / 255, 118 / 255, 0.2) : Qt.rgba(255 / 255, 23 / 255, 68 / 255, 0.2)
                    border.color: Backend.adbStatus === "Connected" ? Style.success : Style.danger
                    border.width: 1

                    Text {
                        id: adbStatusText
                        anchors.centerIn: parent
                        text: Backend.adbStatus === "Connected" ? "ADB Online" : "ADB Offline"
                        color: Backend.adbStatus === "Connected" ? Style.success : Style.danger
                        font.pixelSize: 10
                        font.weight: Font.Medium
                    }
                }

                Item {
                    width: 10
                }

                // Close button
                Rectangle {
                    width: 28
                    height: 28
                    radius: 14
                    color: "#1AFFFFFF"

                    Text {
                        anchors.centerIn: parent
                        text: "X"
                        color: Style.textSecondary
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.hide()
                    }
                }
            }

            // Status bar
            Rectangle {
                Layout.fillWidth: true
                height: 40
                radius: 8
                color: "#4D000000"
                border.color: root.isScanning ? Style.primary : Style.surfaceHighlight
                border.width: 1

                Item {
                    anchors.fill: parent
                    anchors.leftMargin: 15
                    anchors.rightMargin: 15

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10

                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            color: {
                                if (root.isScanning)
                                    return Style.primary;
                                if (Backend.adbDevices.length > 0)
                                    return Style.success;
                                if (Backend.adbStatus === "No ADB")
                                    return Style.danger;
                                return Style.textSecondary;
                            }

                            SequentialAnimation on opacity {
                                running: root.isScanning
                                loops: Animation.Infinite
                                NumberAnimation {
                                    to: 0.3
                                    duration: 400
                                }
                                NumberAnimation {
                                    to: 1.0
                                    duration: 400
                                }
                            }
                        }

                        Text {
                            text: root.statusText
                            color: Style.textPrimary
                            font.pixelSize: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Rectangle {
                        width: 70
                        height: 26
                        radius: 6
                        color: root.isScanning ? "#1AFFFFFF" : Style.primary
                        anchors.centerIn: parent

                        Text {
                            anchors.centerIn: parent
                            text: root.isScanning ? "Scanning..." : "Scan"
                            color: root.isScanning ? Style.textSecondary : Style.background
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: root.isScanning ? Qt.ArrowCursor : Qt.PointingHandCursor
                            enabled: !root.isScanning
                            onClicked: root.startScan()
                        }
                    }
                }
            }

            // Device list
            GlassPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                glassOpacity: 0.3

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10

                    Text {
                        text: "Available Emulators"
                        color: Style.textSecondary
                        font.pixelSize: 11
                    }

                    // Scanning animation
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: root.isScanning

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 15

                            Rectangle {
                                width: 40
                                height: 40
                                radius: 20
                                color: "transparent"
                                border.width: 3
                                border.color: Style.primary
                                Layout.alignment: Qt.AlignHCenter

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: Style.primary
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.top: parent.top
                                    anchors.topMargin: 3
                                }

                                RotationAnimation on rotation {
                                    from: 0
                                    to: 360
                                    duration: 1000
                                    loops: Animation.Infinite
                                    running: root.isScanning
                                }
                            }

                            Text {
                                text: "Scanning ADB devices..."
                                color: Style.textSecondary
                                font.pixelSize: 12
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }

                    // Device list view
                    ListView {
                        id: deviceListView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: !root.isScanning
                        clip: true
                        spacing: 8
                        model: Backend.adbDevices

                        delegate: Rectangle {
                            width: deviceListView.width
                            height: 55
                            radius: 8
                            color: tempSelectedDevice === modelData ? "#3300E5FF" : "#0DFFFFFF"
                            border.width: tempSelectedDevice === modelData ? 2 : 1
                            border.color: tempSelectedDevice === modelData ? Style.primary : Style.surfaceHighlight

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12

                                Rectangle {
                                    width: 10
                                    height: 10
                                    radius: 5
                                    color: Style.success
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Text {
                                        text: {
                                            var deviceId = modelData;
                                            if (deviceId.indexOf("5555") >= 0)
                                                return "BlueStacks";
                                            if (deviceId.indexOf("62001") >= 0)
                                                return "NoxPlayer";
                                            if (deviceId.indexOf("21503") >= 0)
                                                return "LDPlayer";
                                            if (deviceId.indexOf("7555") >= 0)
                                                return "MuMu Player";
                                            if (deviceId.indexOf("emulator") >= 0)
                                                return "Android Emulator";
                                            return "Android Device";
                                        }
                                        color: Style.textPrimary
                                        font.pixelSize: 13
                                        font.bold: true
                                    }

                                    Text {
                                        text: modelData
                                        color: Style.textSecondary
                                        font.pixelSize: 11
                                    }
                                }

                                Rectangle {
                                    width: 70
                                    height: 28
                                    radius: 6
                                    color: tempSelectedDevice === modelData ? Style.success : Style.primary

                                    Text {
                                        anchors.centerIn: parent
                                        text: tempSelectedDevice === modelData ? "Selected" : "Select"
                                        color: Style.background
                                        font.pixelSize: 11
                                        font.bold: true
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            tempSelectedDevice = modelData;
                                        }
                                    }
                                }
                            }
                        }

                        // Separator
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Style.surfaceHighlight
                            visible: Backend.installedEmulators.length > 0
                        }

                        Text {
                            text: "Installed Emulators (Launch)"
                            color: Style.textSecondary
                            font.pixelSize: 11
                            visible: Backend.installedEmulators.length > 0
                        }

                        // Installed Emulators List
                        ListView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.min(150, contentHeight)
                            visible: Backend.installedEmulators.length > 0
                            clip: true
                            spacing: 8
                            model: Backend.installedEmulators

                            delegate: Rectangle {
                                width: parent.width
                                height: 50
                                radius: 8
                                color: "#0DFFFFFF"
                                border.width: 1
                                border.color: Style.surfaceHighlight

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 12

                                    Text {
                                        text: modelData.icon
                                        font.pixelSize: 16
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Text {
                                            text: modelData.name
                                            color: Style.textPrimary
                                            font.pixelSize: 13
                                            font.bold: true
                                        }

                                        Text {
                                            text: "Click Open to launch"
                                            color: Style.textSecondary
                                            font.pixelSize: 10
                                        }
                                    }

                                    Rectangle {
                                        width: 60
                                        height: 26
                                        radius: 6
                                        color: Style.secondary

                                        Text {
                                            anchors.centerIn: parent
                                            text: "Open"
                                            color: Style.background
                                            font.pixelSize: 11
                                            font.bold: true
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                Backend.launchEmulator(modelData.path);
                                                root.hide();
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: !root.isScanning && Backend.adbDevices.length === 0
                            text: Backend.adbStatus === "No ADB" ? "ADB not found.\nInstall Android SDK platform-tools." : "No emulators found.\nMake sure your emulator is running."
                            color: Style.textSecondary
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }
            }

            // Action buttons - Three buttons in a row
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                // Kill ADB button (neon purple)
                Rectangle {
                    Layout.preferredWidth: 100
                    height: 36
                    radius: 8
                    color: "#33E040FB"
                    border.color: Style.secondary
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Kill ADB"
                        color: Style.secondary
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.statusText = "Killing ADB server...";
                            Backend.disconnectAdb();
                            root.tempSelectedDevice = "";
                            root.statusText = "ADB server stopped";
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // Disconnect button (grey glass, turns red when connected)
                Rectangle {
                    Layout.preferredWidth: 110
                    height: 36
                    radius: 8
                    color: Backend.adbDevices.length > 0 ? "#33FF1744" : "#1AFFFFFF"
                    border.color: Backend.adbDevices.length > 0 ? Style.danger : Style.textSecondary
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Disconnect"
                        color: Backend.adbDevices.length > 0 ? Style.danger : Style.textSecondary
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Backend.adbDevices.length > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                        enabled: Backend.adbDevices.length > 0
                        onClicked: {
                            root.statusText = "Disconnecting...";
                            Backend.disconnectAdb();
                            root.tempSelectedDevice = "";
                            root.statusText = "Disconnected";
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // OK / Connect button
                Rectangle {
                    Layout.preferredWidth: 100
                    height: 36
                    radius: 8
                    color: tempSelectedDevice !== "" ? Style.success : Style.primary

                    Text {
                        anchors.centerIn: parent
                        text: "OK"
                        color: Style.background
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (tempSelectedDevice !== "") {
                                Backend.selectedDevice = tempSelectedDevice;
                            }
                            root.hide();
                        }
                    }
                }
            }
        }
    }
}
