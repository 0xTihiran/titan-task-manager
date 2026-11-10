import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."
import "components"
ApplicationWindow {
    id: appWindow
    width: 1200
    height: 750
    minimumWidth: 950
    minimumHeight: 600
    visible: true
    title: "TITAN Task Manager"
    color: Theme.canvas
    font.family: Theme.font
    property int selectedPage: 0
    readonly property var pageTitles: ["Overview","Workloads","Processes","Workspaces","THM Decisions","Slices"]
    readonly property var pageSymbols: ["▦","◈","≡","▤","◷","▧"]

    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            color: Theme.sidebar
            Layout.preferredWidth: 218
            Layout.fillHeight: true
            border.color: Theme.border
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 7
                RowLayout {
                    Layout.topMargin: 10
                    Layout.bottomMargin: 37
                    spacing: 11
                    Rectangle {
                        width: 39; height: 39; radius: 11
                        color: Theme.accent
                        Text { anchors.centerIn:parent; text: "T"; font.pixelSize:22; font.weight:Font.Black; color:Theme.textPrimary }
                    }
                    ColumnLayout {
                        spacing: 0
                        Text { text: "TITAN"; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:20; font.weight:Font.Black; font.letterSpacing:1.3 }
                        Text { text: "TASK MANAGER"; color:Theme.textLow; font.family:Theme.font; font.pixelSize:9; font.letterSpacing:1.5 }
                    }
                }
                Text { text:"WORKSPACE"; color:Theme.textLow; font.family:Theme.font; font.pixelSize:10; font.letterSpacing:1.3; Layout.leftMargin:13; Layout.bottomMargin:7 }
                Repeater {
                    model: appWindow.pageTitles
                    delegate: Rectangle {
                        required property int index
                        required property string modelData
                        Layout.fillWidth:true
                        Layout.preferredHeight:45
                        radius:10
                        color: appWindow.selectedPage===index ? Theme.accentSoft : (navMouse.containsMouse ? Theme.surfaceHover : "transparent")
                        border.color: appWindow.selectedPage===index ? Theme.accent : "transparent"
                        Behavior on color { ColorAnimation { duration:110 } }
                        Rectangle { width:3; height:19; radius:2; anchors.left:parent.left; anchors.verticalCenter:parent.verticalCenter; color:Theme.accent; visible:appWindow.selectedPage===index }
                        Row {
                            anchors.verticalCenter:parent.verticalCenter
                            anchors.left:parent.left
                            anchors.leftMargin:15
                            spacing:13
                            Text { text: appWindow.pageSymbols[index]; font.pixelSize:19; color:appWindow.selectedPage===index ? Theme.accent : Theme.textSecondary; anchors.verticalCenter:parent.verticalCenter }
                            Text { text:modelData; font.family:Theme.font; font.pixelSize:13; font.weight:appWindow.selectedPage===index ? Font.DemiBold : Font.Normal; color: appWindow.selectedPage===index ? Theme.textPrimary : Theme.textSecondary; anchors.verticalCenter:parent.verticalCenter }
                        }
                        MouseArea { id:navMouse; anchors.fill:parent; hoverEnabled:true; cursorShape:Qt.PointingHandCursor; onClicked: appWindow.selectedPage=index }
                    }
                }
                Item { Layout.fillHeight:true }
                Rectangle { Layout.fillWidth:true; height:1; color:Theme.border; Layout.bottomMargin:12 }
                RowLayout {
                    spacing:9
                    Rectangle { width:8; height:8; radius:4; color:thm.available ? Theme.success : Theme.warning }
                    Text { text:thm.available ? "THM connected" : "THM unavailable"; color:Theme.textSecondary; font.family:Theme.font; font.pixelSize:12 }
                }
                Text { text:"THM v"+(thm.version || "—")+"  •  Dev preview 0.4"; font.family:Theme.font; color:Theme.textLow; font.pixelSize:10; Layout.bottomMargin:11 }
            }
        }

        ColumnLayout {
            Layout.fillWidth:true
            Layout.fillHeight:true
            spacing:0
            Rectangle {
                Layout.fillWidth:true
                Layout.preferredHeight:72
                color:Theme.canvas
                border.color:Theme.border
                RowLayout {
                    anchors.fill:parent
                    anchors.leftMargin:27
                    anchors.rightMargin:27
                    spacing:15
                    ColumnLayout {
                        spacing:2
                        Text { text: appWindow.pageTitles[appWindow.selectedPage]; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:20; font.weight:Font.Bold }
                        Text { text:"ARCHTITAN OS   /   SYSTEM MONITOR"; color:Theme.textLow; font.family:Theme.font; font.pixelSize:10; font.letterSpacing:1.0 }
                    }
                    Item { Layout.fillWidth:true }
                    Rectangle {
                        Layout.preferredWidth:250
                        Layout.preferredHeight:37
                        radius:9
                        color:Theme.surface
                        border.color:Theme.border
                        RowLayout {
                            anchors.fill:parent
                            anchors.leftMargin:12
                            anchors.rightMargin:10
                            Text { text:"⌕"; color:Theme.textSecondary; font.pixelSize:22 }
                            TextField {
                                id:search
                                Layout.fillWidth:true
                                placeholderText:"Search workloads or processes"
                                font.family:Theme.font
                                font.pixelSize:12
                                color:Theme.textPrimary
                                placeholderTextColor:Theme.textLow
                                background:Item {}
                                leftPadding:0
                                rightPadding:0
                            }
                        }
                    }
                    ActionButton { label:"Refresh"; symbol:"↻"; onClicked:{ thm.refresh(); sys.refresh(); proc.refresh(); slices.refresh(); decisions.refresh(); hyprland.reconnect() } }
                }
            }
            StackLayout {
                Layout.fillWidth:true
                Layout.fillHeight:true
                Layout.leftMargin:26
                Layout.rightMargin:26
                Layout.topMargin:23
                Layout.bottomMargin:20
                currentIndex:appWindow.selectedPage
                OverviewPage { }
                WorkloadsPage { searchText:search.text }
                ProcessesPage { searchText:search.text }
                WorkspacesPage { }
                DecisionsPage { }
                SlicesPage { }
            }
        }
    }
}
