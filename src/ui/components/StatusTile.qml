import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import NeoZ
import "../style"

// Glass Bubble Status Tile - Theme aware
Rectangle {
    id: root

    property string label: "Status"
    property string value: "---"
    property string subtext: ""
    property bool isActive: false
    property color activeColor: Style.primary
    property string iconText: "●"
    property bool isHovered: false

    signal clicked

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumWidth: 100

    // Theme-aware styling from HUD theme
    radius: Style.hudTileRadius

    // Use HUD theme background color
    color: Style.hudTileBackground

    // Dynamic border using HUD theme colors
    border.width: isActive ? 2 : 1
    border.color: isActive ? activeColor : (isHovered ? Style.hudTileBorderActive : Style.hudTileBorder)

    // Smooth transitions
    Behavior on color {
        ColorAnimation {
            duration: Style.motionDuration
        }
    }
    Behavior on radius {
        NumberAnimation {
            duration: Style.motionDuration
        }
    }
    Behavior on border.width {
        NumberAnimation {
            duration: 200
            easing.type: Easing.OutCubic
        }
    }

    // === GLASS 3D EFFECT LAYERS ===

    // Top glass highlight (3D shine effect)
    Rectangle {
        id: topHighlight
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 2
        height: parent.height * 0.4
        radius: parent.radius - 2
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Style.isLightMode ? Qt.rgba(0, 0, 0, 0.03) : Qt.rgba(1, 1, 1, 0.15)
            }
            GradientStop {
                position: 0.5
                color: Style.isLightMode ? Qt.rgba(0, 0, 0, 0.01) : Qt.rgba(1, 1, 1, 0.05)
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    // Inner bubble surface
    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        radius: parent.radius - 1
        color: "transparent"
        border.width: 1
        border.color: Style.isLightMode ? Qt.rgba(0, 0, 0, 0.06) : Qt.rgba(1, 1, 1, 0.08)
    }

    // Bottom shadow for 3D depth
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 2
        height: parent.height * 0.3
        radius: parent.radius - 2
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 0.7
                color: Style.isLightMode ? Qt.rgba(0, 0, 0, 0.03) : Qt.rgba(0, 0, 0, 0.1)
            }
            GradientStop {
                position: 1.0
                color: Style.isLightMode ? Qt.rgba(0, 0, 0, 0.06) : Qt.rgba(0, 0, 0, 0.2)
            }
        }
    }

    // === SABER GLOW EFFECT (when active) ===
    layer.enabled: (isActive || isHovered) && Style.hudTileGlowEnabled
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: isActive ? activeColor : Qt.rgba(1, 1, 1, 0.2)
        shadowBlur: isActive ? 0.8 : 0.3
        shadowOpacity: isActive ? 0.8 : 0.15
        shadowVerticalOffset: 0
        shadowHorizontalOffset: 0
        autoPaddingEnabled: true
    }

    // === ANIMATED GLOW RING (Saber pulsing effect) ===
    Rectangle {
        id: glowRing
        anchors.fill: parent
        anchors.margins: -3
        radius: parent.radius + 3
        color: "transparent"
        border.width: isActive ? 2 : 0
        border.color: Qt.alpha(activeColor, glowOpacity)
        visible: isActive

        property real glowOpacity: 0.6

        SequentialAnimation on glowOpacity {
            running: isActive
            loops: Animation.Infinite
            NumberAnimation {
                to: 0.9
                duration: 800
                easing.type: Easing.InOutSine
            }
            NumberAnimation {
                to: 0.4
                duration: 800
                easing.type: Easing.InOutSine
            }
        }
    }

    // === CONTENT ===
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 6
        anchors.bottomMargin: 6
        spacing: 8

        // Icon with glow
        Item {
            width: 26
            height: 26
            Layout.alignment: Qt.AlignVCenter

            // Icon background (frosted glass effect)
            Rectangle {
                anchors.centerIn: parent
                width: 24
                height: 24
                radius: 8
                color: isActive ? Qt.alpha(activeColor, 0.2) : (Style.isLightMode ? Qt.rgba(0, 0, 0, 0.05) : Qt.rgba(1, 1, 1, 0.08))
                border.width: 1
                border.color: isActive ? Qt.alpha(activeColor, 0.5) : (Style.isLightMode ? Qt.rgba(0, 0, 0, 0.1) : Qt.rgba(1, 1, 1, 0.1))

                Behavior on color {
                    ColorAnimation {
                        duration: 200
                    }
                }
                Behavior on border.color {
                    ColorAnimation {
                        duration: 200
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: iconText
                    font.pixelSize: 12
                    color: isActive ? activeColor : Style.hudLabelColor

                    Behavior on color {
                        ColorAnimation {
                            duration: 200
                        }
                    }
                }
            }
        }

        // Text content
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 1

            Text {
                text: root.label
                color: Style.hudLabelColor
                font.pixelSize: 9
                font.weight: Font.Medium
                opacity: 0.8
            }

            Text {
                text: root.value
                color: isActive ? activeColor : Style.hudValueColor
                font.pixelSize: 11
                font.bold: true

                Behavior on color {
                    ColorAnimation {
                        duration: 200
                    }
                }
            }

            Text {
                visible: subtext !== ""
                text: root.subtext
                color: isActive ? Qt.lighter(activeColor, 1.3) : Style.textSecondary
                font.pixelSize: 8
                opacity: 0.7
            }
        }

        // Active indicator dot
        Rectangle {
            visible: isActive
            width: 6
            height: 6
            radius: 3
            color: activeColor
            Layout.alignment: Qt.AlignTop | Qt.AlignRight
            Layout.topMargin: 2

            // Pulsing animation
            SequentialAnimation on opacity {
                running: isActive
                loops: Animation.Infinite
                NumberAnimation {
                    to: 1.0
                    duration: 600
                    easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    to: 0.3
                    duration: 600
                    easing.type: Easing.InOutSine
                }
            }
        }
    }

    // === HOVER & CLICK HANDLER ===
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onEntered: root.isHovered = true
        onExited: root.isHovered = false
        onClicked: root.clicked()
    }

    // Press effect (slight scale down)
    states: State {
        name: "pressed"
        when: hoverArea.pressed
        PropertyChanges {
            target: root
            scale: 0.97
        }
    }

    transitions: Transition {
        NumberAnimation {
            property: "scale"
            duration: 100
            easing.type: Easing.OutCubic
        }
    }
}
