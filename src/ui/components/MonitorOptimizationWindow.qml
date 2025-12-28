import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import NeoZ
import "../style"

// MonitorOptimizationWindow - Premium Glass Black Galaxy UI
// "Monitor Optimization Core" - Luxury glassmorphism command center
Item {
    id: root
    anchors.fill: parent
    visible: false
    z: 2000

    property bool isOpen: false
    property bool adbConnected: Backend.adbStatus === "Connected"
    property bool isIdle: false

    // Change tracking
    property bool hasChanges: false
    property real savedXMultiplier: 0
    property real savedYMultiplier: 0
    property real savedSlowZone: 35
    property real savedSmoothing: 40

    // Current values (bound to sliders)
    property real currentXMultiplier: Backend.xMultiplier
    property real currentYMultiplier: Backend.yMultiplier
    property real currentSlowZone: 35
    property real currentSmoothing: 40

    // Check for changes
    function checkChanges() {
        root.hasChanges = (root.currentXMultiplier !== root.savedXMultiplier) || (root.currentYMultiplier !== root.savedYMultiplier) || (root.currentSlowZone !== root.savedSlowZone) || (root.currentSmoothing !== root.savedSmoothing);
    }

    // Idle timer for ambient mode
    Timer {
        id: idleTimer
        interval: 10000
        onTriggered: root.isIdle = true
    }

    function resetIdle() {
        root.isIdle = false;
        idleTimer.restart();
    }

    function open() {
        root.visible = true;
        root.isOpen = true;
        // Save current values on open
        root.savedXMultiplier = Backend.xMultiplier;
        root.savedYMultiplier = Backend.yMultiplier;
        root.currentXMultiplier = Backend.xMultiplier;
        root.currentYMultiplier = Backend.yMultiplier;
        root.hasChanges = false;
        openAnim.start();
        idleTimer.start();
    }

    function tryClose() {
        if (root.hasChanges) {
            discardPopup.visible = true;
        } else {
            close();
        }
    }

    function close() {
        closeAnim.start();
        idleTimer.stop();
    }

    function applyChanges() {
        // Apply to backend
        Backend.xMultiplier = root.currentXMultiplier;
        Backend.yMultiplier = root.currentYMultiplier;
        // Save as new baseline
        root.savedXMultiplier = root.currentXMultiplier;
        root.savedYMultiplier = root.currentYMultiplier;
        root.savedSlowZone = root.currentSlowZone;
        root.savedSmoothing = root.currentSmoothing;
        root.hasChanges = false;
    }

    function discardChanges() {
        // Restore saved values
        root.currentXMultiplier = root.savedXMultiplier;
        root.currentYMultiplier = root.savedYMultiplier;
        root.currentSlowZone = root.savedSlowZone;
        root.currentSmoothing = root.savedSmoothing;
        root.hasChanges = false;
    }

    // Mouse movement resets idle
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
        onPositionChanged: resetIdle()
        onPressed: function (mouse) {
            mouse.accepted = false;
            resetIdle();
        }
    }

    // Backdrop with galaxy background
    Rectangle {
        id: backdrop
        anchors.fill: parent
        color: "transparent"
        opacity: 0

        GalaxyBackground {
            anchors.fill: parent
            adbConnected: root.adbConnected
        }

        // Backdrop click - no action (only X button closes)
        MouseArea {
            anchors.fill: parent
            // Disabled: onClicked: root.tryClose() - only X button closes
        }
    }

    // Main glass panel
    Rectangle {
        id: mainPanel
        anchors.centerIn: parent
        width: Math.min(parent.width - 40, 1280)
        height: Math.min(parent.height - 60, 800)
        radius: 26
        color: Qt.rgba(0.04, 0.05, 0.08, 0.85)
        border.color: Qt.rgba(1, 1, 1, 0.08)
        border.width: 1
        clip: true
        opacity: 0
        scale: 0.92

        // Idle dim effect
        Behavior on opacity {
            NumberAnimation {
                duration: 400
            }
        }

        // Glass frost overlay
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Qt.rgba(0.06, 0.07, 0.1, 0.4)
        }

        // Top edge glass shine
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            anchors.topMargin: 1
            height: 1
            radius: parent.radius
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
                GradientStop {
                    position: 0.3
                    color: Qt.rgba(1, 1, 1, 0.2)
                }
                GradientStop {
                    position: 0.5
                    color: Qt.rgba(1, 1, 1, 0.35)
                }
                GradientStop {
                    position: 0.7
                    color: Qt.rgba(1, 1, 1, 0.2)
                }
                GradientStop {
                    position: 1.0
                    color: "transparent"
                }
            }
        }

        // ADB Connected edge glow
        Rectangle {
            anchors.fill: parent
            anchors.margins: -4
            radius: parent.radius + 4
            color: "transparent"
            border.color: "#6EEBFF"
            border.width: 2
            opacity: root.adbConnected ? 0.35 : 0
            visible: root.adbConnected

            SequentialAnimation on opacity {
                running: root.adbConnected && root.isOpen
                loops: Animation.Infinite
                NumberAnimation {
                    from: 0.35
                    to: 0.2
                    duration: 1800
                    easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    from: 0.2
                    to: 0.35
                    duration: 1800
                    easing.type: Easing.InOutSine
                }
            }
        }

        // Corner glow pulses when ADB connected
        Repeater {
            model: 4
            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: "#6EEBFF"
                opacity: root.adbConnected ? 0.3 : 0
                visible: root.adbConnected

                x: index % 2 === 0 ? -5 : mainPanel.width - 15
                y: index < 2 ? -5 : mainPanel.height - 15

                SequentialAnimation on opacity {
                    running: root.adbConnected && root.isOpen
                    loops: Animation.Infinite
                    NumberAnimation {
                        from: 0.3
                        to: 0.1
                        duration: 1500 + index * 200
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        from: 0.1
                        to: 0.3
                        duration: 1500 + index * 200
                        easing.type: Easing.InOutSine
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 16
            clip: true
            opacity: 1.0

            // ========== HEADER ==========
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                spacing: 16

                // Neo-Z emblem with waveform
                Rectangle {
                    width: 44
                    height: 44
                    radius: 10
                    color: Qt.rgba(1, 1, 1, 0.04)
                    border.color: Qt.rgba(1, 1, 1, 0.1)
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: -4
                        text: "N"
                        color: "#6EEBFF"
                        font.pixelSize: 22
                        font.bold: true
                        font.family: "Segoe UI"
                    }

                    // Animated waveform
                    Canvas {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 4
                        height: 10

                        property real phase: 0
                        NumberAnimation on phase {
                            from: 0
                            to: Math.PI * 2
                            duration: 2000
                            loops: Animation.Infinite
                        }

                        onPhaseChanged: requestPaint()

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            ctx.beginPath();
                            ctx.moveTo(0, height / 2);
                            for (var x = 0; x < width; x++) {
                                var y = height / 2 + Math.sin(x * 0.4 + phase) * 2.5;
                                ctx.lineTo(x, y);
                            }
                            ctx.strokeStyle = "#6EEBFF";
                            ctx.lineWidth = 1.5;
                            ctx.stroke();
                        }
                    }
                }

                // Title section
                Column {
                    spacing: 3

                    Text {
                        text: "MONITOR · INPUT · RESOLUTION ENGINE"
                        color: "#E6EAF0"
                        font.pixelSize: 15
                        font.bold: true
                        font.letterSpacing: 2
                    }

                    Row {
                        spacing: 8

                        Text {
                            text: "Real-time Sensitivity Normalization Active"
                            color: "#9AA4B2"
                            font.pixelSize: 11
                        }

                        Rectangle {
                            width: 6
                            height: 6
                            radius: 3
                            color: "#00C853"
                            anchors.verticalCenter: parent.verticalCenter

                            SequentialAnimation on opacity {
                                loops: Animation.Infinite
                                NumberAnimation {
                                    from: 1
                                    to: 0.3
                                    duration: 1000
                                }
                                NumberAnimation {
                                    from: 0.3
                                    to: 1
                                    duration: 1000
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // Status pills
                Row {
                    spacing: 8

                    // Resolution pill
                    Rectangle {
                        width: resText.width + 24
                        height: 30
                        radius: 8
                        color: Qt.rgba(0.1, 0.5, 0.9, 0.12)
                        border.color: "#6EEBFF"
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 6
                            Text {
                                text: "RES"
                                color: "#6EEBFF"
                                font.pixelSize: 9
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                id: resText
                                text: Backend.resolution || "2560×1440"
                                color: "#E6EAF0"
                                font.pixelSize: 11
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    // Refresh rate pill
                    Rectangle {
                        width: 80
                        height: 30
                        radius: 8
                        color: Qt.rgba(0.6, 0.5, 1, 0.1)
                        border.color: "#9A8CFF"
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 6
                            Text {
                                text: "Hz"
                                color: "#9A8CFF"
                                font.pixelSize: 9
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: "144"
                                color: "#E6EAF0"
                                font.pixelSize: 11
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    // Polling pill
                    Rectangle {
                        width: 85
                        height: 30
                        radius: 8
                        color: Qt.rgba(0.6, 0.5, 1, 0.1)
                        border.color: "#9A8CFF"
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 6
                            Text {
                                text: "POLL"
                                color: "#9A8CFF"
                                font.pixelSize: 9
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: "1000"
                                color: "#E6EAF0"
                                font.pixelSize: 11
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    // ADB status pill
                    Rectangle {
                        width: 110
                        height: 30
                        radius: 8
                        color: root.adbConnected ? Qt.rgba(0, 0.8, 0.3, 0.12) : Qt.rgba(0.3, 0.3, 0.3, 0.2)
                        border.color: root.adbConnected ? "#00C853" : "#666"
                        border.width: 1
                        Row {
                            anchors.centerIn: parent
                            spacing: 6
                            Text {
                                text: "ADB"
                                color: root.adbConnected ? "#00C853" : "#666"
                                font.pixelSize: 9
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: root.adbConnected ? "CONNECTED" : "OFFLINE"
                                color: root.adbConnected ? "#00C853" : "#888"
                                font.pixelSize: 10
                            }
                        }
                    }

                    // ========== PRESET CONFIDENCE BADGE ==========
                    // 0 = Native, 1 = Scaled, 2 = Mismatch
                    Rectangle {
                        width: 95
                        height: 30
                        radius: 8
                        color: {
                            var conf = Backend.presetConfidence || 0;
                            if (conf === 0)
                                return Qt.rgba(0, 0.8, 0.3, 0.12);  // Native
                            if (conf === 1)
                                return Qt.rgba(1, 0.7, 0, 0.12);    // Scaled
                            return Qt.rgba(1, 0.3, 0.3, 0.12);                  // Mismatch
                        }
                        border.color: {
                            var conf = Backend.presetConfidence || 0;
                            if (conf === 0)
                                return "#00C853";
                            if (conf === 1)
                                return "#FFAB40";
                            return "#FF5252";
                        }
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 5

                            Text {
                                text: {
                                    var conf = Backend.presetConfidence || 0;
                                    if (conf === 0)
                                        return "✔";
                                    if (conf === 1)
                                        return "⚠";
                                    return "✖";
                                }
                                color: {
                                    var conf = Backend.presetConfidence || 0;
                                    if (conf === 0)
                                        return "#00C853";
                                    if (conf === 1)
                                        return "#FFAB40";
                                    return "#FF5252";
                                }
                                font.pixelSize: 12
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: {
                                    var conf = Backend.presetConfidence || 0;
                                    if (conf === 0)
                                        return "Native";
                                    if (conf === 1)
                                        return "Scaled";
                                    return "Mismatch";
                                }
                                color: {
                                    var conf = Backend.presetConfidence || 0;
                                    if (conf === 0)
                                        return "#00C853";
                                    if (conf === 1)
                                        return "#FFAB40";
                                    return "#FF5252";
                                }
                                font.pixelSize: 10
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }

                // ========== INPUT AUTHORITY TOGGLE ==========
                Rectangle {
                    width: 140
                    height: 36
                    radius: 10
                    color: Backend.inputAuthorityEnabled ? Qt.rgba(1, 0.2, 0.2, 0.15) : Qt.rgba(0.5, 0.5, 0.5, 0.1)
                    border.color: Backend.inputAuthorityEnabled ? "#FF5252" : "#666"
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: 8

                        Rectangle {
                            width: 32
                            height: 18
                            radius: 9
                            color: Backend.inputAuthorityEnabled ? "#FF5252" : "#444"
                            anchors.verticalCenter: parent.verticalCenter

                            Rectangle {
                                width: 14
                                height: 14
                                radius: 7
                                color: "#E6EAF0"
                                anchors.verticalCenter: parent.verticalCenter
                                x: Backend.inputAuthorityEnabled ? 16 : 2
                                Behavior on x {
                                    NumberAnimation {
                                        duration: 150
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Backend.inputAuthorityEnabled = !Backend.inputAuthorityEnabled
                            }
                        }

                        Text {
                            text: Backend.inputAuthorityEnabled ? "AUTHORITY" : "READ-ONLY"
                            color: Backend.inputAuthorityEnabled ? "#FF5252" : "#9AA4B2"
                            font.pixelSize: 9
                            font.bold: true
                            font.letterSpacing: 1
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                // Close button
                Rectangle {
                    width: 36
                    height: 36
                    radius: 10
                    color: closeBtn.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent"
                    border.color: closeBtn.containsMouse ? Qt.rgba(1, 1, 1, 0.15) : "transparent"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        color: closeBtn.containsMouse ? "#E6EAF0" : "#9AA4B2"
                        font.pixelSize: 18
                    }

                    MouseArea {
                        id: closeBtn
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.tryClose()
                    }
                }
            }

            // ========== MAIN 3-COLUMN LAYOUT ==========
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 20

                // ========== LEFT: Monitor Intelligence ==========
                Rectangle {
                    Layout.preferredWidth: 260
                    Layout.fillHeight: true
                    radius: 18
                    color: Qt.rgba(0.05, 0.06, 0.09, 0.7)
                    border.color: Qt.rgba(1, 1, 1, 0.06)
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 14

                        Text {
                            text: "Monitor Intelligence"
                            color: "#E6EAF0"
                            font.pixelSize: 13
                            font.bold: true
                            font.letterSpacing: 1
                        }

                        // Resolution metric
                        Rectangle {
                            Layout.fillWidth: true
                            height: 60
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.35)

                            Row {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12

                                Text {
                                    text: "🖥️"
                                    font.pixelSize: 20
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 2
                                    Text {
                                        text: "Resolution"
                                        color: "#9AA4B2"
                                        font.pixelSize: 10
                                    }
                                    Text {
                                        text: Backend.resolution || "2560 × 1440"
                                        color: "#E6EAF0"
                                        font.pixelSize: 15
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        // Aspect ratio bar
                        Rectangle {
                            Layout.fillWidth: true
                            height: 50
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.35)

                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 6

                                Text {
                                    text: "Aspect Ratio"
                                    color: "#9AA4B2"
                                    font.pixelSize: 10
                                }

                                Rectangle {
                                    width: parent.width
                                    height: 10
                                    radius: 5
                                    color: Qt.rgba(1, 1, 1, 0.08)

                                    Rectangle {
                                        width: parent.width * 0.5625  // 16:9
                                        height: parent.height
                                        radius: 5
                                        gradient: Gradient {
                                            orientation: Gradient.Horizontal
                                            GradientStop {
                                                position: 0
                                                color: "#6EEBFF"
                                            }
                                            GradientStop {
                                                position: 1
                                                color: "#9A8CFF"
                                            }
                                        }
                                    }

                                    Text {
                                        anchors.right: parent.right
                                        anchors.rightMargin: 8
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: "16:9"
                                        color: "#E6EAF0"
                                        font.pixelSize: 9
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        // Refresh rate with pulse
                        Rectangle {
                            Layout.fillWidth: true
                            height: 60
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.35)

                            Row {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12

                                Rectangle {
                                    width: 28
                                    height: 28
                                    radius: 14
                                    color: "#9A8CFF"
                                    opacity: 0.2
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        anchors.centerIn: parent
                                        text: "⚡"
                                        font.pixelSize: 14
                                    }

                                    SequentialAnimation on opacity {
                                        loops: Animation.Infinite
                                        NumberAnimation {
                                            from: 0.2
                                            to: 0.4
                                            duration: 694
                                        }  // 144Hz = 6.94ms
                                        NumberAnimation {
                                            from: 0.4
                                            to: 0.2
                                            duration: 694
                                        }
                                    }
                                }

                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 2
                                    Text {
                                        text: "Refresh Rate"
                                        color: "#9AA4B2"
                                        font.pixelSize: 10
                                    }
                                    Text {
                                        text: "144 Hz"
                                        color: "#E6EAF0"
                                        font.pixelSize: 15
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        // Frame time sparkline
                        Rectangle {
                            Layout.fillWidth: true
                            height: 55
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.35)

                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 4

                                Row {
                                    width: parent.width
                                    Text {
                                        text: "Frame Time"
                                        color: "#9AA4B2"
                                        font.pixelSize: 10
                                    }
                                    Item {
                                        width: parent.width - 100
                                    }
                                    Text {
                                        text: "6.94 ms"
                                        color: "#6EEBFF"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                }

                                Canvas {
                                    width: parent.width
                                    height: 20

                                    property real phase: 0
                                    NumberAnimation on phase {
                                        from: 0
                                        to: 1
                                        duration: 3000
                                        loops: Animation.Infinite
                                    }
                                    onPhaseChanged: requestPaint()

                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.clearRect(0, 0, width, height);
                                        ctx.beginPath();
                                        ctx.moveTo(0, height / 2);
                                        for (var x = 0; x < width; x++) {
                                            var noise = Math.sin(x * 0.2 + phase * 20) * 3 + Math.sin(x * 0.5 + phase * 10) * 2;
                                            ctx.lineTo(x, height / 2 + noise);
                                        }
                                        var gradient = ctx.createLinearGradient(0, 0, width, 0);
                                        gradient.addColorStop(0, "#6EEBFF");
                                        gradient.addColorStop(1, "#9A8CFF");
                                        ctx.strokeStyle = gradient;
                                        ctx.lineWidth = 1.5;
                                        ctx.stroke();
                                    }
                                }
                            }
                        }

                        // Adaptive sync
                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.35)

                            Row {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 10

                                Text {
                                    text: "Adaptive Sync"
                                    color: "#9AA4B2"
                                    font.pixelSize: 11
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Item {
                                    width: 1
                                    Layout.fillWidth: true
                                }
                                Rectangle {
                                    width: 45
                                    height: 22
                                    radius: 11
                                    color: Qt.rgba(0, 0.8, 0.3, 0.2)
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        anchors.centerIn: parent
                                        text: "ON"
                                        color: "#00C853"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }

                // ========== CENTER: Sensitivity Math Engine ==========
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 18
                    color: Qt.rgba(0.05, 0.06, 0.09, 0.7)
                    border.color: Qt.rgba(1, 1, 1, 0.06)
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 20

                        Text {
                            text: "Sensitivity Math Engine"
                            color: "#E6EAF0"
                            font.pixelSize: 13
                            font.bold: true
                            font.letterSpacing: 1
                        }

                        // X/Y Axis sliders
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 24

                            // X Multiplier
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Text {
                                    text: "X Axis Multiplier"
                                    color: "#9AA4B2"
                                    font.pixelSize: 11
                                }
                                GlassSlider {
                                    id: xSlider
                                    Layout.fillWidth: true
                                    height: 50
                                    value: Backend.xMultiplier
                                    from: -1
                                    to: 1
                                    centerZero: true
                                    unit: "x"
                                    onValueChanged: {
                                        Backend.xMultiplier = value;
                                        root.currentXMultiplier = value;
                                        root.checkChanges();
                                    }
                                }
                                Text {
                                    text: "Resolution-normalized gain"
                                    color: "#666"
                                    font.pixelSize: 9
                                    font.italic: true
                                }
                            }

                            // Y Multiplier
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Text {
                                    text: "Y Axis Multiplier"
                                    color: "#9AA4B2"
                                    font.pixelSize: 11
                                }
                                GlassSlider {
                                    id: ySlider
                                    Layout.fillWidth: true
                                    height: 50
                                    value: Backend.yMultiplier
                                    from: -1
                                    to: 1
                                    centerZero: true
                                    unit: "x"
                                    onValueChanged: {
                                        Backend.yMultiplier = value;
                                        root.currentYMultiplier = value;
                                        root.checkChanges();
                                    }
                                }
                                Text {
                                    text: "Hz-locked time correction"
                                    color: "#666"
                                    font.pixelSize: 9
                                    font.italic: true
                                }
                            }
                        }

                        // Slow Zone and Smoothing
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 28

                            // Slow Zone radial
                            ColumnLayout {
                                Layout.alignment: Qt.AlignHCenter
                                spacing: 10

                                RadialSlider {
                                    id: slowZoneSlider
                                    Layout.alignment: Qt.AlignHCenter
                                    value: Backend.velocityLowThreshold * 100
                                    label: "Slow Zone"
                                    onValueChanged: {
                                        Backend.velocityLowThreshold = value / 100;
                                        root.currentSlowZone = value;
                                        curveGraph.requestPaint();  // Refresh graph
                                        root.checkChanges();
                                    }
                                }

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Velocity falloff control"
                                    color: "#666"
                                    font.pixelSize: 9
                                }
                            }

                            // Smoothing
                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: 14

                                Text {
                                    text: "Input Smoothing"
                                    color: "#9AA4B2"
                                    font.pixelSize: 11
                                }

                                GlassSlider {
                                    id: smoothingSlider
                                    Layout.fillWidth: true
                                    height: 50
                                    value: Backend.smoothing
                                    from: 0
                                    to: 200
                                    unit: " ms"
                                    decimals: 0
                                    onValueChanged: {
                                        Backend.smoothing = value;
                                    }
                                }

                                // Latency indicator
                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 32
                                    radius: 8
                                    color: Qt.rgba(0, 0, 0, 0.35)

                                    Row {
                                        anchors.centerIn: parent
                                        spacing: 8
                                        Text {
                                            text: "Effective latency:"
                                            color: "#9AA4B2"
                                            font.pixelSize: 10
                                        }
                                        Text {
                                            text: "42 ms"
                                            color: "#FFAB40"
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                    }
                                }

                                Item {
                                    Layout.fillHeight: true
                                }
                            }
                        }
                    }
                }

                // ========== RIGHT: Adaptive Sensitivity Matrix ==========
                Rectangle {
                    Layout.preferredWidth: 280
                    Layout.fillHeight: true
                    radius: 18
                    color: Qt.rgba(0.05, 0.06, 0.09, 0.7)
                    border.color: Qt.rgba(1, 1, 1, 0.06)
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 14

                        Text {
                            text: "Adaptive Sensitivity Matrix"
                            color: "#E6EAF0"
                            font.pixelSize: 13
                            font.bold: true
                            font.letterSpacing: 1
                        }

                        // Animated 4-Point Radar Chart
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 200
                            radius: 14
                            color: Qt.rgba(0, 0, 0, 0.4)

                            Canvas {
                                id: curveGraph
                                anchors.fill: parent
                                anchors.margins: 10

                                // Continuous animation phase (0-1 cycles every 2 seconds)
                                property real animPhase: 0
                                property real scanAngle: 0

                                NumberAnimation on animPhase {
                                    from: 0
                                    to: 1
                                    duration: 2000
                                    loops: Animation.Infinite
                                }

                                NumberAnimation on scanAngle {
                                    from: 0
                                    to: 360
                                    duration: 4000
                                    loops: Animation.Infinite
                                }

                                onAnimPhaseChanged: requestPaint()
                                onScanAngleChanged: requestPaint()

                                onPaint: {
                                    var ctx = getContext("2d");
                                    ctx.clearRect(0, 0, width, height);

                                    // ===== GET 5 DATA POINTS (including ADB) =====
                                    var xMult = Math.abs(Backend.xMultiplier || 1.0);
                                    var yMult = Math.abs(Backend.yMultiplier || 1.0);
                                    var smoothingNorm = (Backend.smoothing || 0) / 200;
                                    var velocityFalloff = (Backend.velocityLowThreshold || 0.5) / 10;
                                    var adbActive = root.adbConnected ? 1.0 : 0.0;

                                    // Center of the radar
                                    var cx = width / 2;
                                    var cy = height / 2;
                                    var maxRadius = Math.min(width, height) * 0.36;

                                    // Animation effects
                                    var pulse = Math.sin(animPhase * Math.PI * 2) * 0.1 + 1.0;
                                    var scanRad = scanAngle * Math.PI / 180;

                                    // ===== BACKGROUND GLOW =====
                                    var bgGrad = ctx.createRadialGradient(cx, cy, 0, cx, cy, maxRadius * 1.3);
                                    bgGrad.addColorStop(0, "rgba(110, 235, 255, 0.05)");
                                    bgGrad.addColorStop(0.5, "rgba(154, 140, 255, 0.03)");
                                    bgGrad.addColorStop(1, "rgba(0, 0, 0, 0)");
                                    ctx.fillStyle = bgGrad;
                                    ctx.fillRect(0, 0, width, height);

                                    // ===== ROTATING SCANNER SWEEP =====
                                    ctx.save();
                                    ctx.translate(cx, cy);
                                    ctx.rotate(scanRad);
                                    var sweepGrad = ctx.createLinearGradient(0, 0, maxRadius, 0);
                                    sweepGrad.addColorStop(0, "rgba(110, 235, 255, 0.3)");
                                    sweepGrad.addColorStop(0.7, "rgba(110, 235, 255, 0.05)");
                                    sweepGrad.addColorStop(1, "rgba(0, 0, 0, 0)");
                                    ctx.fillStyle = sweepGrad;
                                    ctx.beginPath();
                                    ctx.moveTo(0, 0);
                                    ctx.arc(0, 0, maxRadius, -0.15, 0.15);
                                    ctx.closePath();
                                    ctx.fill();
                                    ctx.restore();

                                    // ===== ORBITAL RINGS (animated) =====
                                    for (var ring = 0.3; ring <= 1.0; ring += 0.35) {
                                        var ringPulse = Math.sin(animPhase * Math.PI * 2 + ring * 2) * 0.02 + 1.0;
                                        ctx.strokeStyle = "rgba(110, 235, 255, " + (0.08 - ring * 0.03) + ")";
                                        ctx.lineWidth = 1;
                                        ctx.setLineDash([3, 6]);
                                        ctx.beginPath();
                                        ctx.arc(cx, cy, maxRadius * ring * ringPulse, 0, Math.PI * 2);
                                        ctx.stroke();
                                        ctx.setLineDash([]);
                                    }

                                    // ===== DRAW 5 AXIS LINES (pentagon) =====
                                    var numPoints = root.adbConnected ? 5 : 4;
                                    var labels = root.adbConnected ? ["X", "Y", "τ", "V", "ADB"] : ["X", "Y", "τ", "V"];
                                    var colors = root.adbConnected ? ["rgba(110,235,255,1)", "rgba(255,110,154,1)", "rgba(154,140,255,1)", "rgba(110,255,180,1)", "rgba(255,215,0,1)"] : ["rgba(110,235,255,1)", "rgba(255,110,154,1)", "rgba(154,140,255,1)", "rgba(110,255,180,1)"];
                                    var colorsAlpha = root.adbConnected ? ["rgba(110,235,255,0.3)", "rgba(255,110,154,0.3)", "rgba(154,140,255,0.3)", "rgba(110,255,180,0.3)", "rgba(255,215,0,0.3)"] : ["rgba(110,235,255,0.3)", "rgba(255,110,154,0.3)", "rgba(154,140,255,0.3)", "rgba(110,255,180,0.3)"];
                                    var dataPoints = root.adbConnected ? [xMult, yMult, smoothingNorm, velocityFalloff, adbActive] : [xMult, yMult, smoothingNorm, velocityFalloff];

                                    var angles = [];
                                    for (var a = 0; a < numPoints; a++) {
                                        angles.push(-Math.PI / 2 + (a * 2 * Math.PI / numPoints));
                                    }

                                    // Draw axis lines with gradient
                                    for (var i = 0; i < numPoints; i++) {
                                        var ax = cx + Math.cos(angles[i]) * maxRadius;
                                        var ay = cy + Math.sin(angles[i]) * maxRadius;

                                        var axisGrad = ctx.createLinearGradient(cx, cy, ax, ay);
                                        axisGrad.addColorStop(0, "rgba(255, 255, 255, 0.02)");
                                        axisGrad.addColorStop(1, colorsAlpha[i]);
                                        ctx.strokeStyle = "rgba(255, 255, 255, 0.08)";
                                        ctx.lineWidth = 1;
                                        ctx.beginPath();
                                        ctx.moveTo(cx, cy);
                                        ctx.lineTo(ax, ay);
                                        ctx.stroke();

                                        // Pulsing end markers
                                        var markerPulse = Math.sin(animPhase * Math.PI * 2 + i) * 0.3 + 1.0;
                                        ctx.beginPath();
                                        ctx.arc(ax, ay, 3 * markerPulse, 0, Math.PI * 2);
                                        ctx.fillStyle = colors[i];
                                        ctx.fill();

                                        // Labels
                                        ctx.fillStyle = colors[i];
                                        ctx.font = "bold 9px sans-serif";
                                        ctx.textAlign = "center";
                                        ctx.textBaseline = "middle";
                                        ctx.fillText(labels[i], ax + Math.cos(angles[i]) * 12, ay + Math.sin(angles[i]) * 12);
                                    }

                                    // ===== CALCULATE DATA POINTS ON RADAR =====
                                    var points = [];
                                    for (var j = 0; j < numPoints; j++) {
                                        var val = Math.min(1.0, dataPoints[j]) * pulse;
                                        var px = cx + Math.cos(angles[j]) * maxRadius * val;
                                        var py = cy + Math.sin(angles[j]) * maxRadius * val;
                                        points.push({
                                            x: px,
                                            y: py,
                                            val: val,
                                            color: colors[j],
                                            colorAlpha: colorsAlpha[j],
                                            angle: angles[j]
                                        });
                                    }

                                    // ===== DRAW FILLED RADAR SHAPE WITH GLOW =====
                                    // Outer glow (Simulated with stroke opacity instead of shadowBlur for performance)
                                    ctx.beginPath();
                                    ctx.moveTo(points[0].x, points[0].y);
                                    for (var k = 1; k < numPoints; k++) {
                                        ctx.lineTo(points[k].x, points[k].y);
                                    }
                                    ctx.closePath();
                                    ctx.strokeStyle = "rgba(110, 235, 255, 0.7)";
                                    ctx.lineWidth = 2;
                                    ctx.stroke();

                                    // Gradient fill
                                    var fillGrad = ctx.createRadialGradient(cx, cy, 0, cx, cy, maxRadius);
                                    fillGrad.addColorStop(0, root.adbConnected ? "rgba(255, 215, 0, 0.25)" : "rgba(110, 235, 255, 0.25)");
                                    fillGrad.addColorStop(0.5, "rgba(154, 140, 255, 0.15)");
                                    fillGrad.addColorStop(1, "rgba(255, 110, 154, 0.05)");
                                    ctx.fillStyle = fillGrad;
                                    ctx.fill();

                                    // ===== PARTICLE TRAILS (Optimized) =====
                                    for (var p = 0; p < 4; p++) { // Reduced from 8 to 4
                                        var particleAngle = (animPhase * Math.PI * 2 + p * Math.PI / 2) % (Math.PI * 2);
                                        var particleR = maxRadius * 0.3 + Math.sin(animPhase * Math.PI * 4 + p) * maxRadius * 0.2;
                                        var px2 = cx + Math.cos(particleAngle) * particleR;
                                        var py2 = cy + Math.sin(particleAngle) * particleR;
                                        var particleAlpha = 0.3 + Math.sin(animPhase * Math.PI * 2 + p) * 0.2;

                                        ctx.beginPath();
                                        ctx.arc(px2, py2, 2, 0, Math.PI * 2);
                                        ctx.fillStyle = "rgba(110, 235, 255, " + particleAlpha + ")";
                                        ctx.fill();
                                    }

                                    // ===== DRAW DATA POINT NODES (animated) =====
                                    for (var m = 0; m < numPoints; m++) {
                                        var pt = points[m];
                                        var nodePulse = Math.sin(animPhase * Math.PI * 2 + m * 0.8) * 0.4 + 1.0;

                                        // Connecting lines between adjacent nodes
                                        if (m > 0) {
                                            ctx.strokeStyle = "rgba(255, 255, 255, 0.1)";
                                            ctx.lineWidth = 1;
                                            ctx.setLineDash([2, 4]);
                                            ctx.beginPath();
                                            ctx.moveTo(points[m - 1].x, points[m - 1].y);
                                            ctx.lineTo(pt.x, pt.y);
                                            ctx.stroke();
                                            ctx.setLineDash([]);
                                        }

                                        // Outer glow ring
                                        ctx.beginPath();
                                        ctx.arc(pt.x, pt.y, 12 * nodePulse, 0, Math.PI * 2);
                                        var nodeGlow = ctx.createRadialGradient(pt.x, pt.y, 0, pt.x, pt.y, 12 * nodePulse);
                                        nodeGlow.addColorStop(0, pt.color);
                                        nodeGlow.addColorStop(0.5, pt.colorAlpha);
                                        nodeGlow.addColorStop(1, "rgba(0,0,0,0)");
                                        ctx.fillStyle = nodeGlow;
                                        ctx.fill();

                                        // Core dot with ring
                                        ctx.beginPath();
                                        ctx.arc(pt.x, pt.y, 5, 0, Math.PI * 2);
                                        ctx.fillStyle = pt.color;
                                        ctx.fill();
                                        ctx.strokeStyle = "rgba(255, 255, 255, 0.5)";
                                        ctx.lineWidth = 1;
                                        ctx.stroke();
                                    }

                                    // ===== ENERGY CORE (center) =====
                                    var combinedEffect = 0;
                                    for (var e = 0; e < numPoints; e++) {
                                        combinedEffect += dataPoints[e];
                                    }
                                    combinedEffect /= numPoints;
                                    var coreRadius = 12 + combinedEffect * 15 * pulse;

                                    // Rotating energy rings
                                    for (var er = 0; er < 3; er++) {
                                        ctx.save();
                                        ctx.translate(cx, cy);
                                        ctx.rotate(scanRad * (er % 2 === 0 ? 1 : -1) * 0.5);
                                        ctx.strokeStyle = "rgba(255, 255, 255, " + (0.3 - er * 0.1) + ")";
                                        ctx.lineWidth = 1;
                                        ctx.beginPath();
                                        ctx.arc(0, 0, coreRadius + er * 4, 0, Math.PI * 1.2);
                                        ctx.stroke();
                                        ctx.restore();
                                    }

                                    // Core glow
                                    var coreGrad = ctx.createRadialGradient(cx, cy, 0, cx, cy, coreRadius * 1.5);
                                    coreGrad.addColorStop(0, root.adbConnected ? "rgba(255, 215, 0, 0.8)" : "rgba(255, 255, 255, 0.9)");
                                    coreGrad.addColorStop(0.4, root.adbConnected ? "rgba(255, 215, 0, 0.4)" : "rgba(110, 235, 255, 0.4)");
                                    coreGrad.addColorStop(1, "rgba(0, 0, 0, 0)");
                                    ctx.beginPath();
                                    ctx.arc(cx, cy, coreRadius * 1.5, 0, Math.PI * 2);
                                    ctx.fillStyle = coreGrad;
                                    ctx.fill();

                                    // Inner core
                                    ctx.beginPath();
                                    ctx.arc(cx, cy, coreRadius * 0.4, 0, Math.PI * 2);
                                    ctx.fillStyle = root.adbConnected ? "#FFD700" : "#fff";
                                    ctx.fill();

                                    // ===== STATUS LABELS =====
                                    ctx.font = "bold 8px sans-serif";
                                    ctx.textAlign = "right";
                                    ctx.fillStyle = "#6EEBFF";
                                    ctx.fillText("X:" + xMult.toFixed(2), width - 5, 12);
                                    ctx.fillStyle = "#FF6E9A";
                                    ctx.fillText("Y:" + yMult.toFixed(2), width - 5, 24);
                                    ctx.textAlign = "left";
                                    ctx.fillStyle = "#9A8CFF";
                                    ctx.fillText("τ:" + (Backend.smoothing || 0) + "ms", 5, 12);
                                    ctx.fillStyle = "#6EFFB4";
                                    ctx.fillText("V:" + (Backend.velocityLowThreshold || 0.5).toFixed(1), 5, 24);

                                    // ADB Status
                                    if (root.adbConnected) {
                                        ctx.fillStyle = "#FFD700";
                                        ctx.textAlign = "center";
                                        ctx.fillText("⚡ ADB LINKED", cx, height - 8);
                                    } else {
                                        ctx.fillStyle = "#555";
                                        ctx.textAlign = "center";
                                        ctx.fillText("ADB OFFLINE", cx, height - 8);
                                    }
                                }
                            }
                        }

                        // Emulator DPI Control
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 14
                            color: Qt.rgba(0, 0, 0, 0.35)
                            border.color: root.adbConnected ? "#6EEBFF" : Qt.rgba(1, 1, 1, 0.05)
                            border.width: 1
                            opacity: root.adbConnected ? 1 : 0.45

                            Behavior on border.color {
                                ColorAnimation {
                                    duration: 300
                                }
                            }
                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 300
                                }
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 10

                                Row {
                                    spacing: 10
                                    Text {
                                        text: "📱"
                                        font.pixelSize: 18
                                    }
                                    Text {
                                        text: "Emulator DPI Control"
                                        color: root.adbConnected ? "#E6EAF0" : "#666"
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                }

                                Text {
                                    text: root.adbConnected ? "Direct Emulator DPI Control — Active" : "Connect ADB to unlock"
                                    color: root.adbConnected ? "#6EEBFF" : "#666"
                                    font.pixelSize: 10
                                }

                                GlassSlider {
                                    Layout.fillWidth: true
                                    height: 45
                                    enabled: root.adbConnected
                                    value: 320
                                    from: 120
                                    to: 640
                                    decimals: 0
                                    unit: " dpi"
                                    opacity: root.adbConnected ? 1 : 0.4
                                }

                                Item {
                                    Layout.fillHeight: true
                                }
                            }
                        }
                    }
                }
            }

            // ========== FOOTER: Control Strip ==========
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                radius: 16
                color: Qt.rgba(0, 0, 0, 0.4)
                border.color: Qt.rgba(1, 1, 1, 0.05)
                border.width: 1
                clip: true  // Prevent child overflow

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    spacing: 24

                    // Latency Indicator
                    Rectangle {
                        width: 110
                        height: 36
                        radius: 10
                        color: {
                            var lat = Backend.latencyMs || 0;
                            if (lat < 5)
                                return Qt.rgba(0, 0.8, 0.3, 0.15);
                            if (lat < 10)
                                return Qt.rgba(1, 0.7, 0, 0.15);
                            return Qt.rgba(1, 0.3, 0.3, 0.15);
                        }
                        border.color: {
                            var lat = Backend.latencyMs || 0;
                            if (lat < 5)
                                return "#00C853";
                            if (lat < 10)
                                return "#FFAB40";
                            return "#FF5252";
                        }
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 4
                            Text {
                                text: "LATENCY"
                                color: "#888"
                                font.pixelSize: 8
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: (Backend.latencyMs || 0).toFixed(1) + " ms"
                                color: {
                                    var lat = Backend.latencyMs || 0;
                                    if (lat < 5)
                                        return "#00C853";
                                    if (lat < 10)
                                        return "#FFAB40";
                                    return "#FF5252";
                                }
                                font.pixelSize: 10
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    // ========== LIVE VELOCITY TELEMETRY ==========
                    Rectangle {
                        width: 120
                        height: 36
                        radius: 10
                        color: Qt.rgba(0.6, 0.5, 1, 0.1)
                        border.color: "#9A8CFF"
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 4
                            Text {
                                text: "VEL"
                                color: "#888"
                                font.pixelSize: 8
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: (Backend.mouseVelocity || 0).toFixed(0) + " px/s"
                                color: "#9A8CFF"
                                font.pixelSize: 10
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            // Direction indicator (mini arrow)
                            Rectangle {
                                width: 10
                                height: 10
                                radius: 2
                                color: "transparent"
                                anchors.verticalCenter: parent.verticalCenter

                                Canvas {
                                    id: dirCanvas
                                    anchors.fill: parent
                                    property real angle: Backend.mouseAngleDegrees || 0

                                    onAngleChanged: requestPaint()
                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.reset();
                                        ctx.translate(5, 5);
                                        ctx.rotate(angle * Math.PI / 180);
                                        ctx.beginPath();
                                        ctx.moveTo(0, -4);
                                        ctx.lineTo(3, 4);
                                        ctx.lineTo(-3, 4);
                                        ctx.closePath();
                                        ctx.fillStyle = "#9A8CFF";
                                        ctx.fill();
                                    }
                                }
                            }
                        }
                    }

                    // Effective Angular Sensitivity
                    Row {
                        spacing: 6
                        Text {
                            text: "Effective:"
                            color: "#666"
                            font.pixelSize: 10
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: (Backend.effectiveAngularSensitivity || 0).toFixed(2) + "° / cm"
                            color: "#6EEBFF"
                            font.pixelSize: 11
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    // Safe Zone Clamp Lock
                    Row {
                        spacing: 6
                        Rectangle {
                            width: 18
                            height: 18
                            radius: 4
                            color: Backend.safeZoneClampEnabled ? Qt.rgba(0, 0.8, 0.3, 0.2) : "transparent"
                            border.color: Backend.safeZoneClampEnabled ? "#00C853" : "#666"
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                anchors.centerIn: parent
                                text: Backend.safeZoneClampEnabled ? "✓" : ""
                                color: "#00C853"
                                font.pixelSize: 12
                                font.bold: true
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Backend.safeZoneClampEnabled = !Backend.safeZoneClampEnabled
                            }
                        }
                        Text {
                            text: "Lock Safe Zone"
                            color: "#9AA4B2"
                            font.pixelSize: 10
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    // ========== SIMULATE / APPLY / ROLLBACK BUTTONS ==========
                    Row {
                        spacing: 10

                        // Simulate button
                        Rectangle {
                            width: 85
                            height: 32
                            radius: 8
                            color: simulateBtn.containsMouse ? Qt.rgba(0.6, 0.5, 1, 0.25) : Qt.rgba(0.6, 0.5, 1, 0.12)
                            border.color: "#9A8CFF"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "▶ Simulate"
                                color: "#9A8CFF"
                                font.pixelSize: 10
                                font.bold: true
                            }

                            MouseArea {
                                id: simulateBtn
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Backend.enableSimulateMode(true)
                            }
                        }

                        // Apply button
                        Rectangle {
                            width: 85
                            height: 32
                            radius: 8
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop {
                                    position: 0
                                    color: applyBtn.containsMouse ? "#7FFFFF" : "#6EEBFF"
                                }
                                GradientStop {
                                    position: 1
                                    color: applyBtn.containsMouse ? "#AAAAFF" : "#9A8CFF"
                                }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: "✔ Apply"
                                color: "#0A0C12"
                                font.pixelSize: 10
                                font.bold: true
                            }

                            MouseArea {
                                id: applyBtn
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    Backend.takeSnapshot();
                                    root.applyChanges();
                                }
                            }
                        }

                        // Rollback button
                        Rectangle {
                            width: 85
                            height: 32
                            radius: 8
                            color: rollbackBtn.containsMouse ? Qt.rgba(1, 0.4, 0.4, 0.25) : Qt.rgba(1, 0.4, 0.4, 0.12)
                            border.color: "#FF6B6B"
                            border.width: 1
                            opacity: Backend.hasSnapshot ? 1.0 : 0.4

                            Text {
                                anchors.centerIn: parent
                                text: "↩ Rollback"
                                color: "#FF6B6B"
                                font.pixelSize: 10
                                font.bold: true
                            }

                            MouseArea {
                                id: rollbackBtn
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                enabled: Backend.hasSnapshot
                                onClicked: Backend.rollback()
                            }
                        }
                    }
                }
            }
        }
    }

    // Open animation
    ParallelAnimation {
        id: openAnim
        NumberAnimation {
            target: backdrop
            property: "opacity"
            from: 0
            to: 1
            duration: 280
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: mainPanel
            property: "opacity"
            from: 0
            to: 1
            duration: 280
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: mainPanel
            property: "scale"
            from: 0.92
            to: 1
            duration: 280
            easing.type: Easing.OutCubic
        }
    }

    // Close animation
    ParallelAnimation {
        id: closeAnim
        NumberAnimation {
            target: backdrop
            property: "opacity"
            to: 0
            duration: 220
            easing.type: Easing.InCubic
        }
        NumberAnimation {
            target: mainPanel
            property: "opacity"
            to: 0
            duration: 220
            easing.type: Easing.InCubic
        }
        NumberAnimation {
            target: mainPanel
            property: "scale"
            to: 0.95
            duration: 220
            easing.type: Easing.InCubic
        }
        onFinished: {
            root.visible = false;
            root.isOpen = false;
        }
    }

    Keys.onEscapePressed: tryClose()

    // ========== DISCARD CONFIRMATION POPUP ==========
    Rectangle {
        id: discardPopup
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.7)
        visible: false
        z: 100

        Rectangle {
            anchors.centerIn: parent
            width: 420
            height: 200
            radius: 20
            color: Qt.rgba(0.06, 0.07, 0.1, 0.98)
            border.color: "#FFAB40"
            border.width: 2

            // Glass shine
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 4
                height: 1
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop {
                        position: 0.0
                        color: "transparent"
                    }
                    GradientStop {
                        position: 0.5
                        color: Qt.rgba(1, 1, 1, 0.2)
                    }
                    GradientStop {
                        position: 1.0
                        color: "transparent"
                    }
                }
            }

            Column {
                anchors.centerIn: parent
                spacing: 24

                // Warning icon
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "⚠️"
                    font.pixelSize: 32
                }

                // Title
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Unsaved Changes"
                    color: "#FFAB40"
                    font.pixelSize: 18
                    font.bold: true
                }

                // Message
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Do you want to discard your changes?"
                    color: "#9AA4B2"
                    font.pixelSize: 13
                }

                // Buttons
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 16

                    // Discard button
                    Rectangle {
                        width: 130
                        height: 40
                        radius: 10
                        color: discardBtn.containsMouse ? Qt.rgba(1, 0.4, 0.4, 0.25) : Qt.rgba(1, 0.4, 0.4, 0.15)
                        border.color: "#FF6B6B"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "Discard"
                            color: "#FF6B6B"
                            font.pixelSize: 13
                            font.bold: true
                        }

                        MouseArea {
                            id: discardBtn
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                discardChanges();
                                discardPopup.visible = false;
                                close();
                            }
                        }
                    }

                    // Apply & Close button
                    Rectangle {
                        width: 130
                        height: 40
                        radius: 10
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop {
                                position: 0
                                color: applyCloseBtn.containsMouse ? "#7FFFFF" : "#6EEBFF"
                            }
                            GradientStop {
                                position: 1
                                color: applyCloseBtn.containsMouse ? "#AAAAFF" : "#9A8CFF"
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "Apply & Close"
                            color: "#0A0C12"
                            font.pixelSize: 13
                            font.bold: true
                        }

                        MouseArea {
                            id: applyCloseBtn
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                applyChanges();
                                discardPopup.visible = false;
                                close();
                            }
                        }
                    }
                }
            }
        }
    }
}
