import QtQuick
import "../style"

// GalaxyBackground - Deep black galaxy with cosmic particles & parallax
// Premium luxury background with mystical glow effects
Item {
    id: root
    
    property real mouseX: 0.5
    property real mouseY: 0.5
    property bool adbConnected: false
    
    // Mouse tracking for parallax
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPositionChanged: function(mouse) {
            root.mouseX = mouse.x / width;
            root.mouseY = mouse.y / height;
        }
    }
    
    // Base gradient - slightly lighter for better visibility
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#0A0E16" }
            GradientStop { position: 0.3; color: "#0C1018" }
            GradientStop { position: 0.6; color: "#0E121C" }
            GradientStop { position: 1.0; color: "#101424" }
        }
    }
    
    // ========== MYSTICAL GLOW ORBS ==========
    
    // Large cyan orb (top-left)
    Rectangle {
        id: glowOrb1
        width: 350
        height: 350
        radius: width / 2
        x: -80 + (mouseX - 0.5) * 40
        y: -60 + (mouseY - 0.5) * 30
        opacity: 0.18 + (adbConnected ? 0.08 : 0)
        
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#6EEBFF" }
            GradientStop { position: 0.4; color: "#4BA8FF" }
            GradientStop { position: 1.0; color: "transparent" }
        }
        
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.18; to: 0.12; duration: 3500; easing.type: Easing.InOutSine }
            NumberAnimation { from: 0.12; to: 0.18; duration: 3500; easing.type: Easing.InOutSine }
        }
        
        Behavior on x { NumberAnimation { duration: 180 } }
        Behavior on y { NumberAnimation { duration: 180 } }
    }
    
    // Violet orb (right side)
    Rectangle {
        id: glowOrb2
        width: 400
        height: 400
        radius: width / 2
        x: parent.width - 200 + (mouseX - 0.5) * 35
        y: parent.height * 0.3 + (mouseY - 0.5) * 25
        opacity: 0.15 + (adbConnected ? 0.06 : 0)
        
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#9A8CFF" }
            GradientStop { position: 0.5; color: "#6B5AE0" }
            GradientStop { position: 1.0; color: "transparent" }
        }
        
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.15; to: 0.10; duration: 4200; easing.type: Easing.InOutSine }
            NumberAnimation { from: 0.10; to: 0.15; duration: 4200; easing.type: Easing.InOutSine }
        }
        
        Behavior on x { NumberAnimation { duration: 200 } }
        Behavior on y { NumberAnimation { duration: 200 } }
    }
    
    // Small cyan accent (bottom-left)
    Rectangle {
        id: glowOrb3
        width: 200
        height: 200
        radius: width / 2
        x: parent.width * 0.1 + (mouseX - 0.5) * 50
        y: parent.height - 180 + (mouseY - 0.5) * 40
        opacity: 0.14
        
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#6EEBFF" }
            GradientStop { position: 0.6; color: "#2A8C9F" }
            GradientStop { position: 1.0; color: "transparent" }
        }
        
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.14; to: 0.08; duration: 2800; easing.type: Easing.InOutSine }
            NumberAnimation { from: 0.08; to: 0.14; duration: 2800; easing.type: Easing.InOutSine }
        }
    }
    
    // Amber accent (center-right)
    Rectangle {
        id: glowOrb4
        width: 180
        height: 180
        radius: width / 2
        x: parent.width * 0.75 + (mouseX - 0.5) * 25
        y: parent.height * 0.1 + (mouseY - 0.5) * 20
        opacity: 0.08
        
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#FFAB40" }
            GradientStop { position: 0.5; color: "#CC8030" }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }
    
    // ========== CENTRAL BLOOM ==========
    Rectangle {
        id: centerBloom
        width: parent.width * 0.8
        height: parent.height * 0.8
        anchors.centerIn: parent
        radius: width / 2
        opacity: 0.08 + (adbConnected ? 0.04 : 0)
        
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#6EEBFF" }
            GradientStop { position: 0.2; color: "#9A8CFF" }
            GradientStop { position: 0.5; color: "#1A0A2E" }
            GradientStop { position: 1.0; color: "transparent" }
        }
        
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.08; to: 0.05; duration: 5000; easing.type: Easing.InOutSine }
            NumberAnimation { from: 0.05; to: 0.08; duration: 5000; easing.type: Easing.InOutSine }
        }
    }
    
    // ========== STAR PARTICLES ==========
    Canvas {
        id: particleCanvas
        anchors.fill: parent
        
        property real time: 0
        property real offsetX: (mouseX - 0.5) * 30
        property real offsetY: (mouseY - 0.5) * 30
        
        Timer {
            interval: 50
            running: true
            repeat: true
            onTriggered: {
                particleCanvas.time += 0.05;
                particleCanvas.requestPaint();
            }
        }
        
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            
            // 80 particles for more density
            var particles = 80;
            for (var i = 0; i < particles; i++) {
                var seed = i * 127.1 + 311.7;
                var px = ((Math.sin(seed) * 43758.5453) % 1);
                if (px < 0) px += 1;
                var py = ((Math.cos(seed * 0.7) * 43758.5453) % 1);
                if (py < 0) py += 1;
                
                var driftX = time * 0.3 * (0.5 + (i % 5) * 0.1);
                var driftY = time * 0.2 * (0.5 + (i % 7) * 0.1);
                
                var x = ((px * width + driftX + offsetX) % width);
                if (x < 0) x += width;
                var y = ((py * height + driftY + offsetY) % height);
                if (y < 0) y += height;
                
                // Slightly larger particles (more visible)
                var size = 0.8 + (i % 10) * 0.15;
                
                // Higher opacity for better visibility
                var alpha = 0.2 + (i % 5) * 0.1;
                alpha *= 0.7 + Math.sin(time * 0.5 + i) * 0.3;
                
                // Mix of cyan-white particles
                var colorType = i % 4;
                var r, g, b;
                if (colorType === 0) { r = 110; g = 235; b = 255; }  // Cyan
                else if (colorType === 1) { r = 154; g = 140; b = 255; }  // Violet
                else { r = 200 + (i % 3) * 18; g = 210 + (i % 4) * 12; b = 230; }  // White
                
                ctx.beginPath();
                ctx.arc(x, y, size, 0, Math.PI * 2);
                ctx.fillStyle = "rgba(" + r + "," + g + "," + b + "," + alpha + ")";
                ctx.fill();
            }
        }
        
        Behavior on offsetX { NumberAnimation { duration: 150 } }
        Behavior on offsetY { NumberAnimation { duration: 150 } }
    }
    
    // ========== LIGHT RAYS ==========
    Canvas {
        id: rayCanvas
        anchors.fill: parent
        opacity: 0.04
        
        Component.onCompleted: requestPaint()
        
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            
            // Subtle light rays from top-left
            var centerX = width * 0.15;
            var centerY = height * 0.1;
            
            for (var i = 0; i < 5; i++) {
                var angle = (i / 5) * 0.8 + 0.3;
                var rayLen = Math.max(width, height) * 1.5;
                
                var gradient = ctx.createLinearGradient(
                    centerX, centerY,
                    centerX + Math.cos(angle) * rayLen,
                    centerY + Math.sin(angle) * rayLen
                );
                gradient.addColorStop(0, "rgba(110, 235, 255, 0.3)");
                gradient.addColorStop(0.3, "rgba(110, 235, 255, 0.1)");
                gradient.addColorStop(1, "transparent");
                
                ctx.beginPath();
                ctx.moveTo(centerX, centerY);
                ctx.lineTo(
                    centerX + Math.cos(angle - 0.02) * rayLen,
                    centerY + Math.sin(angle - 0.02) * rayLen
                );
                ctx.lineTo(
                    centerX + Math.cos(angle + 0.02) * rayLen,
                    centerY + Math.sin(angle + 0.02) * rayLen
                );
                ctx.closePath();
                ctx.fillStyle = gradient;
                ctx.fill();
            }
        }
    }
    
    // ========== SOFT VIGNETTE ==========
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.15) }
            GradientStop { position: 0.2; color: "transparent" }
            GradientStop { position: 0.8; color: "transparent" }
            GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.2) }
        }
    }
}
