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
                implicitSize: Math.min(parent.width, parent.height) * 0.8
                shape: MaterialShape.Circle
                color: Qt.hsla(0.6, 0.7, 0.6, 1.0)
                rotation: autoRotateCheckbox.checked ? continuousRotation : rotationSlider.value

                property real continuousRotation: 0

                NumberAnimation on continuousRotation {
                    running: autoRotateCheckbox.checked
                    from: 0
                    to: 360
                    duration: 10000
                    loops: Animation.Infinite
                }

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: "#ff5555"
                    border.width: 2
                    visible: showBoundsCheckbox.checked
                }
            }

            Timer {
                id: autoMorphTimer
                property int shapeIndex: 0
                interval: mainShape.animationDuration + 500
                running: autoMorphCheckbox.checked
                repeat: true
                onTriggered: {
                    shapeIndex = (shapeIndex + 1) % 35
                    mainShape.shape = window.shapeEnums[shapeIndex]
                    mainShape.color = Qt.hsla(shapeIndex * 0.03, 0.7, 0.6, 1.0)
                }
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
                                    implicitSize: 50
                                    shape: window.shapeEnums[index]
                                    color: Qt.hsla(index * 0.03, 0.7, 0.6, 1.0)
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
                                    mainShape.color = Qt.hsla(index * 0.03, 0.7, 0.6, 1.0)
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
                text: "Duration:"
                color: "#e0e0e0"
            }

            Slider {
                id: durationSlider
                Layout.fillWidth: true
                from: 100
                to: 2000
                value: 350
                onValueChanged: mainShape.animationDuration = value
            }

            Label {
                text: Math.round(durationSlider.value) + "ms"
                color: "#e0e0e0"
                Layout.preferredWidth: 60
            }

            Label {
                text: "Rotation:"
                color: "#e0e0e0"
            }

            Slider {
                id: rotationSlider
                Layout.fillWidth: true
                from: 0
                to: 360
                value: 0
            }

            Label {
                text: Math.round(rotationSlider.value) + "°"
                color: "#e0e0e0"
                Layout.preferredWidth: 40
            }

            CheckBox {
                id: showBoundsCheckbox
                text: "Bounds"
                checked: false
                contentItem: Text {
                    text: showBoundsCheckbox.text
                    color: "#e0e0e0"
                    leftPadding: showBoundsCheckbox.indicator.width + 4
                    verticalAlignment: Text.AlignVCenter
                }
            }

            CheckBox {
                id: autoRotateCheckbox
                text: "Spin"
                checked: false
                contentItem: Text {
                    text: autoRotateCheckbox.text
                    color: "#e0e0e0"
                    leftPadding: autoRotateCheckbox.indicator.width + 4
                    verticalAlignment: Text.AlignVCenter
                }
            }

            CheckBox {
                id: autoMorphCheckbox
                text: "Auto"
                checked: false
                contentItem: Text {
                    text: autoMorphCheckbox.text
                    color: "#e0e0e0"
                    leftPadding: autoMorphCheckbox.indicator.width + 4
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: "Random"
                onClicked: {
                    var randomIndex = Math.floor(Math.random() * 35)
                    mainShape.shape = window.shapeEnums[randomIndex]
                    mainShape.color = Qt.hsla(randomIndex * 0.03, 0.7, 0.6, 1.0)
                }
            }
        }

        // Manual morph controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Label {
                text: "From:"
                color: "#e0e0e0"
            }

            ComboBox {
                id: fromShapeCombo
                Layout.preferredWidth: 120
                model: window.shapeNames
                currentIndex: 0
                onCurrentIndexChanged: manualShape.fromShape = window.shapeEnums[currentIndex]
            }

            Label {
                text: "To:"
                color: "#e0e0e0"
            }

            ComboBox {
                id: toShapeCombo
                Layout.preferredWidth: 120
                model: window.shapeNames
                currentIndex: 34
                onCurrentIndexChanged: manualShape.toShape = window.shapeEnums[currentIndex]
            }

            Label {
                text: "Progress:"
                color: "#e0e0e0"
            }

            Slider {
                id: progressSlider
                Layout.fillWidth: true
                from: 0
                to: 1
                value: 0.5
                onValueChanged: manualShape.morphProgress = value
            }

            Label {
                text: Math.round(progressSlider.value * 100) + "%"
                color: "#e0e0e0"
                Layout.preferredWidth: 40
            }

            MaterialShape {
                id: manualShape
                implicitSize: 50
                fromShape: window.shapeEnums[fromShapeCombo.currentIndex]
                toShape: window.shapeEnums[toShapeCombo.currentIndex]
                morphProgress: progressSlider.value
                color: Qt.hsla(0.8, 0.7, 0.6, 1.0)
            }
        }
    }
}
