import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import M3Shapes

ApplicationWindow {
    id: window
    visible: true
    width: 1024
    height: 768
    title: "Material Shape Gallery"
    color: "#1a1a2e"

    readonly property var shapeNames: [
        "Circle", "Square", "Slanted", "Arch", "Fan", "Arrow", "SemiCircle",
        "Oval", "Pill", "Triangle", "Diamond", "ClamShell", "Pentagon", "Gem",
        "Sunny", "VerySunny", "Cookie4", "Cookie6", "Cookie7", "Cookie9",
        "Cookie12", "Ghostish", "Clover4", "Clover8", "Burst", "SoftBurst",
        "Boom", "SoftBoom", "Flower", "Puffy", "PuffyDiamond", "PixelCircle",
        "PixelTriangle", "Bun", "Heart"
    ]

    readonly property var shapeEnums: [
        MaterialShape.Circle, MaterialShape.Square, MaterialShape.Slanted,
        MaterialShape.Arch, MaterialShape.Fan, MaterialShape.Arrow,
        MaterialShape.SemiCircle, MaterialShape.Oval, MaterialShape.Pill,
        MaterialShape.Triangle, MaterialShape.Diamond, MaterialShape.ClamShell,
        MaterialShape.Pentagon, MaterialShape.Gem, MaterialShape.Sunny,
        MaterialShape.VerySunny, MaterialShape.Cookie4Sided, MaterialShape.Cookie6Sided,
        MaterialShape.Cookie7Sided, MaterialShape.Cookie9Sided, MaterialShape.Cookie12Sided,
        MaterialShape.Ghostish, MaterialShape.Clover4Leaf, MaterialShape.Clover8Leaf,
        MaterialShape.Burst, MaterialShape.SoftBurst, MaterialShape.Boom,
        MaterialShape.SoftBoom, MaterialShape.Flower, MaterialShape.Puffy,
        MaterialShape.PuffyDiamond, MaterialShape.PixelCircle, MaterialShape.PixelTriangle,
        MaterialShape.Bun, MaterialShape.Heart
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Main shape display
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 300
            color: "#16213e"
            radius: 12

            MaterialShape {
                id: mainShape
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height) * 0.8
                height: width
                shape: MaterialShape.Circle
                fillColor: Qt.hsla(0.6, 0.7, 0.6, 1.0)
                // Uses Material 3 default animation (350ms, EmphasizedDecelerate)
            }

            Text {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 10
                text: window.shapeNames[mainShape.shape] + (mainShape.morphProgress < 1.0 ? " (" + Math.round(mainShape.morphProgress * 100) + "%)" : "")
                color: "#e0e0e0"
                font.pixelSize: 16
            }
        }

        // Shape selector grid
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#16213e"
            radius: 12

            ScrollView {
                id: scrollView
                anchors.fill: parent
                anchors.margins: 10
                clip: true

                GridLayout {
                    width: scrollView.width - 20
                    columns: Math.max(1, Math.floor((scrollView.width - 20) / 90))
                    columnSpacing: 8
                    rowSpacing: 8

                    Repeater {
                        model: 35

                        Rectangle {
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 90
                            color: mainShape.shape === window.shapeEnums[index] ? "#2d4a7c" : "#1a1a2e"
                            radius: 6
                            border.color: mainShape.shape === window.shapeEnums[index] ? "#4a90d9" : "transparent"
                            border.width: 2

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 5
                                spacing: 2

                                MaterialShape {
                                    Layout.alignment: Qt.AlignHCenter
                                    Layout.preferredWidth: 50
                                    Layout.preferredHeight: 50
                                    shape: window.shapeEnums[index]
                                    fillColor: Qt.hsla(index * 0.03, 0.7, 0.6, 1.0)
                                    animationDuration: 0
                                }

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    Layout.fillWidth: true
                                    text: window.shapeNames[index]
                                    color: "#e0e0e0"
                                    font.pixelSize: 9
                                    horizontalAlignment: Text.AlignHCenter
                                    elide: Text.ElideRight
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    mainShape.shape = window.shapeEnums[index]
                                    mainShape.fillColor = Qt.hsla(index * 0.03, 0.7, 0.6, 1.0)
                                }
                            }
                        }
                    }
                }
            }
        }

        // Controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Label {
                text: "Morph Duration:"
                color: "#e0e0e0"
            }

            Slider {
                id: durationSlider
                Layout.fillWidth: true
                from: 100
                to: 2000
                value: 350  // Material 3 default
                onValueChanged: mainShape.animationDuration = value
            }

            Label {
                text: Math.round(durationSlider.value) + "ms"
                color: "#e0e0e0"
                Layout.preferredWidth: 60
            }

            Button {
                text: "Random"
                onClicked: {
                    var randomIndex = Math.floor(Math.random() * 35)
                    mainShape.shape = window.shapeEnums[randomIndex]
                    mainShape.fillColor = Qt.hsla(randomIndex * 0.03, 0.7, 0.6, 1.0)
                }
            }
        }
    }
}
