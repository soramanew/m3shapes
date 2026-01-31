#include "MaterialShapeItem.hpp"
#include "../core/RoundedPolygon.hpp"
#include "../shapes/Shapes.hpp"
#include <QPainter>
#include <QVariantMap>
#include <cmath>
#include <numbers>

using namespace RoundedPolygon;

// ========== RoundedPolygonWrapper ==========

RoundedPolygonWrapper::RoundedPolygonWrapper(const RoundedPolygonShape& shape)
    : m_shape(shape) {}

const RoundedPolygonShape& RoundedPolygonWrapper::shape() const {
    static RoundedPolygonShape defaultShape =
        MaterialShapes::getShape(MaterialShapes::ShapeType::Circle);
    return m_shape.has_value() ? *m_shape : defaultShape;
}

RoundedPolygonWrapper RoundedPolygonWrapper::normalized() const {
    if (!m_shape.has_value()) {
        return {};
    }
    return RoundedPolygonWrapper(m_shape->normalized());
}

// ========== MaterialShapeItem ==========

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

// ========== Factory functions ==========

QVariantMap MaterialShapeItem::point(
    float x, float y, float radius, float smoothing) {
    QVariantMap map;
    map["x"] = x;
    map["y"] = y;
    map["radius"] = radius;
    map["smoothing"] = smoothing;
    return map;
}

RoundedPolygonWrapper MaterialShapeItem::polygon(const QVariantList& vertices,
    int reps, float centerX, float centerY, bool mirroring) {
    if (vertices.isEmpty()) {
        return {};
    }

    std::vector<MaterialShapes::PointNRound> points;
    points.reserve(static_cast<size_t>(vertices.size()));

    for (const QVariant& vertex : vertices) {
        float x = 0.0f, y = 0.0f;
        float radius = 0.0f;
        float smoothing = 0.0f;

        if (vertex.canConvert<QVariantMap>()) {
            QVariantMap map = vertex.toMap();
            x = map.value("x", 0.0f).toFloat();
            y = map.value("y", 0.0f).toFloat();
            radius = map.value("radius", 0.0f).toFloat();
            smoothing = map.value("smoothing", 0.0f).toFloat();
        } else if (vertex.canConvert<QPointF>()) {
            QPointF pt = vertex.toPointF();
            x = static_cast<float>(pt.x());
            y = static_cast<float>(pt.y());
        } else if (vertex.canConvert<QVariantList>()) {
            QVariantList coords = vertex.toList();
            if (coords.size() >= 2) {
                x = coords[0].toFloat();
                y = coords[1].toFloat();
            }
        }

        points.emplace_back(x, y, CornerRounding(radius, smoothing));
    }

    return RoundedPolygonWrapper(
        MaterialShapes::customPolygon(points, reps, centerX, centerY, mirroring)
            .normalized());
}

RoundedPolygonWrapper MaterialShapeItem::regularPolygon(
    int numVertices, float radius, float smoothing) {
    if (numVertices < 3) {
        numVertices = 3;
    }
    return RoundedPolygonWrapper(
        RoundedPolygonShape(numVertices, 1.0f, 0.0f, 0.0f,
            CornerRounding(radius, smoothing))
            .normalized());
}

RoundedPolygonWrapper MaterialShapeItem::star(
    int points, float innerRadius, float radius, float smoothing) {
    if (points < 2) {
        points = 2;
    }
    return RoundedPolygonWrapper(
        Shapes::star(points, 1.0f, innerRadius, CornerRounding(radius, smoothing))
            .normalized());
}

RoundedPolygonWrapper MaterialShapeItem::rectangle(
    float width, float height, float radius, float smoothing) {
    return RoundedPolygonWrapper(
        Shapes::rectangle(width, height, CornerRounding(radius, smoothing))
            .normalized());
}

RoundedPolygonWrapper MaterialShapeItem::squircle(float n, int segments) {
    if (n < 0.1f) {
        n = 0.1f;
    }
    if (segments < 4) {
        segments = 4;
    }

    std::vector<float> vertices;
    vertices.reserve(static_cast<size_t>(segments) * 2);

    const float exp = 2.0f / n;
    const float pi2 = static_cast<float>(std::numbers::pi) * 2.0f;

    for (int i = 0; i < segments; ++i) {
        float t = pi2 * static_cast<float>(i) / static_cast<float>(segments);
        float cosT = std::cos(t);
        float sinT = std::sin(t);

        // Superellipse: |x|^n + |y|^n = 1
        // Parametric: x = sign(cos(t)) * |cos(t)|^(2/n)
        //             y = sign(sin(t)) * |sin(t)|^(2/n)
        float x = std::copysign(std::pow(std::abs(cosT), exp), cosT);
        float y = std::copysign(std::pow(std::abs(sinT), exp), sinT);

        // Scale to 0-1 range (centered at 0.5)
        vertices.push_back(x * 0.5f + 0.5f);
        vertices.push_back(y * 0.5f + 0.5f);
    }

    // No corner rounding needed - the curve itself is smooth
    return RoundedPolygonWrapper(
        RoundedPolygonShape(vertices, CornerRounding::Unrounded).normalized());
}

// ========== Property setters ==========

void MaterialShapeItem::setShape(Shape shape) {
    if (m_targetShape != shape) {
        m_targetShape = shape;
        emit shapeChanged();

        if (m_animation->state() == QAbstractAnimation::Running) {
            m_animation->stop();
        }

        // Skip morphing during initial creation
        if (!isComponentComplete()) {
            m_currentShape = shape;
            m_fromShape = shape;
            m_toShape = shape;
            auto targetShape = getShapeForEnum(shape);
            m_morph = std::make_unique<Morph>(targetShape, targetShape);
            m_morphProgress = 1.0f;
            return;
        }

        // Start from toShape (handles transition from manual to auto mode)
        startMorph(m_toShape, shape);
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

void MaterialShapeItem::setFromShape(Shape shape) {
    if (m_fromShape != shape) {
        m_fromShape = shape;
        emit fromShapeChanged();
        rebuildMorph();
    }
}

void MaterialShapeItem::setToShape(Shape shape) {
    if (m_toShape != shape) {
        m_toShape = shape;
        emit toShapeChanged();
        rebuildMorph();
    }
}

void MaterialShapeItem::setCustomShape(const RoundedPolygonWrapper& shape) {
    if (!shape.isValid()) {
        return;
    }

    m_customShape = shape;
    emit customShapeChanged();

    // Only rebuild morph if shape is already Custom
    if (m_targetShape == Custom) {
        if (!isComponentComplete()) {
            m_morph = std::make_unique<Morph>(shape.shape(), shape.shape());
            m_morphProgress = 1.0f;
        } else {
            rebuildMorph();
        }
        update();
    }
}

void MaterialShapeItem::setCustomFromShape(const RoundedPolygonWrapper& shape) {
    if (!shape.isValid()) {
        return;
    }
    m_customFromShape = shape;
    m_fromShape = Custom;
    emit customFromShapeChanged();
    emit fromShapeChanged();
    rebuildMorph();
}

void MaterialShapeItem::setCustomToShape(const RoundedPolygonWrapper& shape) {
    if (!shape.isValid()) {
        return;
    }
    m_customToShape = shape;
    m_toShape = Custom;
    emit customToShapeChanged();
    emit toShapeChanged();
    rebuildMorph();
}

void MaterialShapeItem::rebuildMorph() {
    RoundedPolygonShape from = getShapeForEnum(m_fromShape);
    RoundedPolygonShape to = getShapeForEnum(m_toShape);
    m_morph = std::make_unique<Morph>(from, to);
    update();
}

RoundedPolygonShape MaterialShapeItem::getShapeForEnum(Shape shape) const {
    if (shape == Custom) {
        // Check which custom shape to use based on context
        if (m_customToShape.isValid() && shape == m_toShape) {
            return m_customToShape.shape();
        }
        if (m_customFromShape.isValid() && shape == m_fromShape) {
            return m_customFromShape.shape();
        }
        if (m_customShape.isValid()) {
            return m_customShape.shape();
        }
        // Fallback to circle
        return MaterialShapes::getShape(MaterialShapes::ShapeType::Circle);
    }
    return MaterialShapes::getShape(static_cast<MaterialShapes::ShapeType>(shape));
}

void MaterialShapeItem::startMorph(Shape from, Shape to) {
    // Sync manual morph properties
    m_fromShape = from;
    m_toShape = to;
    emit fromShapeChanged();
    emit toShapeChanged();

    auto fromShape = getShapeForEnum(from);
    auto toShape = getShapeForEnum(to);

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
    m_fromShape = m_targetShape;
    m_morphProgress = 1.0f;
    emit fromShapeChanged();
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
    float cX = itemWidth / 2.0f;
    float cY = itemHeight / 2.0f;

    auto transformPoint = [&](float px, float py) -> QPointF {
        float x = cX + (px - 0.5f) * size;
        float y = cY + (py - 0.5f) * size;
        return QPointF(static_cast<qreal>(x), static_cast<qreal>(y));
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
        painter->setPen(QPen(m_strokeColor, static_cast<qreal>(m_strokeWidth)));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
    }
}
