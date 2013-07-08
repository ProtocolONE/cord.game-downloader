import QtQuick 1.1
import "Tooltip/Tooltip.js" as Tooltip

MouseArea {
    id: mouser

    property alias cursor: cursorArea.cursor
    property string toolTip
    property string tooltipPosition
    property bool tooltipGlueCenter: false

    hoverEnabled: true
    Component.onCompleted: Tooltip.track(mouser)
    Component.onDestruction: Tooltip.release(mouser)

    onMousePositionChanged: {
       // console.log(mainWindowRectanglw.childAt(mouseX, mouseY));
    }

    CursorShapeArea {
        id: cursorArea

        anchors.fill: parent
        visible: mouser.containsMouse
    }
}
