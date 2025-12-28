import QtQuick
import QtQuick.Layouts
import NeoZ // <--- CRITICAL FIX

GlassPanel {
    id: root
    property string title: ""
    property string subtitle: ""
    property string status: ""
    property color accent: Style.primary
    
    // Clickable functionality
    property bool clickable: false
    signal clicked()

    Layout.fillWidth: true
    Layout.preferredHeight: 70
    
    // GlassPanel properties
    glassOpacity: clickable && mouseArea.containsMouse ? 0.9 : 0.7
    glowColor: root.accent // Default glow color matches accent
    
    // Visual feedback for clickable pills
    Behavior on glassOpacity { NumberAnimation { duration: 150 } }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Icon Box
        Rectangle {
            width: 30; height: 30
            color: "transparent"
            border.color: root.accent
            radius: 4
            Text { 
                text: root.clickable ? "🔄" : "⚡"
                anchors.centerIn: parent
                color: root.accent 
            }
        }

        // Text Info
        Column {
            Text { text: root.title; color: Style.textPrimary; font.bold: true }
            Text { text: root.subtitle; color: Style.textSecondary; font.pixelSize: 10 }
            Text { text: root.status; color: root.accent; font.pixelSize: 10 }
        }
    }
    
    // MouseArea for click handling
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.clickable
        cursorShape: root.clickable ? Qt.PointingHandCursor : Qt.ArrowCursor
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
