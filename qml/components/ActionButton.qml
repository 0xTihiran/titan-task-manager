import QtQuick
import ".."
Rectangle {
    id: root
    signal clicked()
    property string label: "Action"
    property string symbol: "↻"
    property bool highlighted: false
    implicitWidth: row.implicitWidth+24
    implicitHeight: 36
    radius: 9
    color: mouse.containsMouse ? Theme.surfaceHover : (highlighted ? Theme.accentSoft : Theme.surfaceRaised)
    border.color: highlighted ? Theme.accent : Theme.border
    Behavior on color { ColorAnimation { duration: 110 } }
    Row {
        id: row
        anchors.centerIn: parent
        spacing: 7
        Text { text: root.symbol; color: highlighted ? Theme.accent : Theme.textSecondary; font.pixelSize: 17; anchors.verticalCenter: parent.verticalCenter }
        Text { text: root.label; color: Theme.textPrimary; font.family: Theme.font; font.pixelSize: 12; font.weight: Font.DemiBold; anchors.verticalCenter: parent.verticalCenter }
    }
    MouseArea { id:mouse; anchors.fill:parent; hoverEnabled:true; cursorShape:Qt.PointingHandCursor; onClicked: root.clicked() }
}
