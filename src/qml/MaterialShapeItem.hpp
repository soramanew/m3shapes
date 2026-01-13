#pragma once

#include "../morph/Morph.hpp"
#include "../shapes/MaterialShapes.hpp"
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QQuickItem>
#include <QSGGeometry>
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
class MaterialShapeItem : public QQuickItem {
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
    Q_PROPERTY(int animationDuration READ animationDuration WRITE
            setAnimationDuration NOTIFY animationDurationChanged)
    Q_PROPERTY(QEasingCurve animationEasing READ animationEasing WRITE
            setAnimationEasing NOTIFY animationEasingChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY
            fillColorChanged)
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

    [[nodiscard]] int animationDuration() const { return m_animationDuration; }

    void setAnimationDuration(int duration);

    [[nodiscard]] QEasingCurve animationEasing() const {
        return m_animationEasing;
    }

    void setAnimationEasing(const QEasingCurve& easing);

    [[nodiscard]] QColor fillColor() const { return m_fillColor; }

    void setFillColor(const QColor& color);

    [[nodiscard]] QColor strokeColor() const { return m_strokeColor; }

    void setStrokeColor(const QColor& color);

    [[nodiscard]] float strokeWidth() const { return m_strokeWidth; }

    void setStrokeWidth(float width);

    [[nodiscard]] float shapeRotation() const { return m_shapeRotation; }

    void setShapeRotation(float rotation);

    [[nodiscard]] float morphProgress() const { return m_morphProgress; }

signals:
    void shapeChanged();
    void animationDurationChanged();
    void animationEasingChanged();
    void fillColorChanged();
    void strokeColorChanged();
    void strokeWidthChanged();
    void shapeRotationChanged();
    void morphProgressChanged();

protected:
    QSGNode* updatePaintNode(
        QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

private slots:
    void onAnimationValueChanged(const QVariant& value);
    void onMorphFinished();

private:
    void buildGeometry(QSGGeometry* geometry);
    void startMorph(Shape from, Shape to);
    void setMorphProgress(float progress);

    Shape m_currentShape = Circle;
    Shape m_targetShape = Circle;
    // Material 3 expressive spring: damping=0.8, stiffness=380
    // Approximated with custom bezier curve
    int m_animationDuration = 350;
    QEasingCurve m_animationEasing;
    float m_morphProgress = 1.0f;
    QColor m_fillColor = Qt::black;
    QColor m_strokeColor = Qt::transparent;
    float m_strokeWidth = 0.0f;
    float m_shapeRotation = 0.0f;

    std::unique_ptr<RoundedPolygon::Morph> m_morph;
    QPropertyAnimation* m_animation = nullptr;
    bool m_geometryDirty = true;
};
