import QtQuick
import "../style"  // Style singleton access

Item {
    id: root
    implicitWidth: 200
    implicitHeight: 200

    // Properties to animate the chart data (0.0 to 1.0)
    property real v1: 0.8 // Stability
    property real v2: 0.6 // Flick Control
    property real v3: 0.9 // Micro-Adjust
    property real v4: 0.5 // Control
    property real v5: 0.7 // Exio-Adjust

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();

            var cx = width / 2;
            var cy = height / 2;
            var radius = Math.min(width, height) / 2 - 20;

            // 1. Draw the Background Grid (The faint pentagons)
            ctx.strokeStyle = Style.surfaceHighlight;
            ctx.lineWidth = 1;
            for (var level = 1; level <= 4; level++) {
                var r = radius * (level / 4.0);
                drawPentagon(ctx, cx, cy, r);
                ctx.stroke();
            }

            // 2. Draw the Data Shape (The glowing filled area)
            ctx.beginPath();
            var angleStep = (Math.PI * 2) / 5;
            // -Math.PI/2 rotates it so the first point is at the top
            var startAngle = -Math.PI / 2;

            // Calculate points based on values
            var p1 = getPoint(cx, cy, radius * root.v1, startAngle);
            var p2 = getPoint(cx, cy, radius * root.v2, startAngle + angleStep);
            var p3 = getPoint(cx, cy, radius * root.v3, startAngle + angleStep * 2);
            var p4 = getPoint(cx, cy, radius * root.v4, startAngle + angleStep * 3);
            var p5 = getPoint(cx, cy, radius * root.v5, startAngle + angleStep * 4);

            ctx.moveTo(p1.x, p1.y);
            ctx.lineTo(p2.x, p2.y);
            ctx.lineTo(p3.x, p3.y);
            ctx.lineTo(p4.x, p4.y);
            ctx.lineTo(p5.x, p5.y);
            ctx.closePath();

            // Fill with semi-transparent Cyan
            ctx.fillStyle = "#3300E5FF";
            ctx.fill();

            // Stroke with solid Cyan
            ctx.strokeStyle = Style.primary;
            ctx.lineWidth = 2;
            ctx.stroke();

            // Draw Dots at vertices
            drawDot(ctx, p1);
            drawDot(ctx, p2);
            drawDot(ctx, p3);
            drawDot(ctx, p4);
            drawDot(ctx, p5);
        }

        // Helper: Get X,Y coordinates
        function getPoint(cx, cy, r, angle) {
            return {
                x: cx + r * Math.cos(angle),
                y: cy + r * Math.sin(angle)
            };
        }

        // Helper: Draw a single pentagon path
        function drawPentagon(ctx, cx, cy, r) {
            ctx.beginPath();
            var startAngle = -Math.PI / 2;
            var angleStep = (Math.PI * 2) / 5;
            for (var i = 0; i < 5; i++) {
                var angle = startAngle + i * angleStep;
                var px = cx + r * Math.cos(angle);
                var py = cy + r * Math.sin(angle);
                if (i === 0)
                    ctx.moveTo(px, py);
                else
                    ctx.lineTo(px, py);
            }
            ctx.closePath();
        }

        // Helper: Draw glowing dots
        function drawDot(ctx, p) {
            ctx.beginPath();
            ctx.arc(p.x, p.y, 3, 0, 2 * Math.PI);
            ctx.fillStyle = "#FFFFFF";
            ctx.fill();
        }
    }

    // Labels
    Text {
        text: "Stability"
        color: Style.textSecondary
        font.pixelSize: 10
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
    }
    Text {
        text: "Flick Control"
        color: Style.textSecondary
        font.pixelSize: 10
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 40
    }
    Text {
        text: "Micro-Adjust"
        color: Style.textSecondary
        font.pixelSize: 10
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
    }
    Text {
        text: "Control"
        color: Style.textSecondary
        font.pixelSize: 10
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
    }
    Text {
        text: "Exio-Adjust"
        color: Style.textSecondary
        font.pixelSize: 10
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 40
    }
}
