import QtQuick
import QtQuick.Layouts
import "../style"

// Circular Health Ring with animated progress
Rectangle {
    id: root

    property real value: 85 // 0-100
    property real animatedValue: 0
    property color ringColor: {
        if (value >= 90)
            return CyberpunkColors.matrixGreen;
        if (value >= 70)
            return "#FFFF00";
        if (value >= 50)
            return CyberpunkColors.warningOrange;
        return CyberpunkColors.dangerRed;
    }

    width: 200
    height: 200
    radius: width / 2
    color: "transparent"

    Component.onCompleted: valueAnim.start()

    onValueChanged: valueAnim.start()

    NumberAnimation {
        id: valueAnim
        target: root
        property: "animatedValue"
        to: root.value
        duration: 1000
        easing.type: Easing.OutCubic
    }

    // Background ring
    Canvas {
        id: bgRing
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            ctx.lineWidth = 12;
            ctx.strokeStyle = Qt.rgba(1, 1, 1, 0.1);
            ctx.beginPath();
            ctx.arc(width / 2, height / 2, width / 2 - 15, 0, Math.PI * 2);
            ctx.stroke();
        }
    }

    // Progress ring
    Canvas {
        id: progressRing
        anchors.fill: parent

        property real progress: root.animatedValue / 100

        onProgressChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            ctx.lineWidth = 12;
            ctx.lineCap = "round";

            // Create gradient
            var gradient = ctx.createLinearGradient(0, 0, width, height);
            gradient.addColorStop(0, root.ringColor.toString());
            gradient.addColorStop(1, Qt.lighter(root.ringColor, 1.3).toString());
            ctx.strokeStyle = gradient;

            ctx.beginPath();
            ctx.arc(width / 2, height / 2, width / 2 - 15, -Math.PI / 2, -Math.PI / 2 + (Math.PI * 2 * progress));
            ctx.stroke();
        }
    }

    // Glow effect
    Canvas {
        id: glowRing
        anchors.fill: parent
        opacity: 0.4

        property real progress: root.animatedValue / 100

        onProgressChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            ctx.lineWidth = 20;
            ctx.lineCap = "round";
            ctx.strokeStyle = root.ringColor.toString();
            ctx.shadowColor = root.ringColor.toString();
            ctx.shadowBlur = 15;

            ctx.beginPath();
            ctx.arc(width / 2, height / 2, width / 2 - 15, -Math.PI / 2, -Math.PI / 2 + (Math.PI * 2 * progress));
            ctx.stroke();
        }
    }

    // Center content
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 4

        Text {
            text: Math.round(root.animatedValue) + "%"
            color: CyberpunkColors.glowingWhite
            font.pixelSize: 36
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "SYSTEM HEALTH"
            color: CyberpunkColors.dimGray
            font.pixelSize: 11
            font.letterSpacing: 2
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
