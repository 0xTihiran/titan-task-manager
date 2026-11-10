import QtQuick
import QtQuick.Layouts
import "."
import "components"

Item {
    id: page
    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 5
                Text { text: "THM decisions"; font.family: Theme.font; font.pixelSize: 23; font.weight: Font.Bold; color: Theme.textPrimary }
                Text { text: "Historical decisions from the THM read-only UNIX socket"; font.family: Theme.font; font.pixelSize: 12; color: Theme.textSecondary }
            }
            Item { Layout.fillWidth: true }
            StatusChip { label: decisions.connected ? "Socket connected" : "Socket unavailable"; tone: decisions.connected ? Theme.success : Theme.warning }
            ActionButton { label: "Refresh decisions"; symbol: "↻"; onClicked: decisions.refresh() }
        }
        Panel {
            Layout.fillWidth: true
            Layout.preferredHeight: 145
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12
                SectionTitle { Layout.fillWidth: true; title: "Latest published action"; hint: "THM telemetry last_action" }
                Text {
                    text: thm.lastDecision || "No recent action has been published in the telemetry snapshot."
                    font.family: Theme.font; font.pixelSize: 14; color: Theme.textPrimary
                    wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Text { text: "The historical view is inspection-only. It does not send mutation commands."; color: Theme.textLow; font.pixelSize: 11 }
            }
        }
        Panel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 17
                spacing: 12
                RowLayout {
                    SectionTitle { title: "Recent THM decisions"; hint: "decisions 50" }
                    Item { Layout.fillWidth: true }
                    Text { text: decisions.connected ? (String(decisions.lines.length)+" lines") : "Offline"; font.pixelSize: 11; color: Theme.textLow }
                }
                Text {
                    visible: !decisions.connected
                    text: "THM socket not available: " + decisions.errorMessage
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    color: Theme.warning; font.family: Theme.font; font.pixelSize: 12
                }
                Text {
                    visible: decisions.connected && decisions.lines.length===0
                    text: "THM returned no historical decisions."
                    color: Theme.textSecondary; font.family: Theme.font; font.pixelSize: 12
                }
                ListView {
                    id: logs
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    visible: decisions.connected && decisions.lines.length>0
                    spacing: 8
                    model: decisions.lines
                    delegate: Rectangle {
                        required property string modelData
                        required property int index
                        width: logs.width
                        height: Math.max(44, line.implicitHeight+22)
                        color: index%2===0 ? Theme.surfaceRaised : Theme.surface
                        radius: 8
                        Text {
                            id: line
                            anchors.fill: parent
                            anchors.margins: 11
                            text: modelData
                            color: Theme.textSecondary
                            font.family: "monospace"
                            font.pixelSize: 12
                            wrapMode: Text.WrapAnywhere
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
        Text { text: "Socket: "+decisions.socketPath+"   •   Automatic refresh every 5s"; font.family: Theme.font; font.pixelSize: 10; color: Theme.textLow }
    }
}
