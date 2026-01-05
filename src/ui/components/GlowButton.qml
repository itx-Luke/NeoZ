import QtQuick
import QtQuick.Layouts
import "../style"

// Neon Glow Button with particle effects
Rectangle {
    id: root

    property string text: "BUTTON"
    property string icon: ""
    property color baseColor: CyberpunkColors.neonBlue
    property color glowColor: baseColor
    property bool enabled: true
    property int buttonType: 0 // 0=primary, 1=secondary, 2=tertiary

    signal clicked

    width: 180
    height: buttonType === 0 ? 48 : (buttonType === 1 ? 40 : 32)
    radius: 10

    // Background based on button type
    color: {
        if (!enabled)
            return Qt.rgba(0.3, 0.3, 0.3, 0.3);
        if (buttonType === 2)
            return "transparent";
        return mouseArea.containsMouse ? Qt.darker(baseColor, 1.1) : baseColor;
    }

    border.width: buttonType === 2 ? 1 : 0
    border.color: buttonType === 2 ? baseColor : "transparent"

    opacity: enabled ? 1.0 : 0.5

    // Outer glow
    Rectangle {
        visible: buttonType === 0 && enabled
        anchors.fill: parent
        anchors.margins: -4
        radius: parent.radius + 4
        color: "transparent"
        border.width: 3
        border.color: Qt.rgba(glowColor.r, glowColor.g, glowColor.b, mouseArea.containsMouse ? 0.5 : 0.25)
        z: -1

        Behavior on border.color {
            ColorAnimation {
                duration: 200
            }
        }
    }

    // Inner gradient highlight
    Rectangle {
        visible: buttonType !== 2
        anchors.fill: parent
        anchors.margins: 1
        radius: parent.radius - 1
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.rgba(1, 1, 1, 0.15)
            }
            GradientStop {
                position: 0.3
                color: "transparent"
            }
        }
    }

    // Content
    RowLayout {
        anchors.centerIn: parent
        spacing: 8

        Text {
            visible: root.icon !== ""
            text: root.icon
            font.pixelSize: buttonType === 0 ? 20 : 16
        }

        Text {
            text: root.text
            color: buttonType === 2 ? root.baseColor : "#FFFFFF"
            font.pixelSize: buttonType === 0 ? 14 : (buttonType === 1 ? 13 : 12)
            font.bold: true
            font.letterSpacing: buttonType === 0 ? 1.5 : 0.5
        }
    }

    // Mouse interaction
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.enabled
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

        onClicked: if (root.enabled)
            root.clicked()
        onPressed: if (root.enabled)
            pressAnim.start()
    }

    // Press animation
    NumberAnimation {
        id: pressAnim
        target: root
        property: "scale"
        from: 1.0
        to: 0.95
        duration: 100
        easing.type: Easing.OutQuad
        onFinished: releaseAnim.start()
    }

    NumberAnimation {
        id: releaseAnim
        target: root
        property: "scale"
        from: 0.95
        to: 1.0
        duration: 150
        easing.type: Easing.OutBack
    }

    // Hover animation
    Behavior on scale {
        NumberAnimation {
            duration: 100
        }
    }
    scale: mouseArea.containsMouse && enabled ? 1.03 : 1.0
}
