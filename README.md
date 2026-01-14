# M3Shapes

A Qt QML plugin providing Material Design 3 shapes with morph animation support. Port of Android's `androidx.graphics.shapes` library.

## Building

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_CXX_COMPILER=clang++
ninja
```

## Usage

### Import

```qml
import M3Shapes
```

### Basic Usage

```qml
MaterialShape {
    width: 100
    height: 100
    shape: MaterialShape.Heart
    color: "red"
}
```

### Morph Animation

When you change the `shape` property, the component automatically morphs to the new shape:

```qml
MaterialShape {
    id: myShape
    width: 200
    height: 200
    shape: MaterialShape.Circle
    color: "#6750A4"
}

Button {
    text: "Morph to Heart"
    onClicked: myShape.shape = MaterialShape.Heart
}
```

### Animation Customization

```qml
MaterialShape {
    shape: MaterialShape.Star
    color: "gold"

    // Duration in milliseconds (default: 350)
    animationDuration: 500

    // Custom easing curve
    animationEasing.type: Easing.OutBounce
}
```

### Manual Morph Control

For full control over morphing, use `fromShape`, `toShape`, and `morphProgress`:

```qml
MaterialShape {
    id: manualShape
    width: 200
    height: 200
    fromShape: MaterialShape.Circle
    toShape: MaterialShape.Heart
    morphProgress: slider.value
    color: "purple"
}

Slider {
    id: slider
    from: 0
    to: 1
    value: 0.5
}
```

## Properties

| Property            | Type       | Default     | Description                           |
| ------------------- | ---------- | ----------- | ------------------------------------- |
| `shape`             | Shape enum | Circle      | Target shape (auto-animates on change)|
| `fromShape`         | Shape enum | Circle      | Start shape for manual morphing       |
| `toShape`           | Shape enum | Circle      | End shape for manual morphing         |
| `morphProgress`     | float      | 1.0         | Morph progress 0-1 (read/write)       |
| `color`             | color      | black       | Fill color                            |
| `implicitSize`      | real       | 0           | Sets implicitWidth and implicitHeight |
| `strokeColor`       | color      | transparent | Stroke color                          |
| `strokeWidth`       | float      | 0           | Stroke width                          |
| `animationDuration` | int        | 350         | Morph duration in ms                  |
| `animationEasing`   | easing     | spring-like | Animation easing curve                |

## Available Shapes

```
MaterialShape.Circle
MaterialShape.Square
MaterialShape.Slanted
MaterialShape.Arch
MaterialShape.Fan
MaterialShape.Arrow
MaterialShape.SemiCircle
MaterialShape.Oval
MaterialShape.Pill
MaterialShape.Triangle
MaterialShape.Diamond
MaterialShape.ClamShell
MaterialShape.Pentagon
MaterialShape.Gem
MaterialShape.Sunny
MaterialShape.VerySunny
MaterialShape.Cookie4Sided
MaterialShape.Cookie6Sided
MaterialShape.Cookie7Sided
MaterialShape.Cookie9Sided
MaterialShape.Cookie12Sided
MaterialShape.Ghostish
MaterialShape.Clover4Leaf
MaterialShape.Clover8Leaf
MaterialShape.Burst
MaterialShape.SoftBurst
MaterialShape.Boom
MaterialShape.SoftBoom
MaterialShape.Flower
MaterialShape.Puffy
MaterialShape.PuffyDiamond
MaterialShape.PixelCircle
MaterialShape.PixelTriangle
MaterialShape.Bun
MaterialShape.Heart
```

## Example

```qml
import QtQuick
import QtQuick.Controls
import M3Shapes

ApplicationWindow {
    visible: true
    width: 400
    height: 400

    MaterialShape {
        id: shape
        anchors.centerIn: parent
        width: 200
        height: 200
        shape: MaterialShape.Circle
        color: Qt.hsla(0.8, 0.7, 0.6, 1.0)
    }

    Row {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10

        Button {
            text: "Circle"
            onClicked: shape.shape = MaterialShape.Circle
        }
        Button {
            text: "Heart"
            onClicked: shape.shape = MaterialShape.Heart
        }
        Button {
            text: "Star"
            onClicked: shape.shape = MaterialShape.Sunny
        }
    }
}
```
