import QtQuick
import QtQuick.Layouts
import ".."
RowLayout {
    property string title: "Section"
    property string hint: ""
    spacing: 12
    Text { text: parent.title; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize: 17; font.weight:Font.DemiBold }
    Text { text: parent.hint; color:Theme.textLow; font.family:Theme.font; font.pixelSize:11 }
    Item { Layout.fillWidth:true }
}
