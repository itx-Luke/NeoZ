import QtQuick
import NeoZ

// NeoZText - Theme-aware text component with type variants
Text {
    id: control

    // Theme properties
    property color normalColor: Style.textPrimary
    property color secondaryColor: Style.textSecondary
    property color accentColor: Style.accentColor
    property int textType: 0 // 0=Normal, 1=Secondary, 2=Accent, 3=Title

    font: Style.bodyFont
    color: getTextColor()

    Behavior on color {
        ColorAnimation {
            duration: Style.motionDuration * Style.animationSpeed
        }
    }

    function getTextColor() {
        switch (textType) {
        case 1:
            return secondaryColor;
        case 2:
            return accentColor;
        case 3:
            return normalColor;
        default:
            return normalColor;
        }
    }

    // Helper functions for different text types
    function setAsSecondary() {
        textType = 1;
    }
    function setAsAccent() {
        textType = 2;
    }
    function setAsTitle() {
        textType = 3;
        font.bold = true;
        font.pixelSize = Style.bodyFont.pixelSize + 4;
    }
}
