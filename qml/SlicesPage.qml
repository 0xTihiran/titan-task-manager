import QtQuick
import QtQuick.Layouts
import "."
import "components"
Item {
    id:page
    Flickable {
        anchors.fill:parent
        contentWidth:width
        contentHeight:content.implicitHeight+24
        clip:true
        ColumnLayout {
            id:content
            width:parent.width-4
            spacing:16
            RowLayout {
                Layout.fillWidth:true
                ColumnLayout {
                    spacing:4
                    Text { text:"Resource slices"; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:23; font.weight:Font.Bold }
                    Text { text:"Live cgroup v2 metrics · read only"; color:Theme.textSecondary; font.family:Theme.font; font.pixelSize:12 }
                }
                Item { Layout.fillWidth:true }
                StatusChip { label:"No modifications allowed"; tone:Theme.accent }
            }
            Repeater {
                model:slices.slices
                delegate:Panel {
                    required property var modelData
                    Layout.fillWidth:true
                    Layout.preferredHeight:153
                    ColumnLayout {
                        anchors.fill:parent
                        anchors.margins:17
                        spacing:14
                        RowLayout {
                            Text { text:modelData.name; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:16; font.weight:Font.DemiBold }
                            Item { Layout.fillWidth:true }
                            StatusChip { label:modelData.available ? "Available" : "Missing"; tone:modelData.available ? Theme.success : Theme.warning }
                            StatusChip { visible:modelData.available && modelData.frozen==="1"; label:"Frozen"; tone:Theme.danger }
                        }
                        RowLayout {
                            spacing:12
                            Repeater {
                                model:[
                                    {label:"CPU WEIGHT",value:modelData.cpuWeight},
                                    {label:"CPU MAX",value:modelData.cpuMax},
                                    {label:"MEMORY CURRENT",value:modelData.memoryCurrent},
                                    {label:"MEMORY HIGH",value:modelData.memoryHigh},
                                    {label:"PIDS",value:modelData.pidsCurrent},
                                    {label:"FREEZE",value:modelData.frozen}
                                ]
                                delegate:ColumnLayout {
                                    required property var modelData
                                    Layout.fillWidth:true
                                    spacing:7
                                    Text { text:modelData.label; font.family:Theme.font; font.pixelSize:9; color:Theme.textLow }
                                    Text { text:String(modelData.value); font.family:Theme.font; font.pixelSize:12; color:Theme.textPrimary; elide:Text.ElideRight; Layout.maximumWidth:130 }
                                }
                            }
                        }
                        Text { text:"Population: "+modelData.populated+"   •   kernel sysfs read-only snapshot"; font.family:Theme.font; font.pixelSize:10; color:Theme.textLow }
                    }
                }
            }
        }
    }
}
