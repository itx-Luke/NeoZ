import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NeoZ
import "../components"

RowLayout {
    id: root
    spacing: 0

    property int selectedCurve: 0
    property var curves: ["FF_OneTap_v2", "Linear", "LowSpeedPrecision", "AggressiveFlick"]

    // Track if any value has changed from saved state
    property bool hasChanges: {
        return Math.abs(xSlider.value - Backend.xMultiplier) > 0.001 || Math.abs(ySlider.value - Backend.yMultiplier) > 0.001 || Math.abs(slowZoneSlider.value - Backend.slowZone) > 0.5 || Math.abs(smoothingSlider.value - Backend.smoothing) > 0.5 || curves[selectedCurve] !== Backend.curve || dpiInput.text !== Backend.mouseDpi.toString();
    }

    // === LEFT: ACTIVE SENSITIVITY PROFILE ===
    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 15
            anchors.topMargin: 10
            anchors.rightMargin: 10
            spacing: 20

            Text {
                text: "Active Sensitivity Profile"
                color: Style.textPrimary
                font.pixelSize: 18
                font.bold: true
            }

            // Profile Selector Row
            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                glassOpacity: 0.3

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 25

                    Text {
                        text: "Active Profile"
                        color: Style.textSecondary
                        font.pixelSize: 13
                        Layout.alignment: Qt.AlignVCenter
                    }

                    GlassComboBox {
                        model: ["Free Fire — Bluestacks — 1080p", "Free Fire — MSI — 720p"]
                        Layout.preferredWidth: 300
                        Layout.alignment: Qt.AlignVCenter
                    }

                    RowLayout {
                        spacing: 20
                        Layout.alignment: Qt.AlignVCenter

                        // New Profile Button
                        Column {
                            spacing: 4
                            Layout.alignment: Qt.AlignVCenter
                            NeonButton {
                                text: "+"
                                accentColor: Style.primary
                                implicitWidth: 40
                                implicitHeight: 40
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "New"
                                color: Style.textSecondary
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        // Duplicate Button
                        Column {
                            spacing: 4
                            Layout.alignment: Qt.AlignVCenter
                            NeonButton {
                                text: "⎌"
                                accentColor: Style.textSecondary
                                implicitWidth: 40
                                implicitHeight: 40
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "Duplicate"
                                color: Style.textSecondary
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        // Delete Button
                        Column {
                            spacing: 4
                            Layout.alignment: Qt.AlignVCenter
                            NeonButton {
                                text: "🗑"
                                accentColor: Style.danger
                                implicitWidth: 40
                                implicitHeight: 40
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "Delete"
                                color: Style.textSecondary
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }
            }

            // Base Input
            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                glassOpacity: 0.2

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Column {
                        spacing: 5
                        Text {
                            text: "Base Input"
                            color: Style.textPrimary
                            font.bold: true
                        }
                        Text {
                            text: "Mouse DPI"
                            color: Style.textSecondary
                            font.pixelSize: 11
                        }
                        Text {
                            text: "Use your actual hardware DPI"
                            color: Style.textSecondary
                            font.pixelSize: 10
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        width: 100
                        height: 36
                        radius: 6
                        color: "transparent"
                        border.color: Style.primary
                        border.width: 1

                        gradient: Gradient {
                            orientation: Gradient.Vertical
                            GradientStop {
                                position: 0.0
                                color: Qt.alpha(Style.primary, 0.2)
                            }
                            GradientStop {
                                position: 1.0
                                color: Qt.alpha(Style.primary, 0.05)
                            }
                        }

                        TextInput {
                            id: dpiInput
                            anchors.centerIn: parent
                            text: Backend.mouseDpi.toString()
                            color: Style.primary
                            font.pixelSize: 16
                            font.bold: true
                            horizontalAlignment: TextInput.AlignHCenter
                            inputMethodHints: Qt.ImhDigitsOnly
                            validator: IntValidator {
                                bottom: 100
                                top: 16000
                            }
                            selectByMouse: true

                            onEditingFinished: {
                                // Only validate - don't save yet (Apply button does that)
                                var newDpi = parseInt(text);
                                if (isNaN(newDpi) || newDpi < 100 || newDpi > 16000) {
                                    text = Backend.mouseDpi.toString();
                                }
                            }
                        }

                        // DPI step buttons
                        Row {
                            anchors.right: parent.right
                            anchors.rightMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "−"
                                color: Style.textSecondary
                                font.pixelSize: 14
                                font.bold: true

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        var currentDpi = parseInt(dpiInput.text) || Backend.mouseDpi;
                                        var newDpi = currentDpi - 100;
                                        if (newDpi >= 100)
                                            dpiInput.text = newDpi.toString();
                                    }
                                }
                            }

                            Text {
                                text: "+"
                                color: Style.textSecondary
                                font.pixelSize: 14
                                font.bold: true

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        var currentDpi = parseInt(dpiInput.text) || Backend.mouseDpi;
                                        var newDpi = currentDpi + 100;
                                        if (newDpi <= 16000)
                                            dpiInput.text = newDpi.toString();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Multipliers Section
            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                glassOpacity: 0.2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Text {
                        text: "Multipliers"
                        color: Style.textPrimary
                        font.bold: true
                    }

                    // X Multiplier
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        spacing: 20

                        Column {
                            Layout.preferredWidth: 120
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 4
                            Text {
                                text: "X Multiplier"
                                color: Style.textSecondary
                                font.pixelSize: 13
                            }
                            Text {
                                text: "0.10–4.00"
                                color: Style.textSecondary
                                font.pixelSize: 10
                            }
                        }

                        Slider {
                            id: xSlider
                            Layout.fillWidth: true
                            from: 0.10
                            to: 4.00
                            value: Backend.xMultiplier
                            stepSize: 0.01
                        }

                        Rectangle {
                            width: 60
                            height: 30
                            radius: 4
                            color: Qt.rgba(1, 1, 1, 0.1)
                            border.color: Style.surfaceHighlight
                            border.width: 1
                            Text {
                                anchors.centerIn: parent
                                text: xSlider.value.toFixed(2)
                                color: Style.textPrimary
                                font.bold: true
                            }
                        }
                    }

                    // Y Multiplier
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        spacing: 20

                        Column {
                            Layout.preferredWidth: 120
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 4
                            Text {
                                text: "Y Multiplier"
                                color: Style.textSecondary
                                font.pixelSize: 13
                            }
                            Text {
                                text: "0.10–4.00"
                                color: Style.textSecondary
                                font.pixelSize: 10
                            }
                        }

                        Slider {
                            id: ySlider
                            Layout.fillWidth: true
                            from: 0.10
                            to: 4.00
                            value: Backend.yMultiplier
                            stepSize: 0.01
                        }

                        Rectangle {
                            width: 60
                            height: 30
                            radius: 4
                            color: Qt.rgba(1, 1, 1, 0.1)
                            border.color: Style.surfaceHighlight
                            border.width: 1
                            Text {
                                anchors.centerIn: parent
                                text: ySlider.value.toFixed(2)
                                color: Style.textPrimary
                                font.bold: true
                            }
                        }
                    }
                }
            }

            // Aim Assist Behavior - Outer container with electric border
            Item {
                id: aimAssistWrapper
                Layout.fillWidth: true
                Layout.preferredHeight: 210

                property bool adbConnected: Backend.adbStatus === "Connected"
                property bool aimAssistActive: Backend.aimAssistActive
                property real electricPhase: 0

                // Electric phase animation - circulates when connected
                NumberAnimation on electricPhase {
                    running: aimAssistWrapper.adbConnected
                    from: 0
                    to: 1
                    duration: 1500
                    loops: Animation.Infinite
                }

                // === CIRCULATING ELECTRIC BORDER (on outer container) ===
                Canvas {
                    id: electricBorderOuter
                    anchors.fill: parent
                    visible: aimAssistWrapper.adbConnected
                    z: 10

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);

                        var phase = aimAssistWrapper.electricPhase;
                        var r = 18;  // Outer radius
                        var lineWidth = 3;
                        var glowColor = aimAssistWrapper.aimAssistActive ? "#FF4444" : "#00E5FF";

                        var dashLength = 50;
                        var gapLength = 70;
                        var offset = phase * (dashLength + gapLength);

                        ctx.save();
                        ctx.strokeStyle = glowColor;
                        ctx.lineWidth = lineWidth;
                        ctx.lineCap = "round";
                        ctx.setLineDash([dashLength, gapLength]);
                        ctx.lineDashOffset = -offset;
                        ctx.shadowColor = glowColor;
                        ctx.shadowBlur = 12;

                        ctx.beginPath();
                        ctx.moveTo(r + 2, 2);
                        ctx.lineTo(width - r - 2, 2);
                        ctx.arcTo(width - 2, 2, width - 2, r + 2, r);
                        ctx.lineTo(width - 2, height - r - 2);
                        ctx.arcTo(width - 2, height - 2, width - r - 2, height - 2, r);
                        ctx.lineTo(r + 2, height - 2);
                        ctx.arcTo(2, height - 2, 2, height - r - 2, r);
                        ctx.lineTo(2, r + 2);
                        ctx.arcTo(2, 2, r + 2, 2, r);
                        ctx.closePath();
                        ctx.stroke();
                        ctx.restore();
                    }

                    Connections {
                        target: aimAssistWrapper
                        function onElectricPhaseChanged() {
                            electricBorderOuter.requestPaint();
                        }
                        function onAimAssistActiveChanged() {
                            electricBorderOuter.requestPaint();
                        }
                    }
                }

                // === INNER GLOW (on outer container edges) ===
                Canvas {
                    id: innerGlowOuter
                    anchors.fill: parent
                    anchors.margins: 4
                    visible: aimAssistWrapper.adbConnected
                    opacity: 0.6
                    z: 5

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);
                        var glowColor = aimAssistWrapper.aimAssistActive ? "rgba(255,68,68," : "rgba(0,229,255,";
                        var d = 25;

                        var t = ctx.createLinearGradient(0, 0, 0, d);
                        t.addColorStop(0, glowColor + "0.4)");
                        t.addColorStop(1, glowColor + "0)");
                        ctx.fillStyle = t;
                        ctx.fillRect(0, 0, width, d);

                        var b = ctx.createLinearGradient(0, height, 0, height - d);
                        b.addColorStop(0, glowColor + "0.4)");
                        b.addColorStop(1, glowColor + "0)");
                        ctx.fillStyle = b;
                        ctx.fillRect(0, height - d, width, d);

                        var l = ctx.createLinearGradient(0, 0, d, 0);
                        l.addColorStop(0, glowColor + "0.4)");
                        l.addColorStop(1, glowColor + "0)");
                        ctx.fillStyle = l;
                        ctx.fillRect(0, 0, d, height);

                        var rg = ctx.createLinearGradient(width, 0, width - d, 0);
                        rg.addColorStop(0, glowColor + "0.4)");
                        rg.addColorStop(1, glowColor + "0)");
                        ctx.fillStyle = rg;
                        ctx.fillRect(width - d, 0, d, height);
                    }

                    Connections {
                        target: aimAssistWrapper
                        function onAdbConnectedChanged() {
                            innerGlowOuter.requestPaint();
                        }
                        function onAimAssistActiveChanged() {
                            innerGlowOuter.requestPaint();
                        }
                    }
                }

                // === INNER GLASS PANEL ===
                GlassPanel {
                    id: aimAssistContainer
                    anchors.fill: parent
                    anchors.margins: 5
                    radius: 13  // Inner radius = 18 - 5 margin
                    glassOpacity: 0.2
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 12
                        enabled: aimAssistWrapper.adbConnected
                        opacity: aimAssistWrapper.adbConnected ? 1.0 : 0.4

                        RowLayout {
                            Layout.fillWidth: true

                            Text {
                                text: "Aim Assist Behavior"
                                color: Style.textPrimary
                                font.bold: true
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            // Status indicator
                            Row {
                                spacing: 6
                                visible: aimAssistWrapper.adbConnected

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: aimAssistWrapper.aimAssistActive ? "#FF4444" : "#00E5FF"

                                    SequentialAnimation on opacity {
                                        running: aimAssistWrapper.aimAssistActive
                                        loops: Animation.Infinite
                                        NumberAnimation {
                                            to: 0.3
                                            duration: 500
                                        }
                                        NumberAnimation {
                                            to: 1.0
                                            duration: 500
                                        }
                                    }
                                }

                                Text {
                                    text: aimAssistWrapper.aimAssistActive ? "ACTIVE" : "READY"
                                    color: aimAssistWrapper.aimAssistActive ? "#FF4444" : "#00E5FF"
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }
                        }

                        // Slow-zone %
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 50
                            spacing: 20

                            Column {
                                Layout.preferredWidth: 120
                                Layout.alignment: Qt.AlignVCenter
                                spacing: 4
                                Text {
                                    text: "Slow-zone %"
                                    color: Style.textSecondary
                                    font.pixelSize: 13
                                }
                                Text {
                                    text: "0–50"
                                    color: Style.textSecondary
                                    font.pixelSize: 10
                                }
                            }

                            Slider {
                                id: slowZoneSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 50
                                value: Backend.slowZone
                                stepSize: 1
                            }

                            Rectangle {
                                width: 60
                                height: 30
                                radius: 4
                                color: Qt.rgba(1, 1, 1, 0.1)
                                border.color: Style.surfaceHighlight
                                border.width: 1
                                Text {
                                    anchors.centerIn: parent
                                    text: slowZoneSlider.value.toFixed(0) + " %"
                                    color: Style.textPrimary
                                    font.bold: true
                                }
                            }
                        }

                        // Smoothing
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 50
                            spacing: 20

                            Column {
                                Layout.preferredWidth: 120
                                Layout.alignment: Qt.AlignVCenter
                                spacing: 4
                                Text {
                                    text: "Smoothing (ms)"
                                    color: Style.textSecondary
                                    font.pixelSize: 13
                                }
                                Text {
                                    text: "0–200"
                                    color: Style.textSecondary
                                    font.pixelSize: 10
                                }
                            }

                            Slider {
                                id: smoothingSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 200
                                value: Backend.smoothing
                                stepSize: 5
                            }

                            Rectangle {
                                width: 60
                                height: 30
                                radius: 4
                                color: Qt.rgba(1, 1, 1, 0.1)
                                border.color: Style.surfaceHighlight
                                border.width: 1
                                Text {
                                    anchors.centerIn: parent
                                    text: smoothingSlider.value.toFixed(0) + " ms"
                                    color: Style.textPrimary
                                    font.bold: true
                                }
                            }
                        }
                    }
                } // Close GlassPanel

                // === CURTAIN VEIL (slides up when unlocked) ===
                Rectangle {
                    id: curtainVeil
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: aimAssistWrapper.adbConnected ? 0 : parent.height
                    radius: 18  // Match outer radius
                    clip: true
                    z: 20

                    Behavior on height {
                        NumberAnimation {
                            duration: 700
                            easing.type: Easing.OutCubic
                        }
                    }

                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: "#1A1A1A"
                        }
                        GradientStop {
                            position: 0.4
                            color: "#0A0A0A"
                        }
                        GradientStop {
                            position: 1.0
                            color: "#000000"
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 12
                        visible: curtainVeil.height > 80
                        opacity: Math.min(1, curtainVeil.height / 150)

                        Text {
                            text: "🔒"
                            font.pixelSize: 32
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "Connect ADB to unlock"
                            color: "#666666"
                            font.pixelSize: 13
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Rectangle {
                            width: 80
                            height: 2
                            radius: 1
                            anchors.horizontalCenter: parent.horizontalCenter
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop {
                                    position: 0.0
                                    color: "transparent"
                                }
                                GradientStop {
                                    position: 0.5
                                    color: "#333333"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: "transparent"
                                }
                            }
                        }
                    }
                }
            } // Close aimAssistWrapper

            // Change Summary
            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                glassOpacity: 0.2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    Text {
                        text: "Change Summary"
                        color: Style.textPrimary
                        font.bold: true
                    }

                    // Header
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: ""
                            Layout.preferredWidth: 120
                        }
                        Text {
                            text: "Current"
                            color: Style.textSecondary
                            font.pixelSize: 11
                            Layout.preferredWidth: 80
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text {
                            text: "Proposed"
                            color: Style.textSecondary
                            font.pixelSize: 11
                            Layout.preferredWidth: 80
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Style.surfaceHighlight
                    }

                    // Rows
                    Repeater {
                        model: [
                            {
                                label: "DPI",
                                current: "800",
                                proposed: "800"
                            },
                            {
                                label: "X Multiplier",
                                current: "1.12",
                                proposed: xSlider.value.toFixed(2)
                            },
                            {
                                label: "Y Multiplier",
                                current: "1.15",
                                proposed: ySlider.value.toFixed(2)
                            },
                            {
                                label: "cm/360 (est)",
                                current: "31.8 cm",
                                proposed: "29.5 cm"
                            }
                        ]
                        delegate: RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: modelData.label
                                color: Style.textSecondary
                                font.pixelSize: 12
                                Layout.preferredWidth: 120
                            }
                            Text {
                                text: modelData.current
                                color: Style.textPrimary
                                font.pixelSize: 12
                                Layout.preferredWidth: 80
                                horizontalAlignment: Text.AlignHCenter
                            }
                            Row {
                                Layout.preferredWidth: 80
                                spacing: 5
                                Text {
                                    text: modelData.proposed
                                    color: Style.primary
                                    font.bold: true
                                    font.pixelSize: 12
                                }
                                Text {
                                    text: "→"
                                    color: Style.textSecondary
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }

                    Text {
                        text: "Δ cm/360: -7.3% (faster)"
                        color: Style.success
                        font.pixelSize: 11
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }

    // === RIGHT: CURVE & VISUALIZATION ===
    GlassPanel {
        Layout.preferredWidth: 420
        Layout.fillHeight: true
        Layout.margins: 20
        Layout.leftMargin: 10
        glassOpacity: 0.3

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Text {
                text: "Curve & Visualization"
                color: Style.textPrimary
                font.pixelSize: 18
                font.bold: true
            }

            // Curve tabs
            Column {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: "Response Curve"
                    color: Style.textSecondary
                    font.pixelSize: 12
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: root.curves
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            height: 32
                            radius: 6
                            color: root.selectedCurve === index ? Style.primary : Qt.rgba(1, 1, 1, 0.1)
                            border.color: root.selectedCurve === index ? Style.primary : Style.surfaceHighlight
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                color: root.selectedCurve === index ? Style.background : Style.textSecondary
                                font.pixelSize: 10
                                font.bold: root.selectedCurve === index
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.selectedCurve = index
                            }
                        }
                    }
                }
            }

            // Curve Graph
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                color: Qt.rgba(0, 0, 0, 0.3)
                radius: 8
                border.color: Style.surfaceHighlight
                border.width: 1

                Canvas {
                    anchors.fill: parent
                    anchors.margins: 20

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);

                        // Grid
                        ctx.strokeStyle = Qt.rgba(1, 1, 1, 0.1);
                        ctx.lineWidth = 1;
                        for (var i = 0; i <= 4; i++) {
                            ctx.beginPath();
                            ctx.moveTo(0, height * i / 4);
                            ctx.lineTo(width, height * i / 4);
                            ctx.stroke();
                            ctx.beginPath();
                            ctx.moveTo(width * i / 4, 0);
                            ctx.lineTo(width * i / 4, height);
                            ctx.stroke();
                        }

                        // Curve
                        ctx.strokeStyle = "#00E5FF";
                        ctx.lineWidth = 3;
                        ctx.beginPath();
                        ctx.moveTo(0, height);
                        ctx.bezierCurveTo(width * 0.3, height * 0.8, width * 0.5, height * 0.2, width, 0);
                        ctx.stroke();
                    }
                }

                // Axis labels
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Output\nscaling"
                    color: Style.textSecondary
                    font.pixelSize: 9
                    rotation: -90
                }

                Text {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 5
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Input speed"
                    color: Style.textSecondary
                    font.pixelSize: 10
                }
            }

            // Aim Zone Logic
            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                glassOpacity: 0.3

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    anchors.topMargin: 12
                    anchors.leftMargin: 18
                    spacing: 10

                    Text {
                        text: "Aim Zone Logic"
                        color: Style.textSecondary
                        font.pixelSize: 13
                        font.bold: true
                        Layout.alignment: Qt.AlignLeft
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 50

                        Item {
                            Layout.fillWidth: true
                        }

                        // One-tap Zone Orb
                        Column {
                            spacing: 12
                            Layout.alignment: Qt.AlignVCenter

                            property bool active: Backend.drcsEnabled

                            Rectangle {
                                id: oneTapOrb
                                width: 40
                                height: 40
                                radius: 20
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "transparent"

                                // Pointer properties for glow tracking
                                property real pointerX: width / 2
                                property real pointerY: height / 2
                                property real pointerD: oneTapOrb.parent.active ? 0.8 : 0.3

                                // Canvas-based radial glow
                                Canvas {
                                    id: oneTapGlowCanvas
                                    anchors.centerIn: parent
                                    width: 60
                                    height: 60

                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.clearRect(0, 0, width, height);

                                        var intensity = oneTapOrb.pointerD * 0.9;
                                        var centerX = oneTapOrb.pointerX + (width - oneTapOrb.width) / 2;
                                        var centerY = oneTapOrb.pointerY + (height - oneTapOrb.height) / 2;

                                        // Radial glow from pointer position
                                        var gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, width / 2);
                                        if (oneTapOrb.parent.active) {
                                            gradient.addColorStop(0, "hsla(187, 100%, 50%, " + (0.7 * intensity) + ")");
                                            gradient.addColorStop(0.3, "hsla(187, 100%, 45%, " + (0.4 * intensity) + ")");
                                            gradient.addColorStop(0.6, "hsla(187, 100%, 40%, " + (0.15 * intensity) + ")");
                                            gradient.addColorStop(1, "rgba(0, 0, 0, 0)");
                                        } else {
                                            gradient.addColorStop(0, "hsla(0, 0%, 60%, " + (0.3 * intensity) + ")");
                                            gradient.addColorStop(0.5, "hsla(0, 0%, 50%, " + (0.1 * intensity) + ")");
                                            gradient.addColorStop(1, "rgba(0, 0, 0, 0)");
                                        }
                                        ctx.fillStyle = gradient;
                                        ctx.beginPath();
                                        // When grey/inactive, draw glow at pointer position
                                        if (oneTapOrb.parent.active) {
                                            ctx.arc(width / 2, height / 2, width / 2, 0, Math.PI * 2);
                                        } else {
                                            ctx.arc(centerX, centerY, width / 2.5, 0, Math.PI * 2);
                                        }
                                        ctx.fill();
                                    }

                                    Connections {
                                        target: breatheAnim1
                                        function onProgressChanged() {
                                            if (!oneTapOrb.parent.active) {
                                                oneTapOrb.pointerD = 0.3 + Math.sin(breatheAnim1.progress * Math.PI * 2) * 0.15;
                                                oneTapGlowCanvas.requestPaint();
                                            }
                                        }
                                    }
                                }

                                // Click area for toggling (no mouse tracking)
                                MouseArea {
                                    id: oneTapGlowArea
                                    anchors.fill: parent

                                    onClicked: function (mouse) {
                                        Backend.drcsEnabled = !Backend.drcsEnabled;
                                        oneTapGlowCanvas.requestPaint();
                                    }
                                }

                                // Shadow underneath orb
                                Rectangle {
                                    anchors.centerIn: parent
                                    anchors.verticalCenterOffset: 4
                                    width: 32
                                    height: 10
                                    radius: 5
                                    color: Qt.rgba(0, 0, 0, 0.4)
                                    opacity: oneTapOrb.parent.active ? 0.6 : 0.3

                                    Behavior on opacity {
                                        NumberAnimation {
                                            duration: 300
                                        }
                                    }
                                }

                                // Main orb with gradient
                                Rectangle {
                                    id: oneTapMainOrb
                                    anchors.centerIn: parent
                                    width: 36
                                    height: 36
                                    radius: 18

                                    gradient: Gradient {
                                        GradientStop {
                                            position: 0.0
                                            color: oneTapOrb.parent.active ? "#00E5FF" : "#555555"
                                        }
                                        GradientStop {
                                            position: 0.4
                                            color: oneTapOrb.parent.active ? "#00B8D4" : "#444444"
                                        }
                                        GradientStop {
                                            position: 0.7
                                            color: oneTapOrb.parent.active ? "#0088AA" : "#333333"
                                        }
                                        GradientStop {
                                            position: 1.0
                                            color: oneTapOrb.parent.active ? "#005566" : "#222222"
                                        }
                                    }

                                    // Inner highlight for 3D effect
                                    Rectangle {
                                        width: 22
                                        height: 14
                                        radius: 7
                                        x: 8
                                        y: 6
                                        color: oneTapOrb.parent.active ? Qt.rgba(1, 1, 1, 0.6) : Qt.rgba(1, 1, 1, 0.12)
                                        rotation: -25

                                        Behavior on color {
                                            ColorAnimation {
                                                duration: 400
                                            }
                                        }
                                    }

                                    // Secondary highlight
                                    Rectangle {
                                        width: 8
                                        height: 6
                                        radius: 3
                                        x: 32
                                        y: 12
                                        color: oneTapOrb.parent.active ? Qt.rgba(1, 1, 1, 0.3) : Qt.rgba(1, 1, 1, 0.06)
                                        rotation: -15

                                        Behavior on color {
                                            ColorAnimation {
                                                duration: 400
                                            }
                                        }
                                    }

                                    // Breathing scale animation
                                    transform: Scale {
                                        origin.x: 26
                                        origin.y: 26
                                        xScale: oneTapOrb.parent.active ? 1.0 : 0.96 + Math.sin(breatheAnim1.progress * Math.PI * 2) * 0.04
                                        yScale: oneTapOrb.parent.active ? 1.0 : 0.96 + Math.sin(breatheAnim1.progress * Math.PI * 2) * 0.04
                                    }
                                }

                                // Outer ring glow
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: oneTapOrb.parent.active ? 42 : 38
                                    height: width
                                    radius: width / 2
                                    color: "transparent"
                                    border.color: oneTapOrb.parent.active ? Qt.rgba(0, 0.9, 1, 0.6) : Qt.rgba(1, 1, 1, 0.15)
                                    border.width: oneTapOrb.parent.active ? 2 : 1

                                    Behavior on width {
                                        NumberAnimation {
                                            duration: 300
                                        }
                                    }
                                    Behavior on border.color {
                                        ColorAnimation {
                                            duration: 400
                                        }
                                    }
                                    Behavior on border.width {
                                        NumberAnimation {
                                            duration: 300
                                        }
                                    }
                                }

                                NumberAnimation {
                                    id: breatheAnim1
                                    property real progress: 0
                                    target: breatheAnim1
                                    property: "progress"
                                    from: 0
                                    to: 1
                                    duration: 3000
                                    loops: Animation.Infinite
                                    running: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: parent.parent.active = !parent.parent.active
                                }
                            }

                            Text {
                                text: "One-tap Zone"
                                color: parent.active ? Style.primary : Style.textSecondary
                                font.pixelSize: 11
                                font.bold: parent.active
                                anchors.horizontalCenter: parent.horizontalCenter

                                Behavior on color {
                                    ColorAnimation {
                                        duration: 300
                                    }
                                }
                            }
                        }

                        // Red-Zone Assist Orb
                        Column {
                            spacing: 12
                            Layout.alignment: Qt.AlignVCenter

                            property bool active: false

                            Rectangle {
                                id: redZoneOrb
                                width: 40
                                height: 40
                                radius: 20
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "transparent"

                                // Pointer properties for glow tracking
                                property real pointerX: width / 2
                                property real pointerY: height / 2
                                property real pointerD: redZoneOrb.parent.active ? 0.8 : 0.3

                                // Canvas-based radial glow
                                Canvas {
                                    id: redZoneGlowCanvas
                                    anchors.centerIn: parent
                                    width: 60
                                    height: 60

                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.clearRect(0, 0, width, height);

                                        var intensity = redZoneOrb.pointerD * 0.9;
                                        var centerX = redZoneOrb.pointerX + (width - redZoneOrb.width) / 2;
                                        var centerY = redZoneOrb.pointerY + (height - redZoneOrb.height) / 2;

                                        // Radial glow from pointer position
                                        var gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, width / 2);
                                        if (redZoneOrb.parent.active) {
                                            gradient.addColorStop(0, "hsla(340, 100%, 62%, " + (0.7 * intensity) + ")");
                                            gradient.addColorStop(0.3, "hsla(340, 100%, 52%, " + (0.4 * intensity) + ")");
                                            gradient.addColorStop(0.6, "hsla(340, 100%, 42%, " + (0.15 * intensity) + ")");
                                            gradient.addColorStop(1, "rgba(0, 0, 0, 0)");
                                        } else {
                                            gradient.addColorStop(0, "hsla(0, 0%, 60%, " + (0.3 * intensity) + ")");
                                            gradient.addColorStop(0.5, "hsla(0, 0%, 50%, " + (0.1 * intensity) + ")");
                                            gradient.addColorStop(1, "rgba(0, 0, 0, 0)");
                                        }
                                        ctx.fillStyle = gradient;
                                        ctx.beginPath();
                                        // When grey/inactive, draw glow at pointer position
                                        if (redZoneOrb.parent.active) {
                                            ctx.arc(width / 2, height / 2, width / 2, 0, Math.PI * 2);
                                        } else {
                                            ctx.arc(centerX, centerY, width / 2.5, 0, Math.PI * 2);
                                        }
                                        ctx.fill();
                                    }

                                    Connections {
                                        target: breatheAnim2
                                        function onProgressChanged() {
                                            if (!redZoneOrb.parent.active) {
                                                redZoneOrb.pointerD = 0.3 + Math.sin(breatheAnim2.progress * Math.PI * 2) * 0.15;
                                                redZoneGlowCanvas.requestPaint();
                                            }
                                        }
                                    }
                                }

                                // Click area for toggling (no mouse tracking)
                                MouseArea {
                                    id: redZoneGlowArea
                                    anchors.fill: parent

                                    onClicked: function (mouse) {
                                        redZoneOrb.parent.active = !redZoneOrb.parent.active;
                                        redZoneGlowCanvas.requestPaint();
                                    }
                                }

                                // Shadow underneath orb
                                Rectangle {
                                    anchors.centerIn: parent
                                    anchors.verticalCenterOffset: 6
                                    width: 48
                                    height: 16
                                    radius: 8
                                    color: Qt.rgba(0, 0, 0, 0.4)
                                    opacity: redZoneOrb.parent.active ? 0.6 : 0.3

                                    Behavior on opacity {
                                        NumberAnimation {
                                            duration: 300
                                        }
                                    }
                                }

                                // Main orb with gradient
                                Rectangle {
                                    id: redZoneMainOrb
                                    anchors.centerIn: parent
                                    width: 36
                                    height: 36
                                    radius: 18

                                    gradient: Gradient {
                                        GradientStop {
                                            position: 0.0
                                            color: redZoneOrb.parent.active ? "#FF4081" : "#555555"
                                        }
                                        GradientStop {
                                            position: 0.4
                                            color: redZoneOrb.parent.active ? "#E91E63" : "#444444"
                                        }
                                        GradientStop {
                                            position: 0.7
                                            color: redZoneOrb.parent.active ? "#C2185B" : "#333333"
                                        }
                                        GradientStop {
                                            position: 1.0
                                            color: redZoneOrb.parent.active ? "#880E4F" : "#222222"
                                        }
                                    }

                                    // Inner highlight for 3D effect
                                    Rectangle {
                                        width: 22
                                        height: 14
                                        radius: 7
                                        x: 8
                                        y: 6
                                        color: redZoneOrb.parent.active ? Qt.rgba(1, 1, 1, 0.6) : Qt.rgba(1, 1, 1, 0.12)
                                        rotation: -25

                                        Behavior on color {
                                            ColorAnimation {
                                                duration: 400
                                            }
                                        }
                                    }

                                    // Secondary highlight
                                    Rectangle {
                                        width: 8
                                        height: 6
                                        radius: 3
                                        x: 32
                                        y: 12
                                        color: redZoneOrb.parent.active ? Qt.rgba(1, 1, 1, 0.3) : Qt.rgba(1, 1, 1, 0.06)
                                        rotation: -15

                                        Behavior on color {
                                            ColorAnimation {
                                                duration: 400
                                            }
                                        }
                                    }

                                    // Breathing scale animation
                                    transform: Scale {
                                        origin.x: 26
                                        origin.y: 26
                                        xScale: redZoneOrb.parent.active ? 1.0 : 0.96 + Math.sin(breatheAnim2.progress * Math.PI * 2) * 0.04
                                        yScale: redZoneOrb.parent.active ? 1.0 : 0.96 + Math.sin(breatheAnim2.progress * Math.PI * 2) * 0.04
                                    }
                                }

                                // Outer ring glow
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: redZoneOrb.parent.active ? 42 : 38
                                    height: width
                                    radius: width / 2
                                    color: "transparent"
                                    border.color: redZoneOrb.parent.active ? Qt.rgba(1, 0.25, 0.5, 0.6) : Qt.rgba(1, 1, 1, 0.15)
                                    border.width: redZoneOrb.parent.active ? 2 : 1

                                    Behavior on width {
                                        NumberAnimation {
                                            duration: 300
                                        }
                                    }
                                    Behavior on border.color {
                                        ColorAnimation {
                                            duration: 400
                                        }
                                    }
                                    Behavior on border.width {
                                        NumberAnimation {
                                            duration: 300
                                        }
                                    }
                                }

                                NumberAnimation {
                                    id: breatheAnim2
                                    property real progress: 0
                                    target: breatheAnim2
                                    property: "progress"
                                    from: 0
                                    to: 1
                                    duration: 3000
                                    loops: Animation.Infinite
                                    running: true
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: parent.parent.active = !parent.parent.active
                                }
                            }

                            Text {
                                text: "Red-Zone"
                                color: parent.active ? Style.danger : Style.textSecondary
                                font.pixelSize: 11
                                font.bold: parent.active
                                anchors.horizontalCenter: parent.horizontalCenter

                                Behavior on color {
                                    ColorAnimation {
                                        duration: 300
                                    }
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            // Apply buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 15

                NeonButton {
                    id: applyButton
                    text: "✓ Apply Changes"
                    accentColor: Style.success
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    glow: root.hasChanges  // Only glow when there are changes
                    opacity: root.hasChanges ? 1.0 : 0.6
                    onClicked: {
                        Backend.setSensitivity(xSlider.value, ySlider.value, root.curves[root.selectedCurve], slowZoneSlider.value, smoothingSlider.value);
                        // Also apply DPI change
                        var newDpi = parseInt(dpiInput.text);
                        if (!isNaN(newDpi) && newDpi >= 100 && newDpi <= 16000) {
                            Backend.mouseDpi = newDpi;
                        }
                    }

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 200
                        }
                    }
                }

                NeonButton {
                    text: "Revert Unsaved"
                    accentColor: Style.textSecondary
                    Layout.preferredWidth: 130
                    Layout.preferredHeight: 44
                }
            }

            // Log checkbox
            RowLayout {
                Layout.fillWidth: true
                CheckBox {
                    text: "Log this as a new version in history (always on)"
                    checked: true
                }
            }
        }
    }
}
