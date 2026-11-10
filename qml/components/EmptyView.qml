import QtQuick
import QtQuick.Layouts
import ".."
Item {
    property string symbol: "◌"
    property string heading: "No data available"
    property string detail: "Waiting for a supported source."
    implicitHeight: 190
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 10
        Text { Layout.alignment: Qt.AlignHCenter; text: symbol; font.pixelSize: 33; color: Theme.textLow }
        Text { Layout.alignment: Qt.AlignHCenter; text: heading; font.family:Theme.font; font.pixelSize: 17; font.weight:Font.DemiBold; color:Theme.textPrimary }
        Text { Layout.alignment: Qt.AlignHCenter; text: detail; wrapMode:Text.WordWrap; horizontalAlignment:Text.AlignHCenter; Layout.maximumWidth: 520; font.family:Theme.font; font.pixelSize: 12; color:Theme.textSecondary }
    }
}
