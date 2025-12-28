import QtQuick
import "../style"

// RadialSlider - Premium circular slider with velocity falloff visualization
// Used for Slow Zone control in Monitor Optimization panel
Item {
    id: root
    
    property real value: 50
    property real from: 1
    property real to: 100
    property string label: "Slow Zone"
    property color accentColor: "#6EEBFF"
    property color secondaryColor: "#9A8CFF"
    
    width: 160
    height: 160
    
    // Outer glow ring
    Rectangle {
        anchors.fill: parent
        anchors.margins: -6
        radius: width / 2
        color: "transparent"
        border.color: accentColor
        border.width: 2
        opacity: 0.12
        
        // Pulsing
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.12; to: 0.08; duration: 2000; easing.type: Easing.InOutSine }
            NumberAnimation { from: 0.08; to: 0.12; duration: 2000; easing.type: Easing.InOutSine }
        }
    }
    
    // Main ring background
    Rectangle {
        id: ringBg
        anchors.fill: parent
        radius: width / 2
        color: Qt.rgba(0.05, 0.06, 0.1, 0.9)
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1
        
        // Inner glass shine
        Rectangle {
            anchors.centerIn: parent
            width: parent.width - 8
            height: parent.height - 8
            radius: width / 2
            color: "transparent"
            border.color: Qt.rgba(1, 1, 1, 0.05)
            border.width: 1
        }
    }
    
    // Inner darkened zone (represents slow zone extent)
    Rectangle {
        id: innerZone
        anchors.centerIn: parent
        width: parent.width * 0.45 * (value / to)
        height: width
        radius: width / 2
        color: Qt.rgba(0, 0, 0, 0.5)
        border.color: Qt.rgba(1, 1, 1, 0.05)
        border.width: 1
        
        Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
    }
    
    // Velocity falloff curve visualization using Canvas
    Canvas {
        id: falloffCanvas
        anchors.fill: parent
        
        property real progress: (value - from) / (to - from)
        
        onProgressChanged: requestPaint()
        Component.onCompleted: requestPaint()
        
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            
            var centerX = width / 2;
            var centerY = height / 2;
            var outerRadius = width / 2 - 12;
            var innerRadius = outerRadius * progress * 0.45;
            
            // Draw velocity falloff rings (5 concentric rings)
            var rings = 5;
            for (var i = 1; i <= rings; i++) {
                var r = innerRadius + (outerRadius - innerRadius) * (i / rings);
                var alpha = 0.06 + (i / rings) * 0.12;
                
                ctx.beginPath();
                ctx.arc(centerX, centerY, r, 0, Math.PI * 2);
                ctx.strokeStyle = "rgba(110, 235, 255, " + alpha + ")";
                ctx.lineWidth = 1;
                ctx.stroke();
            }
            
            // Draw radial gradient lines
            var lines = 12;
            for (var j = 0; j < lines; j++) {
                var angle = (j / lines) * Math.PI * 2;
                var lineAlpha = 0.08;
                
                ctx.beginPath();
                ctx.moveTo(
                    centerX + Math.cos(angle) * innerRadius,
                    centerY + Math.sin(angle) * innerRadius
                );
                ctx.lineTo(
                    centerX + Math.cos(angle) * outerRadius,
                    centerY + Math.sin(angle) * outerRadius
                );
                ctx.strokeStyle = "rgba(154, 140, 255, " + lineAlpha + ")";
                ctx.lineWidth = 1;
                ctx.stroke();
            }
        }
    }
    
    // Arc progress indicator using Canvas
    Canvas {
        id: arcCanvas
        anchors.fill: parent
        
        property real progress: (value - from) / (to - from)
        
        onProgressChanged: requestPaint()
        Component.onCompleted: requestPaint()
        
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            
            var centerX = width / 2;
            var centerY = height / 2;
            var radius = width / 2 - 8;
            
            // Full arc background
            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
            ctx.strokeStyle = "rgba(255, 255, 255, 0.06)";
            ctx.lineWidth = 5;
            ctx.stroke();
            
            // Progress arc with gradient
            var endAngle = -Math.PI / 2 + progress * Math.PI * 2;
            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, -Math.PI / 2, endAngle);
            
            var gradient = ctx.createLinearGradient(0, 0, width, height);
            gradient.addColorStop(0, "#6EEBFF");
            gradient.addColorStop(0.5, "#9A8CFF");
            gradient.addColorStop(1, "#6EEBFF");
            ctx.strokeStyle = gradient;
            ctx.lineWidth = 5;
            ctx.lineCap = "round";
            ctx.stroke();
        }
    }
    
    // Thumb on the arc
    Rectangle {
        id: thumb
        width: 16
        height: 16
        radius: 8
        color: accentColor
        border.color: Qt.rgba(1, 1, 1, 0.5)
        border.width: 1
        
        property real angle: -Math.PI / 2 + ((value - from) / (to - from)) * Math.PI * 2
        property real orbitRadius: root.width / 2 - 8
        
        x: root.width / 2 + Math.cos(angle) * orbitRadius - width / 2
        y: root.height / 2 + Math.sin(angle) * orbitRadius - height / 2
        
        Behavior on x { NumberAnimation { duration: 60; easing.type: Easing.OutCubic } }
        Behavior on y { NumberAnimation { duration: 60; easing.type: Easing.OutCubic } }
        
        // Inner highlight
        Rectangle {
            anchors.centerIn: parent
            width: parent.width - 4
            height: parent.height - 4
            radius: width / 2
            color: Qt.rgba(1, 1, 1, 0.3)
        }
        
        // Outer glow
        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 12
            height: parent.height + 12
            radius: width / 2
            color: "transparent"
            border.color: parent.color
            border.width: 2
            opacity: dragArea.pressed ? 0.6 : 0.25
            
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
        
        scale: dragArea.pressed ? 1.2 : 1.0
        Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
    }
    
    // Center value display
    Column {
        anchors.centerIn: parent
        spacing: 2
        
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Math.round(value) + "%"
            color: "#E6EAF0"
            font.pixelSize: 24
            font.bold: true
        }
        
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: label
            color: "#9AA4B2"
            font.pixelSize: 10
            font.letterSpacing: 1
        }
    }
    
    // Drag interaction
    MouseArea {
        id: dragArea
        anchors.fill: parent
        
        onPressed: function(mouse) { updateFromMouse(mouse) }
        onPositionChanged: function(mouse) {
            if (pressed) updateFromMouse(mouse)
        }
        
        function updateFromMouse(mouse) {
            var centerX = width / 2;
            var centerY = height / 2;
            var dx = mouse.x - centerX;
            var dy = mouse.y - centerY;
            
            var angle = Math.atan2(dy, dx);
            // Normalize angle to 0-1 range starting from top
            var progress = (angle + Math.PI / 2) / (Math.PI * 2);
            if (progress < 0) progress += 1;
            
            var newValue = from + progress * (to - from);
            value = Math.max(from, Math.min(to, newValue));
        }
    }
}
