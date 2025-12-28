import QtQuick
import QtQuick.Layouts
import NeoZ

// Reusable Icon + Label component for sidebar menu items
Item {
    id: root
    
    property string iconSource: ""
    property string labelText: ""
    property bool isActive: false
    property color activeColor: Style.primary
    
    signal clicked()
    
    implicitHeight: 44
    implicitWidth: parent ? parent.width : 200
    
    Rectangle {
        anchors.fill: parent
        color: root.isActive ? Qt.rgba(root.activeColor.r, root.activeColor.g, root.activeColor.b, 0.15) : "transparent"
        radius: 8
        
        // Left accent bar for active state
        Rectangle {
            visible: root.isActive
            width: 3
            height: parent.height - 10
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            color: root.activeColor
            radius: 2
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            anchors.rightMargin: 15
            spacing: 12
            
            Image {
                source: root.iconSource
                sourceSize.width: 20
                sourceSize.height: 20
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                opacity: root.isActive ? 1.0 : 0.6
                
                // Tint effect for active state (using ColorOverlay alternative)
                layer.enabled: root.isActive
                layer.effect: null // ColorOverlay not available, relying on opacity
            }
            
            Text {
                text: root.labelText
                color: root.isActive ? Style.textPrimary : Style.textSecondary
                font.pixelSize: 13
                font.weight: root.isActive ? Font.Medium : Font.Normal
                Layout.fillWidth: true
            }
        }
        
        Behavior on color { ColorAnimation { duration: 150 } }
    }
    
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: root.clicked()
        
        onEntered: {
            if (!root.isActive) {
                parent.children[0].color = Qt.rgba(1, 1, 1, 0.05)
            }
        }
        onExited: {
            if (!root.isActive) {
                parent.children[0].color = "transparent"
            }
        }
    }
}
