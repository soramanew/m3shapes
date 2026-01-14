#include "MaterialShapeItem.hpp"
#include <QPainter>
#include <cmath>

using namespace RoundedPolygon;

MaterialShapeItem::MaterialShapeItem(QQuickItem* parent)
    : QQuickPaintedItem(parent) {
    setAntialiasing(true);

    // M3 expressive fast spatial
    m_animationEasing.setType(QEasingCurve::BezierSpline);
    m_animationEasing.addCubicBezierSegment(
        QPointF(0.42, 1.67), QPointF(0.21, 0.90), QPointF(1.0, 1.0));

    m_animation = new QPropertyAnimation(this);
    m_animation->setTargetObject(this);
    m_animation->setPropertyName("morphProgress");
    m_animation->setStartValue(0.0f);
    m_animation->setEndValue(1.0f);
    m_animation->setDuration(m_animationDuration);
    m_animation->setEasingCurve(m_animationEasing);

    connect(m_animation, &QPropertyAnimation::valueChanged, this,
        &MaterialShapeItem::onAnimationValueChanged);
    connect(m_animation, &QPropertyAnimation::finished, this,
        &MaterialShapeItem::onMorphFinished);

    // Initialize with circle shape
    auto circleShape =
        MaterialShapes::getShape(MaterialShapes::ShapeType::Circle);
    m_morph = std::make_unique<Morph>(circleShape, circleShape);
}

void MaterialShapeItem::setShape(Shape shape) {
    if (m_targetShape != shape) {
        m_targetShape = shape;
        emit shapeChanged();

        if (m_animation->state() == QAbstractAnimation::Running) {
            m_animation->stop();
        }

        startMorph(m_currentShape, shape);
    }
}

void MaterialShapeItem::setAnimationDuration(int duration) {
    if (m_animationDuration != duration) {
        m_animationDuration = duration;
        m_animation->setDuration(duration);
        emit animationDurationChanged();
    }
}

void MaterialShapeItem::setAnimationEasing(const QEasingCurve& easing) {
    if (m_animationEasing != easing) {
        m_animationEasing = easing;
        m_animation->setEasingCurve(easing);
        emit animationEasingChanged();
    }
}

void MaterialShapeItem::startMorph(Shape from, Shape to) {
    auto fromShape =
        MaterialShapes::getShape(static_cast<MaterialShapes::ShapeType>(from));
    auto toShape =
        MaterialShapes::getShape(static_cast<MaterialShapes::ShapeType>(to));

    m_morph = std::make_unique<Morph>(fromShape, toShape);
    m_morphProgress = 0.0f;

    m_animation->start();

    update();
}

void MaterialShapeItem::onAnimationValueChanged(const QVariant& value) {
    setMorphProgress(value.toFloat());
}

void MaterialShapeItem::setMorphProgress(float progress) {
    if (!qFuzzyCompare(m_morphProgress, progress)) {
        m_morphProgress = progress;
        emit morphProgressChanged();
        update();
    }
}

void MaterialShapeItem::onMorphFinished() {
    m_currentShape = m_targetShape;
    m_morphProgress = 1.0f;
    update();
}

void MaterialShapeItem::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        update();
    }
}

void MaterialShapeItem::setImplicitSize(qreal size) {
    if (!qFuzzyCompare(m_implicitSize, size)) {
        m_implicitSize = size;
        setImplicitWidth(size);
        setImplicitHeight(size);
        emit implicitSizeChanged();
    }
}

void MaterialShapeItem::setStrokeColor(const QColor& color) {
    if (m_strokeColor != color) {
        m_strokeColor = color;
        emit strokeColorChanged();
        update();
    }
}

void MaterialShapeItem::setStrokeWidth(float width) {
    if (!qFuzzyCompare(m_strokeWidth, width)) {
        m_strokeWidth = width;
        emit strokeWidthChanged();
        update();
    }
}

void MaterialShapeItem::setShapeRotation(float rotation) {
    if (!qFuzzyCompare(m_shapeRotation, rotation)) {
        m_shapeRotation = rotation;
        emit shapeRotationChanged();
        update();
    }
}

QPainterPath MaterialShapeItem::buildPath() const {
    QPainterPath path;

    if (m_morph == nullptr) {
        return path;
    }

    auto cubics = m_morph->asCubics(m_morphProgress);

    if (cubics.empty()) {
        return path;
    }

    float itemWidth = static_cast<float>(width());
    float itemHeight = static_cast<float>(height());
    float size = std::min(itemWidth, itemHeight);
    float centerX = itemWidth / 2.0f;
    float centerY = itemHeight / 2.0f;

    float cosR = std::cos(m_shapeRotation * FloatPi / 180.0f);
    float sinR = std::sin(m_shapeRotation * FloatPi / 180.0f);

    auto transformPoint = [&](float px, float py) -> QPointF {
        float x = (px - 0.5f) * size;
        float y = (py - 0.5f) * size;
        float rotX = x * cosR - y * sinR;
        float rotY = x * sinR + y * cosR;
        return QPointF(centerX + rotX, centerY + rotY);
    };

    // Build path from cubics (like Android's toPath())
    bool first = true;
    for (const auto& cubic : cubics) {
        if (first) {
            path.moveTo(transformPoint(cubic.anchor0X(), cubic.anchor0Y()));
            first = false;
        }
        path.cubicTo(
            transformPoint(cubic.control0X(), cubic.control0Y()),
            transformPoint(cubic.control1X(), cubic.control1Y()),
            transformPoint(cubic.anchor1X(), cubic.anchor1Y()));
    }
    path.closeSubpath();

    return path;
}

void MaterialShapeItem::paint(QPainter* painter) {
    if (width() <= 0 || height() <= 0) {
        return;
    }

    QPainterPath path = buildPath();

    // Fill
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_color);
    painter->drawPath(path);

    // Stroke
    if (m_strokeWidth > 0 && m_strokeColor.alpha() > 0) {
        painter->setPen(QPen(m_strokeColor, m_strokeWidth));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
    }
}
