import QtQuick
import ".."
Rectangle {
    id: root
    property string label: "Status"
    property color tone: Theme.success
    property bool filled: false
    implicitWidth: body.implicitWidth+24
    implicitHeight: 28
    radius: height/2
    color: filled ? Theme.accentSoft : Theme.surfaceRaised
    border.color: Theme.border
    Row {
        id: body
        anchors.centerIn: parent
        spacing: 7
        Rectangle { width: 7; height: 7; radius: 4; color: root.tone; anchors.verticalCenter: parent.verticalCenter }
        Text { text: root.label; color: Theme.textSecondary; font.family: Theme.font; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
    }
}
