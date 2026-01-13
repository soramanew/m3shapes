#include "MaterialShapeItem.hpp"
#include <QSGFlatColorMaterial>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <cmath>

using namespace RoundedPolygon;

MaterialShapeItem::MaterialShapeItem(QQuickItem* parent)
    : QQuickItem(parent) {
    setFlag(ItemHasContents, true);

    // Approximate Material 3 spring (damping=0.8, stiffness=380) with bezier
    // This creates a curve with slight overshoot (~5%) that settles back
    m_animationEasing.setType(QEasingCurve::BezierSpline);
    m_animationEasing.addCubicBezierSegment(
        QPointF(0.3, 1.3), QPointF(0.7, 1.0), QPointF(1.0, 1.0));

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
    auto fromShape = MaterialShapes::getShape(
        static_cast<MaterialShapes::ShapeType>(from));
    auto toShape =
        MaterialShapes::getShape(static_cast<MaterialShapes::ShapeType>(to));

    m_morph = std::make_unique<Morph>(fromShape, toShape);
    m_morphProgress = 0.0f;
    m_geometryDirty = true;

    m_animation->start();

    update();
}

void MaterialShapeItem::onAnimationValueChanged(const QVariant& value) {
    setMorphProgress(value.toFloat());
}

void MaterialShapeItem::setMorphProgress(float progress) {
    if (!qFuzzyCompare(m_morphProgress, progress)) {
        m_morphProgress = progress;
        m_geometryDirty = true;
        emit morphProgressChanged();
        update();
    }
}

void MaterialShapeItem::onMorphFinished() {
    m_currentShape = m_targetShape;
    m_morphProgress = 1.0f;
    m_geometryDirty = true;
    update();
}

void MaterialShapeItem::setFillColor(const QColor& color) {
    if (m_fillColor != color) {
        m_fillColor = color;
        emit fillColorChanged();
        update();
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
        m_geometryDirty = true;
        emit shapeRotationChanged();
        update();
    }
}

void MaterialShapeItem::buildGeometry(QSGGeometry* geometry) {
    if (m_morph == nullptr) {
        geometry->allocate(0, 0);
        return;
    }

    auto cubics = m_morph->asCubics(m_morphProgress);

    if (cubics.empty()) {
        geometry->allocate(0, 0);
        return;
    }

    constexpr int segmentsPerCubic = 8;
    int outlineVertices = static_cast<int>(cubics.size()) * segmentsPerCubic;
    int totalVertices = 1 + outlineVertices + 1;
    int numTriangles = outlineVertices;
    int totalIndices = numTriangles * 3;

    geometry->allocate(totalVertices, totalIndices);

    auto* vertices = geometry->vertexDataAsPoint2D();
    auto* indices = geometry->indexDataAsUShort();

    float itemWidth = static_cast<float>(width());
    float itemHeight = static_cast<float>(height());
    float size = std::min(itemWidth, itemHeight);

    float centerX = itemWidth / 2.0f;
    float centerY = itemHeight / 2.0f;

    float cosR = std::cos(m_shapeRotation * FloatPi / 180.0f);
    float sinR = std::sin(m_shapeRotation * FloatPi / 180.0f);

    vertices[0].set(centerX, centerY);

    int vertexIndex = 1;
    for (const auto& cubic : cubics) {
        for (int j = 0; j < segmentsPerCubic; ++j) {
            float t = static_cast<float>(j) / segmentsPerCubic;
            Point pt = cubic.pointOnCurve(t);

            float x = (pt.x - 0.5f) * size;
            float y = (pt.y - 0.5f) * size;

            float rotX = x * cosR - y * sinR;
            float rotY = x * sinR + y * cosR;

            vertices[vertexIndex].set(centerX + rotX, centerY + rotY);
            vertexIndex++;
        }
    }

    vertices[vertexIndex] = vertices[1];

    for (int i = 0; i < numTriangles; ++i) {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = static_cast<unsigned short>(i + 1);
        indices[i * 3 + 2] = static_cast<unsigned short>(i + 2);
    }
}

QSGNode* MaterialShapeItem::updatePaintNode(QSGNode* oldNode,
    UpdatePaintNodeData* /*updatePaintNodeData*/) {
    if (width() <= 0 || height() <= 0) {
        delete oldNode;
        return nullptr;
    }

    auto* node = dynamic_cast<QSGGeometryNode*>(oldNode);
    if (node == nullptr) {
        node = new QSGGeometryNode();
        node->setFlag(QSGNode::OwnsMaterial);
        node->setFlag(QSGNode::OwnsGeometry);

        auto* material = new QSGFlatColorMaterial();
        material->setColor(m_fillColor);
        node->setMaterial(material);

        auto* geometry =
            new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0, 0);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        node->setGeometry(geometry);

        m_geometryDirty = true;
    }

    auto* material = dynamic_cast<QSGFlatColorMaterial*>(node->material());
    if (material != nullptr && material->color() != m_fillColor) {
        material->setColor(m_fillColor);
        node->markDirty(QSGNode::DirtyMaterial);
    }

    if (m_geometryDirty) {
        buildGeometry(node->geometry());
        node->markDirty(QSGNode::DirtyGeometry);
        m_geometryDirty = false;
    }

    return node;
}
