#pragma once

#include "../morph/Morph.hpp"
#include "../shapes/MaterialShapes.hpp"
#include <QEasingCurve>
#include <QPainterPath>
#include <QPainter>
#include <QPropertyAnimation>
#include <QQuickPaintedItem>
#include <memory>

/**
 * MaterialShapeItem is a unified QML component for Material Design shapes.
 * Setting the shape property automatically morphs to the new shape.
 *
 * Configure animation via properties:
 *   MaterialShape {
 *       shape: MaterialShape.Heart
 *       animationDuration: 1000
 *       animationEasing: Easing.OutBounce
 *   }
 */
class MaterialShapeItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(MaterialShape)

public:
    enum Shape {
        Circle = 0,
        Square,
        Slanted,
        Arch,
        Fan,
        Arrow,
        SemiCircle,
        Oval,
        Pill,
        Triangle,
        Diamond,
        ClamShell,
        Pentagon,
        Gem,
        Sunny,
        VerySunny,
        Cookie4Sided,
        Cookie6Sided,
        Cookie7Sided,
        Cookie9Sided,
        Cookie12Sided,
        Ghostish,
        Clover4Leaf,
        Clover8Leaf,
        Burst,
        SoftBurst,
        Boom,
        SoftBoom,
        Flower,
        Puffy,
        PuffyDiamond,
        PixelCircle,
        PixelTriangle,
        Bun,
        Heart
    };
    Q_ENUM(Shape)

    Q_PROPERTY(Shape shape READ shape WRITE setShape NOTIFY shapeChanged)
    Q_PROPERTY(Shape fromShape READ fromShape WRITE setFromShape NOTIFY fromShapeChanged)
    Q_PROPERTY(Shape toShape READ toShape WRITE setToShape NOTIFY toShapeChanged)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE
            setAnimationDuration NOTIFY animationDurationChanged)
    Q_PROPERTY(QEasingCurve animationEasing READ animationEasing WRITE
            setAnimationEasing NOTIFY animationEasingChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal implicitSize READ implicitSize WRITE setImplicitSize NOTIFY
            implicitSizeChanged)
    Q_PROPERTY(QColor strokeColor READ strokeColor WRITE setStrokeColor NOTIFY
            strokeColorChanged)
    Q_PROPERTY(float strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY
            strokeWidthChanged)
    Q_PROPERTY(float shapeRotation READ shapeRotation WRITE setShapeRotation
            NOTIFY shapeRotationChanged)
    Q_PROPERTY(float morphProgress READ morphProgress WRITE setMorphProgress
            NOTIFY morphProgressChanged)

    explicit MaterialShapeItem(QQuickItem* parent = nullptr);

    [[nodiscard]] Shape shape() const { return m_targetShape; }

    void setShape(Shape shape);

    [[nodiscard]] Shape fromShape() const { return m_fromShape; }

    void setFromShape(Shape shape);

    [[nodiscard]] Shape toShape() const { return m_toShape; }

    void setToShape(Shape shape);

    [[nodiscard]] int animationDuration() const { return m_animationDuration; }

    void setAnimationDuration(int duration);

    [[nodiscard]] QEasingCurve animationEasing() const {
        return m_animationEasing;
    }

    void setAnimationEasing(const QEasingCurve& easing);

    [[nodiscard]] QColor color() const { return m_color; }

    void setColor(const QColor& color);

    [[nodiscard]] qreal implicitSize() const { return m_implicitSize; }

    void setImplicitSize(qreal size);

    [[nodiscard]] QColor strokeColor() const { return m_strokeColor; }

    void setStrokeColor(const QColor& color);

    [[nodiscard]] float strokeWidth() const { return m_strokeWidth; }

    void setStrokeWidth(float width);

    [[nodiscard]] float shapeRotation() const { return m_shapeRotation; }

    void setShapeRotation(float rotation);

    [[nodiscard]] float morphProgress() const { return m_morphProgress; }

    void setMorphProgress(float progress);

signals:
    void shapeChanged();
    void fromShapeChanged();
    void toShapeChanged();
    void animationDurationChanged();
    void animationEasingChanged();
    void colorChanged();
    void implicitSizeChanged();
    void strokeColorChanged();
    void strokeWidthChanged();
    void shapeRotationChanged();
    void morphProgressChanged();

public:
    void paint(QPainter* painter) override;

private slots:
    void onAnimationValueChanged(const QVariant& value);
    void onMorphFinished();

private:
    QPainterPath buildPath() const;
    void startMorph(Shape from, Shape to);
    void rebuildMorph();

    Shape m_currentShape = Circle;
    Shape m_targetShape = Circle;
    Shape m_fromShape = Circle;
    Shape m_toShape = Circle;
    int m_animationDuration = 350;
    QEasingCurve m_animationEasing;
    float m_morphProgress = 1.0f;
    QColor m_color = Qt::black;
    qreal m_implicitSize = 0.0;
    QColor m_strokeColor = Qt::transparent;
    float m_strokeWidth = 0.0f;
    float m_shapeRotation = 0.0f;

    std::unique_ptr<RoundedPolygon::Morph> m_morph;
    QPropertyAnimation* m_animation = nullptr;
};
