import QtQuick
import QtQuick.Layouts
import "."
import "components"
Panel {
    id: root
    property string caption: "Metric"
    property string value: "—"
    property string footnote: ""
    property string symbol: "◈"
    property real progress: 0
    property color tone: Theme.accent
    implicitHeight: 150
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 9
        RowLayout {
            Layout.fillWidth: true
            Text { text: root.caption; font.family:Theme.font; font.pixelSize:12; color:Theme.textSecondary }
            Item { Layout.fillWidth:true }
            Rectangle {
                width: 31; height: 31; radius: 9
                color: Theme.surfaceRaised
                Text { anchors.centerIn:parent; text: root.symbol; color:root.tone; font.pixelSize:18 }
            }
        }
        Text { text:root.value; font.family:Theme.font; font.weight:Font.Bold; font.pixelSize:29; color:Theme.textPrimary }
        Text { text:root.footnote; font.family:Theme.font; font.pixelSize:11; color:Theme.textLow; elide:Text.ElideRight; Layout.fillWidth:true }
        Rectangle {
            Layout.fillWidth: true
            height: 4
            radius: 2
            color: Theme.surfaceRaised
            Rectangle {
                width: parent.width*Math.max(0,Math.min(1,root.progress/100))
                height: parent.height
                radius: 2
                color: root.tone
                Behavior on width { NumberAnimation { duration: 350; easing.type: Easing.OutCubic } }
            }
        }
    }
}
