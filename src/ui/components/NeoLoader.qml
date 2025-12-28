import QtQuick
import QtQuick.Controls
import "../style"

// ============================================================================
// NEO-Z BOOT LOADER (NeoLoader)
// Theme-aware boot animation - NO VIDEO during boot (avoids overlay issues)
// ============================================================================

Item {
    id: root
    anchors.fill: parent

    signal finished
    signal startReveal

    property int loadDuration: 5000
    property real progress: 0.0
    property bool isComplete: false

    // Theme-aware colors
    readonly property color sigilColor: ThemeManager.isLightMode ? Qt.rgba(30 / 255, 30 / 255, 40 / 255, 0.9) : Qt.rgba(200 / 255, 200 / 255, 210 / 255, 0.85)
    readonly property color accentColor: Style.primary

    // ========================================
    // LAYER 0: THEME BACKGROUND
    // ========================================

    // Dark theme background
    Rectangle {
        id: darkThemeBg
        anchors.fill: parent
        color: Style.loaderBgColor
        visible: !ThemeManager.isLightMode
        z: 0

        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
                GradientStop {
                    position: 0.85
                    color: Qt.rgba(0, 0, 0, 0.25)
                }
                GradientStop {
                    position: 1.0
                    color: Qt.rgba(0, 0, 0, 0.4)
                }
            }
        }
    }

    // Frost Glass background - Static gradient (NO VIDEO)
    Rectangle {
        id: frostThemeBg
        anchors.fill: parent
        visible: ThemeManager.isLightMode
        z: 0

        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0.0
                color: "#E8F0F8"
            }
            GradientStop {
                position: 0.3
                color: "#F0F4FA"
            }
            GradientStop {
                position: 0.7
                color: "#F8F9FC"
            }
            GradientStop {
                position: 1.0
                color: "#FAFBFD"
            }
        }
    }

    // ========================================
    // LAYER 1: FROSTED GLASS OVERLAY (Frost theme only)
    // ========================================
    Rectangle {
        id: frostedGlassOverlay
        anchors.fill: parent
        anchors.margins: 20
        radius: 24
        visible: ThemeManager.isLightMode
        z: 1

        color: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 0.65)
        border.color: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 0.5)
        border.width: 1

        // Inner depth gradient
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: 23
            color: "transparent"
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 0.25)
                }
                GradientStop {
                    position: 0.5
                    color: "transparent"
                }
                GradientStop {
                    position: 1.0
                    color: Qt.rgba(0, 0, 0, 0.03)
                }
            }
        }

        // ========================================
        // RISING DARK GLOW EFFECT
        // ========================================
        Item {
            id: risingGlowContainer
            anchors.centerIn: parent
            width: parent.width * 0.8
            height: parent.height * 0.6
            clip: true

            Repeater {
                model: 5
                Rectangle {
                    id: glowParticle
                    property real startY: risingGlowContainer.height + index * 60
                    property real particleWidth: 100 + Math.random() * 60

                    width: particleWidth
                    height: 160
                    x: (risingGlowContainer.width - width) / 2 + (index - 2) * 45
                    y: startY
                    radius: width / 2
                    color: "transparent"

                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: "transparent"
                        }
                        GradientStop {
                            position: 0.3
                            color: Qt.rgba(0, 0, 0, 0.06)
                        }
                        GradientStop {
                            position: 0.5
                            color: Qt.rgba(0, 0, 0, 0.12)
                        }
                        GradientStop {
                            position: 0.7
                            color: Qt.rgba(0, 0, 0, 0.06)
                        }
                        GradientStop {
                            position: 1.0
                            color: "transparent"
                        }
                    }

                    SequentialAnimation on y {
                        running: !root.isComplete
                        loops: Animation.Infinite
                        NumberAnimation {
                            from: glowParticle.startY
                            to: -180
                            duration: 3500 + index * 350
                            easing.type: Easing.Linear
                        }
                        PropertyAction {
                            value: glowParticle.startY
                        }
                    }

                    opacity: 0.6 - index * 0.06
                }
            }

            // Central dark aura
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -40
                width: 260
                height: 180
                radius: 130

                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: Qt.rgba(0, 0, 0, 0.10)
                    }
                    GradientStop {
                        position: 0.5
                        color: Qt.rgba(0, 0, 0, 0.05)
                    }
                    GradientStop {
                        position: 1.0
                        color: "transparent"
                    }
                }

                SequentialAnimation on opacity {
                    running: !root.isComplete
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 0.8
                        duration: 2000
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        to: 0.4
                        duration: 2000
                        easing.type: Easing.InOutSine
                    }
                }
            }
        }
    }

    // ========================================
    // LAYER 10: BOOT LOGO CONTAINER
    // ========================================
    Item {
        id: logoContainer
        anchors.centerIn: parent
        width: 320
        height: 400
        opacity: 0
        scale: 0.85
        z: 10

        // ========================================
        // SIGIL CONTAINER
        // ========================================
        Item {
            id: sigilContainer
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            width: 220
            height: 220
            opacity: 0.9

            SequentialAnimation on opacity {
                running: !root.isComplete
                loops: Animation.Infinite
                NumberAnimation {
                    to: 1.0
                    duration: 2000
                    easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    to: 0.8
                    duration: 2000
                    easing.type: Easing.InOutSine
                }
            }

            // OUTER RING
            Canvas {
                id: outerRing
                anchors.fill: parent
                opacity: 0.9

                property real rotationAngle: 0

                RotationAnimation on rotationAngle {
                    running: !root.isComplete
                    from: 0
                    to: 360
                    duration: 14000
                    loops: Animation.Infinite
                }

                onRotationAngleChanged: requestPaint()

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.save();
                    ctx.translate(width / 2, height / 2);
                    ctx.rotate(rotationAngle * Math.PI / 180);

                    ctx.strokeStyle = root.sigilColor.toString();
                    ctx.lineWidth = 1.5;
                    ctx.beginPath();
                    ctx.arc(0, 0, 100, 0, Math.PI * 2);
                    ctx.stroke();

                    for (var i = 0; i < 12; i++) {
                        ctx.save();
                        ctx.rotate(i * Math.PI / 6);
                        ctx.beginPath();
                        ctx.moveTo(95, 0);
                        ctx.lineTo(100, 0);
                        ctx.stroke();
                        ctx.restore();
                    }
                    ctx.restore();
                }
            }

            // INNER RING
            Canvas {
                id: innerRing
                anchors.fill: parent
                opacity: 0.8

                property real rotationAngle: 0

                RotationAnimation on rotationAngle {
                    running: !root.isComplete
                    from: 360
                    to: 0
                    duration: 10000
                    loops: Animation.Infinite
                }

                onRotationAngleChanged: requestPaint()

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.save();
                    ctx.translate(width / 2, height / 2);
                    ctx.rotate(rotationAngle * Math.PI / 180);

                    ctx.strokeStyle = root.sigilColor.toString();
                    ctx.lineWidth = 1;
                    ctx.beginPath();
                    ctx.arc(0, 0, 70, 0, Math.PI * 2);
                    ctx.stroke();

                    ctx.beginPath();
                    for (var i = 0; i < 6; i++) {
                        var angle = i * Math.PI / 3;
                        var x = 70 * Math.cos(angle);
                        var y = 70 * Math.sin(angle);
                        if (i === 0)
                            ctx.moveTo(x, y);
                        else
                            ctx.lineTo(x, y);
                    }
                    ctx.closePath();
                    ctx.stroke();
                    ctx.restore();
                }
            }

            // CENTER Z SYMBOL
            Canvas {
                id: centerSymbol
                anchors.centerIn: parent
                width: 60
                height: 60
                opacity: 0.9

                property real pulseScale: 1.0

                SequentialAnimation on pulseScale {
                    running: !root.isComplete
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 1.05
                        duration: 1500
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        to: 1.0
                        duration: 1500
                        easing.type: Easing.InOutSine
                    }
                }

                onPulseScaleChanged: requestPaint()

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.save();
                    ctx.translate(width / 2, height / 2);
                    ctx.scale(pulseScale, pulseScale);

                    ctx.strokeStyle = root.accentColor.toString();
                    ctx.lineWidth = 3;
                    ctx.lineCap = "round";
                    ctx.beginPath();
                    ctx.moveTo(-15, -15);
                    ctx.lineTo(15, -15);
                    ctx.lineTo(-15, 15);
                    ctx.lineTo(15, 15);
                    ctx.stroke();
                    ctx.restore();
                }
            }

            // ACCENT GLOW RING
            Canvas {
                id: glowRing
                anchors.fill: parent
                opacity: 0.5

                property real glowPhase: 0

                NumberAnimation on glowPhase {
                    running: !root.isComplete
                    from: 0
                    to: Math.PI * 2
                    duration: 3000
                    loops: Animation.Infinite
                }

                onGlowPhaseChanged: requestPaint()

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.save();
                    ctx.translate(width / 2, height / 2);

                    ctx.strokeStyle = root.accentColor.toString();
                    ctx.lineWidth = 2;
                    ctx.beginPath();
                    ctx.arc(0, 0, 85, glowPhase, glowPhase + Math.PI * 0.5);
                    ctx.stroke();
                    ctx.restore();
                }
            }
        }

        // BRAND TEXT
        Text {
            id: brandText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: sigilContainer.bottom
            anchors.topMargin: 25
            text: "NEO-Z"
            font.family: "Georgia"
            font.pixelSize: 28
            font.letterSpacing: 12
            color: ThemeManager.isLightMode ? "#1a1a2e" : "#E8E8E8"
            opacity: 0.95
        }

        // STATUS TEXT
        Text {
            id: statusText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: brandText.bottom
            anchors.topMargin: 12
            text: root.isComplete ? "System Ready" : "initializing system"
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.weight: Font.Light
            font.letterSpacing: 4
            color: ThemeManager.isLightMode ? Qt.rgba(50 / 255, 50 / 255, 60 / 255, 0.65) : Qt.rgba(200 / 255, 200 / 255, 200 / 255, 0.65)

            SequentialAnimation on opacity {
                running: !root.isComplete
                loops: Animation.Infinite
                NumberAnimation {
                    to: 0.8
                    duration: 2000
                    easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    to: 0.4
                    duration: 2500
                    easing.type: Easing.InOutSine
                }
            }
        }

        // PROGRESS BAR
        Item {
            id: progressBar
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: statusText.bottom
            anchors.topMargin: 25
            width: 140
            height: 3

            Rectangle {
                anchors.fill: parent
                radius: 1.5
                color: ThemeManager.isLightMode ? Qt.rgba(0, 0, 0, 0.1) : Qt.rgba(255, 255, 255, 0.12)
            }

            Rectangle {
                anchors.left: parent.left
                height: parent.height
                width: parent.width * root.progress
                radius: 1.5

                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop {
                        position: 0.0
                        color: Style.primary
                    }
                    GradientStop {
                        position: 1.0
                        color: Style.secondary
                    }
                }

                Behavior on width {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.OutQuad
                    }
                }
            }
        }

        Component.onCompleted: bootInAnimation.start()

        ParallelAnimation {
            id: bootInAnimation
            NumberAnimation {
                target: logoContainer
                property: "opacity"
                from: 0
                to: 1
                duration: 800
                easing.type: Easing.OutQuad
            }
            NumberAnimation {
                target: logoContainer
                property: "scale"
                from: 0.85
                to: 1.0
                duration: 800
                easing.type: Easing.OutBack
            }
        }
    }

    // ========================================
    // LOAD PROGRESS TIMER
    // ========================================
    Timer {
        id: loadTimer
        interval: 50
        repeat: true
        running: true

        onTriggered: {
            if (root.progress < 1.0) {
                root.progress += 0.01;
            } else {
                loadTimer.running = false;
                completeSequence.start();
            }
        }
    }

    // ========================================
    // COMPLETION SEQUENCE
    // ========================================
    SequentialAnimation {
        id: completeSequence

        ScriptAction {
            script: {
                root.isComplete = true;
                root.startReveal();
            }
        }

        PauseAnimation {
            duration: 600
        }

        ParallelAnimation {
            NumberAnimation {
                target: logoContainer
                property: "opacity"
                to: 0
                duration: 500
                easing.type: Easing.InQuad
            }
            NumberAnimation {
                target: logoContainer
                property: "scale"
                to: 1.1
                duration: 500
                easing.type: Easing.InQuad
            }
            NumberAnimation {
                target: frostedGlassOverlay
                property: "opacity"
                to: 0
                duration: 600
                easing.type: Easing.InQuad
            }
            NumberAnimation {
                target: frostThemeBg
                property: "opacity"
                to: 0
                duration: 700
                easing.type: Easing.InQuad
            }
            NumberAnimation {
                target: darkThemeBg
                property: "opacity"
                to: 0
                duration: 700
                easing.type: Easing.InQuad
            }
        }

        PauseAnimation {
            duration: 200
        }

        ScriptAction {
            script: root.finished()
        }
    }
}
