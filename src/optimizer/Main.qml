import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window

Window {
    id: mainWindow
    width: 1200
    height: 700
    visible: true
    title: "NeoZ Optimizer"
    color: "#0A0A10"
    flags: Qt.Window | Qt.FramelessWindowHint

    // Drag to move window
    MouseArea {
        id: dragArea
        anchors.fill: parent
        property point clickPos
        onPressed: clickPos = Qt.point(mouse.x, mouse.y)
        onPositionChanged: {
            var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y);
            mainWindow.x += delta.x;
            mainWindow.y += delta.y;
        }
    }

    // Border glow
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: Qt.rgba(0.4, 0.2, 0.9, 0.4)
        radius: 20
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        // Header with gradient background
        Rectangle {
            Layout.fillWidth: true
            height: 56
            radius: 12
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: Qt.rgba(0.1, 0.05, 0.2, 0.6)
                }
                GradientStop {
                    position: 0.5
                    color: Qt.rgba(0.05, 0.1, 0.15, 0.4)
                }
                GradientStop {
                    position: 1.0
                    color: Qt.rgba(0.1, 0.05, 0.2, 0.6)
                }
            }
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.08)

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 12

                // Animated accent bar
                Rectangle {
                    width: 4
                    height: 28
                    radius: 2
                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: "#9D00FF"
                        }
                        GradientStop {
                            position: 0.5
                            color: "#00D4FF"
                        }
                        GradientStop {
                            position: 1.0
                            color: "#00FF88"
                        }
                    }

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation {
                            to: 0.6
                            duration: 1500
                            easing.type: Easing.InOutSine
                        }
                        NumberAnimation {
                            to: 1.0
                            duration: 1500
                            easing.type: Easing.InOutSine
                        }
                    }
                }

                // Title with glow effect
                Text {
                    text: "SYSTEM OPTIMIZATION"
                    color: "#FFFFFF"
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    font.letterSpacing: 2
                    style: Text.Outline
                    styleColor: Qt.rgba(0, 0.8, 1, 0.3)
                }

                // Version badge
                Rectangle {
                    width: 48
                    height: 22
                    radius: 11
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0.0
                            color: "#9D00FF"
                        }
                        GradientStop {
                            position: 1.0
                            color: "#00D4FF"
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "v2.0"
                        color: "#FFFFFF"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // Status indicator
                Row {
                    spacing: 8

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: "#00FF41"

                        SequentialAnimation on opacity {
                            loops: Animation.Infinite
                            NumberAnimation {
                                to: 0.4
                                duration: 800
                            }
                            NumberAnimation {
                                to: 1.0
                                duration: 800
                            }
                        }
                    }

                    Text {
                        text: "ACTIVE"
                        color: "#00FF41"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                        font.letterSpacing: 1
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Item {
                    width: 16
                }

                // Minimize button
                Rectangle {
                    width: 36
                    height: 36
                    radius: 10
                    color: minHover.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.03)
                    border.width: 1
                    border.color: minHover.containsMouse ? Qt.rgba(1, 1, 1, 0.3) : Qt.rgba(1, 1, 1, 0.1)

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                    Behavior on border.color {
                        ColorAnimation {
                            duration: 150
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "-"
                        color: minHover.containsMouse ? "#FFF" : "#888"
                        font.pixelSize: 20
                        font.weight: Font.Bold

                        Behavior on color {
                            ColorAnimation {
                                duration: 150
                            }
                        }
                    }

                    MouseArea {
                        id: minHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: mainWindow.showMinimized()
                    }
                }

                // Close button
                Rectangle {
                    width: 36
                    height: 36
                    radius: 10
                    color: closeHover.containsMouse ? Qt.rgba(1, 0.2, 0.2, 0.4) : Qt.rgba(1, 1, 1, 0.03)
                    border.width: 1
                    border.color: closeHover.containsMouse ? "#FF4444" : Qt.rgba(1, 1, 1, 0.1)

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                    Behavior on border.color {
                        ColorAnimation {
                            duration: 150
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "x"
                        color: closeHover.containsMouse ? "#FF4444" : "#888"
                        font.pixelSize: 14
                        font.weight: Font.Bold

                        Behavior on color {
                            ColorAnimation {
                                duration: 150
                            }
                        }
                    }

                    MouseArea {
                        id: closeHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.quit()
                    }
                }
            }
        }

        // ========== NAVIGATION BAR ==========
        Rectangle {
            Layout.fillWidth: true
            height: 48
            radius: 10
            color: Qt.rgba(0.03, 0.03, 0.08, 0.9)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.08)

            property int currentTab: 0  // 0 = System, 1 = Zereca

            RowLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                // System Tab
                Rectangle {
                    Layout.preferredWidth: 140
                    Layout.fillHeight: true
                    radius: 8
                    color: parent.parent.currentTab === 0 ? Qt.rgba(0.6, 0, 1, 0.25) : systemTabHover.containsMouse ? Qt.rgba(1, 1, 1, 0.05) : "transparent"
                    border.width: parent.parent.currentTab === 0 ? 1 : 0
                    border.color: "#9D00FF"

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: "âš™"
                            font.pixelSize: 14
                        }

                        Text {
                            text: "SYSTEM"
                            color: parent.parent.parent.parent.currentTab === 0 ? "#FFFFFF" : "#888888"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            font.letterSpacing: 1

                            Behavior on color {
                                ColorAnimation {
                                    duration: 150
                                }
                            }
                        }
                    }

                    MouseArea {
                        id: systemTabHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: parent.parent.parent.currentTab = 0
                    }
                }

                // Zereca Tab
                Rectangle {
                    Layout.preferredWidth: 160
                    Layout.fillHeight: true
                    radius: 8
                    color: parent.parent.currentTab === 1 ? Qt.rgba(0, 0.8, 1, 0.2) : zerecaTabHover.containsMouse ? Qt.rgba(1, 1, 1, 0.05) : "transparent"
                    border.width: parent.parent.currentTab === 1 ? 1 : 0
                    border.color: "#00D4FF"

                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: "ðŸ§ "
                            font.pixelSize: 14
                        }

                        Text {
                            text: "ZERECA"
                            color: parent.parent.parent.parent.currentTab === 1 ? "#FFFFFF" : "#888888"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            font.letterSpacing: 1

                            Behavior on color {
                                ColorAnimation {
                                    duration: 150
                                }
                            }
                        }

                        // Beta badge
                        Rectangle {
                            width: 36
                            height: 16
                            radius: 8
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop {
                                    position: 0.0
                                    color: "#00D4FF"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: "#9D00FF"
                                }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: "BETA"
                                color: "#FFFFFF"
                                font.pixelSize: 7
                                font.weight: Font.Bold
                            }
                        }
                    }

                    MouseArea {
                        id: zerecaTabHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: parent.parent.parent.currentTab = 1
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // Tab description
                Text {
                    text: parent.parent.currentTab === 0 ? "Manual system optimizations and tweaks" : "Adaptive AI-powered performance control"
                    color: "#666666"
                    font.pixelSize: 10
                    font.italic: true
                }

                Item {
                    width: 12
                }
            }
        }

        // ========== CONTENT STACK ==========
        StackLayout {
            id: mainContentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: navBar.currentTab

            // Get reference to nav bar
            property Item navBar: parent.children[1]  // Second child is nav bar

            // ===== TAB 0: System Optimization (existing content) =====
            Item {
                id: systemOptimizationTab

                // 3-Column Layout
                RowLayout {
                    anchors.fill: parent
                    spacing: 16

                    // ========== LEFT: SYSTEM VITALITY ==========
                    Rectangle {
                        id: vitalityContainer
                        Layout.preferredWidth: 280
                        Layout.fillHeight: true
                        radius: 20
                        color: Qt.rgba(0.02, 0.02, 0.06, 0.95)
                        border.width: 1
                        border.color: Qt.rgba(0, 0.8, 1, 0.25)
                        clip: true

                        // Outer glow layer
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -2
                            radius: parent.radius + 2
                            color: "transparent"
                            border.width: 1
                            border.color: Qt.rgba(0, 0.8, 1, 0.1)
                            z: -1
                        }

                        // Top accent line
                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: 1
                            height: 2
                            radius: 2
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop {
                                    position: 0.0
                                    color: "transparent"
                                }
                                GradientStop {
                                    position: 0.3
                                    color: "#00D4FF"
                                }
                                GradientStop {
                                    position: 0.7
                                    color: "#00D4FF"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: "transparent"
                                }
                            }
                        }

                        // Floating particles
                        Repeater {
                            model: 20
                            Rectangle {
                                id: particle
                                property real startX: Math.random() * vitalityContainer.width
                                property real startY: Math.random() * vitalityContainer.height
                                property real particleSize: 2 + Math.random() * 3
                                property real animDuration: 10000 + Math.random() * 10000

                                x: startX
                                y: startY
                                width: particleSize
                                height: particleSize
                                radius: particleSize / 2
                                color: Qt.rgba(0, 0.8, 1, 0.4)
                                opacity: 0.3 + Math.random() * 0.4

                                SequentialAnimation on y {
                                    loops: Animation.Infinite
                                    NumberAnimation {
                                        to: particle.startY - 50 - Math.random() * 60
                                        duration: particle.animDuration
                                        easing.type: Easing.InOutSine
                                    }
                                    NumberAnimation {
                                        to: particle.startY
                                        duration: particle.animDuration
                                        easing.type: Easing.InOutSine
                                    }
                                }

                                SequentialAnimation on opacity {
                                    loops: Animation.Infinite
                                    NumberAnimation {
                                        to: 0.15
                                        duration: particle.animDuration * 0.5
                                    }
                                    NumberAnimation {
                                        to: 0.6
                                        duration: particle.animDuration * 0.5
                                    }
                                }
                            }
                        }

                        // Gradient overlay
                        Rectangle {
                            anchors.fill: parent
                            gradient: Gradient {
                                GradientStop {
                                    position: 0.0
                                    color: Qt.rgba(0, 0, 0, 0.2)
                                }
                                GradientStop {
                                    position: 0.5
                                    color: Qt.rgba(0, 0.05, 0.1, 0.1)
                                }
                                GradientStop {
                                    position: 1.0
                                    color: Qt.rgba(0, 0, 0, 0.5)
                                }
                            }
                        }

                        // Holographic Orb System
                        Item {
                            id: orbSystem
                            anchors.fill: parent

                            // Title
                            Text {
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.margins: 16
                                text: "SYSTEM VITALITY"
                                color: "#00D4FF"
                                font.pixelSize: 11
                                font.weight: Font.Bold
                                font.letterSpacing: 1.5
                                z: 10
                            }

                            // Holographic glow rings
                            Repeater {
                                model: 3
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 100 + index * 35
                                    height: 100 + index * 35
                                    radius: width / 2
                                    color: "transparent"
                                    border.width: 1
                                    border.color: Qt.rgba(0, 1, 0.6, 0.2 - index * 0.05)

                                    RotationAnimation on rotation {
                                        from: index % 2 === 0 ? 0 : 360
                                        to: index % 2 === 0 ? 360 : 0
                                        duration: 25000 + index * 8000
                                        loops: Animation.Infinite
                                    }
                                }
                            }

                            // Main holographic health orb
                            Rectangle {
                                id: holoOrb
                                anchors.centerIn: parent
                                width: 100
                                height: 100
                                radius: 50
                                color: "transparent"
                                border.width: 2
                                border.color: "#00FF41"

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: 3
                                    radius: width / 2
                                    gradient: Gradient {
                                        GradientStop {
                                            position: 0.0
                                            color: Qt.rgba(0, 1, 0.4, 0.3)
                                        }
                                        GradientStop {
                                            position: 0.6
                                            color: Qt.rgba(0, 0.5, 0.2, 0.15)
                                        }
                                        GradientStop {
                                            position: 1.0
                                            color: Qt.rgba(0, 0.2, 0.1, 0.05)
                                        }
                                    }
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "87"
                                    font.pixelSize: 36
                                    font.weight: Font.Bold
                                    font.family: "Consolas"
                                    color: "#00FF88"
                                    style: Text.Outline
                                    styleColor: Qt.rgba(0, 1, 0.5, 0.4)
                                }

                                SequentialAnimation on scale {
                                    loops: Animation.Infinite
                                    NumberAnimation {
                                        to: 1.05
                                        duration: 1800
                                        easing.type: Easing.InOutQuad
                                    }
                                    NumberAnimation {
                                        to: 1.0
                                        duration: 1800
                                        easing.type: Easing.InOutQuad
                                    }
                                }

                                SequentialAnimation on opacity {
                                    loops: Animation.Infinite
                                    NumberAnimation {
                                        to: 0.8
                                        duration: 2500
                                        easing.type: Easing.InOutSine
                                    }
                                    NumberAnimation {
                                        to: 1.0
                                        duration: 2500
                                        easing.type: Easing.InOutSine
                                    }
                                }
                            }

                            // 6 Orbiting stat orbs
                            Repeater {
                                model: [
                                    {
                                        label: "CPU",
                                        color: "#00FF41",
                                        angle: 0
                                    },
                                    {
                                        label: "RAM",
                                        color: "#9D00FF",
                                        angle: 60
                                    },
                                    {
                                        label: "DISK",
                                        color: "#00D4FF",
                                        angle: 120
                                    },
                                    {
                                        label: "NET",
                                        color: "#FFD700",
                                        angle: 180
                                    },
                                    {
                                        label: "TEMP",
                                        color: "#FF6B35",
                                        angle: 240
                                    },
                                    {
                                        label: "PWR",
                                        color: "#00FFAA",
                                        angle: 300
                                    }
                                ]

                                Item {
                                    id: orbWrapper
                                    property real orbitRadius: 110
                                    property real baseAngle: modelData.angle
                                    property real currentAngle: baseAngle
                                    property string orbLabel: modelData.label
                                    property string orbColor: modelData.color

                                    // Get value based on label
                                    property string orbValue: {
                                        switch (orbLabel) {
                                        case "CPU":
                                            return Math.round(Optimizer.cpuUsage) + "%";
                                        case "RAM":
                                            return Math.round(Optimizer.ramUsage) + "%";
                                        case "DISK":
                                            return Math.round(Optimizer.diskUsage) + "%";
                                        case "NET":
                                            return Math.round(Optimizer.networkSpeed) + "M";
                                        case "TEMP":
                                            return Math.round(Optimizer.cpuTemp) + "Â°";
                                        case "PWR":
                                            return Math.round(Optimizer.powerDraw) + "W";
                                        default:
                                            return "--";
                                        }
                                    }

                                    anchors.centerIn: parent
                                    width: 46
                                    height: 46

                                    transform: Translate {
                                        x: orbWrapper.orbitRadius * Math.cos(orbWrapper.currentAngle * Math.PI / 180)
                                        y: orbWrapper.orbitRadius * Math.sin(orbWrapper.currentAngle * Math.PI / 180)
                                    }

                                    NumberAnimation on currentAngle {
                                        from: orbWrapper.baseAngle
                                        to: orbWrapper.baseAngle + 360
                                        duration: 40000
                                        loops: Animation.Infinite
                                    }

                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 50
                                        height: 50
                                        radius: 25
                                        color: "transparent"
                                        border.width: 1
                                        border.color: Qt.rgba(Qt.color(orbWrapper.orbColor).r, Qt.color(orbWrapper.orbColor).g, Qt.color(orbWrapper.orbColor).b, 0.4)

                                        SequentialAnimation on opacity {
                                            loops: Animation.Infinite
                                            NumberAnimation {
                                                to: 0.3
                                                duration: 2000
                                            }
                                            NumberAnimation {
                                                to: 0.8
                                                duration: 2000
                                            }
                                        }
                                    }

                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 42
                                        height: 42
                                        radius: 21
                                        color: Qt.rgba(0, 0, 0, 0.75)
                                        border.width: 2
                                        border.color: orbWrapper.orbColor

                                        Rectangle {
                                            anchors.fill: parent
                                            anchors.margins: 2
                                            radius: width / 2
                                            gradient: Gradient {
                                                GradientStop {
                                                    position: 0.0
                                                    color: Qt.rgba(Qt.color(orbWrapper.orbColor).r, Qt.color(orbWrapper.orbColor).g, Qt.color(orbWrapper.orbColor).b, 0.35)
                                                }
                                                GradientStop {
                                                    position: 1.0
                                                    color: "transparent"
                                                }
                                            }
                                        }

                                        Column {
                                            anchors.centerIn: parent
                                            spacing: 1

                                            Text {
                                                text: orbWrapper.orbValue
                                                color: orbWrapper.orbColor
                                                font.pixelSize: 11
                                                font.weight: Font.Bold
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                            Text {
                                                text: orbWrapper.orbLabel
                                                color: Qt.rgba(1, 1, 1, 0.6)
                                                font.pixelSize: 7
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // ========== CENTER: CONTROL MATRIX ==========
                    Rectangle {
                        Layout.preferredWidth: 420
                        Layout.fillHeight: true
                        radius: 20
                        color: Qt.rgba(0.02, 0.02, 0.06, 0.95)
                        border.width: 1
                        border.color: Qt.rgba(0.6, 0, 1, 0.25)
                        clip: true

                        // Outer glow layer
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -2
                            radius: parent.radius + 2
                            color: "transparent"
                            border.width: 1
                            border.color: Qt.rgba(0.6, 0, 1, 0.1)
                            z: -1
                        }

                        // Top accent line
                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: 1
                            height: 2
                            radius: 1
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop {
                                    position: 0.0
                                    color: "transparent"
                                }
                                GradientStop {
                                    position: 0.3
                                    color: "#9D00FF"
                                }
                                GradientStop {
                                    position: 0.7
                                    color: "#9D00FF"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: "transparent"
                                }
                            }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            // Section Header
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Rectangle {
                                    width: 3
                                    height: 18
                                    radius: 1.5
                                    color: "#9D00FF"
                                }

                                Text {
                                    text: "OPTIMIZATION CENTER"
                                    color: "#FFFFFF"
                                    font.pixelSize: 13
                                    font.weight: Font.Bold
                                    font.letterSpacing: 2
                                    style: Text.Outline
                                    styleColor: Qt.rgba(0.6, 0, 1, 0.3)
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }

                            // Load the new Zereca UI Panel
                            Loader {
                                id: zerecaUIPanelLoader
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                source: "ui/ZerecaUIPanel.qml"
                                asynchronous: false
                            }
                        }
                    }

                    // ========== RIGHT: AI COMMAND CENTRE ==========
                    Rectangle {
                        Layout.preferredWidth: 220
                        Layout.fillHeight: true
                        radius: 20
                        color: Qt.rgba(0.02, 0.02, 0.06, 0.95)
                        border.width: 1
                        border.color: Qt.rgba(1, 0.85, 0, 0.25)
                        clip: true

                        // Outer glow layer
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -2
                            radius: parent.radius + 2
                            color: "transparent"
                            border.width: 1
                            border.color: Qt.rgba(1, 0.85, 0, 0.1)
                            z: -1
                        }

                        // Top accent line
                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: 1
                            height: 2
                            radius: 1
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop {
                                    position: 0.0
                                    color: "transparent"
                                }
                                GradientStop {
                                    position: 0.3
                                    color: "#FFD700"
                                }
                                GradientStop {
                                    position: 0.7
                                    color: "#FFD700"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: "transparent"
                                }
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

                                Rectangle {
                                    width: 3
                                    height: 16
                                    radius: 1.5
                                    color: "#FFD700"
                                }

                                Text {
                                    text: "AI ASSISTANT"
                                    color: "#FFD700"
                                    font.pixelSize: 12
                                    font.weight: Font.Bold
                                    font.letterSpacing: 1
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }

                            // AI Status
                            Rectangle {
                                Layout.fillWidth: true
                                height: 60
                                radius: 8
                                color: Qt.rgba(1, 0.85, 0, 0.05)
                                border.width: 1
                                border.color: Qt.rgba(1, 0.85, 0, 0.2)

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 4

                                    Text {
                                        text: "System Status"
                                        color: Qt.rgba(1, 1, 1, 0.6)
                                        font.pixelSize: 9
                                    }

                                    Text {
                                        text: "Optimized"
                                        color: "#00FF41"
                                        font.pixelSize: 14
                                        font.weight: Font.Bold
                                    }
                                }
                            }

                            // Spacer
                            Item {
                                Layout.fillHeight: true
                            }
                        } // End ColumnLayout for AI COMMAND CENTRE
                    } // End RIGHT: AI COMMAND CENTRE Rectangle
                } // End RowLayout (3-column)
            } // End Item (systemOptimizationTab)

            // ===== TAB 1: Zereca Control Plane =====
            Item {
                id: zerecaTab

                Loader {
                    anchors.fill: parent
                    source: "ui/ZerecaView.qml"
                }
            }
        } // End StackLayout

        // ==================== ADVANCED OPTIMIZATION POPUP ====================
        Popup {
            id: advancedPopup
            anchors.centerIn: parent
            width: 700
            height: 520
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

            background: Rectangle {
                color: Qt.rgba(0.05, 0.05, 0.1, 0.98)
                radius: 16
                border.width: 2
                border.color: "#FF6B00"
            }

            contentItem: ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                // Header
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "ADVANCED OPTIMIZATION"
                        color: "#FF6B00"
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        font.letterSpacing: 2
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Rectangle {
                        width: 32
                        height: 32
                        radius: 16
                        color: Qt.rgba(1, 1, 1, 0.1)
                        Text {
                            anchors.centerIn: parent
                            text: "✕"
                            color: "#FFFFFF"
                            font.pixelSize: 16
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: advancedPopup.close()
                        }
                    }
                }

                // Content area
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 12
                    color: Qt.rgba(0, 0, 0, 0.3)

                    Text {
                        anchors.centerIn: parent
                        text: "Advanced optimization options will appear here"
                        color: Qt.rgba(1, 1, 1, 0.5)
                        font.pixelSize: 14
                    }
                }

                // Action buttons
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Item {
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        width: 120
                        height: 40
                        radius: 8
                        color: Qt.rgba(1, 1, 1, 0.1)
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.2)
                        Text {
                            anchors.centerIn: parent
                            text: "Cancel"
                            color: "#FFFFFF"
                            font.pixelSize: 12
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: advancedPopup.close()
                        }
                    }

                    Rectangle {
                        width: 140
                        height: 40
                        radius: 8
                        color: "#FF6B00"
                        Text {
                            anchors.centerIn: parent
                            text: "Apply All"
                            color: "#FFFFFF"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                console.log("Advanced optimization applied");
                                advancedPopup.close();
                            }
                        }
                    }
                }
            }
        }
    } // End ColumnLayout
} // End Window
