#include "MaterialShapeItem.hpp"
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>
#include <cmath>

using namespace RoundedPolygon;

MaterialShapeItem::MaterialShapeItem(QQuickItem* parent)
    : QQuickItem(parent) {
    setFlag(ItemHasContents, true);

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

void MaterialShapeItem::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        m_geometryDirty = true;
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
        m_geometryDirty = true;
        emit shapeRotationChanged();
        update();
    }
}

void MaterialShapeItem::buildGeometry(
    QSGGeometry* geometry, const QColor& fillColor) {
    if (m_morph == nullptr) {
        geometry->allocate(0, 0);
        return;
    }

    auto cubics = m_morph->asCubics(m_morphProgress);

    if (cubics.empty()) {
        geometry->allocate(0, 0);
        return;
    }

    constexpr int segmentsPerCubic = 16;
    int edgeVertexCount = static_cast<int>(cubics.size()) * segmentsPerCubic;

    float itemWidth = static_cast<float>(width());
    float itemHeight = static_cast<float>(height());
    float size = std::min(itemWidth, itemHeight);
    float centerX = itemWidth / 2.0f;
    float centerY = itemHeight / 2.0f;

    float cosR = std::cos(m_shapeRotation * FloatPi / 180.0f);
    float sinR = std::sin(m_shapeRotation * FloatPi / 180.0f);

    // Build edge points first
    std::vector<std::pair<float, float>> edgePoints;
    edgePoints.reserve(static_cast<size_t>(edgeVertexCount));

    for (const auto& cubic : cubics) {
        for (int j = 0; j < segmentsPerCubic; ++j) {
            float t =
                static_cast<float>(j) / static_cast<float>(segmentsPerCubic);
            Point pt = cubic.pointOnCurve(t);

            float x = (pt.x - 0.5f) * size;
            float y = (pt.y - 0.5f) * size;

            float rotX = x * cosR - y * sinR;
            float rotY = x * sinR + y * cosR;

            edgePoints.emplace_back(centerX + rotX, centerY + rotY);
        }
    }

    // Calculate normals and outer fringe points
    std::vector<std::pair<float, float>> outerPoints;
    outerPoints.reserve(edgePoints.size());

    constexpr float aaWidth = 1.0f; // Antialiasing fringe width in pixels

    for (size_t i = 0; i < edgePoints.size(); ++i) {
        size_t prev = (i + edgePoints.size() - 1) % edgePoints.size();
        size_t next = (i + 1) % edgePoints.size();

        // Calculate tangent from neighboring points
        float tx = edgePoints[next].first - edgePoints[prev].first;
        float ty = edgePoints[next].second - edgePoints[prev].second;

        // Normal is perpendicular to tangent (pointing outward)
        float nx = -ty;
        float ny = tx;

        // Normalize
        float len = std::sqrt(nx * nx + ny * ny);
        if (len > 0.0001f) {
            nx /= len;
            ny /= len;
        }

        // Check if normal points outward (away from center)
        float toCenterX = centerX - edgePoints[i].first;
        float toCenterY = centerY - edgePoints[i].second;
        float dot = nx * toCenterX + ny * toCenterY;
        if (dot > 0) {
            nx = -nx;
            ny = -ny;
        }

        // Offset outward by aaWidth pixels
        outerPoints.emplace_back(edgePoints[i].first + nx * aaWidth,
            edgePoints[i].second + ny * aaWidth);
    }

    // Geometry layout:
    // Vertex 0: center
    // Vertices 1 to edgeVertexCount: edge vertices (full alpha)
    // Vertices edgeVertexCount+1 to 2*edgeVertexCount: outer fringe (zero
    // alpha) Plus closing vertices

    int totalVertices = 1 + edgeVertexCount + 1 + edgeVertexCount + 1;

    // Triangles:
    // Fill: edgeVertexCount triangles (center -> edge[i] -> edge[i+1])
    // Fringe: 2 * edgeVertexCount triangles
    int fillTriangles = edgeVertexCount;
    int fringeTriangles = edgeVertexCount * 2;
    int totalIndices = (fillTriangles + fringeTriangles) * 3;

    geometry->allocate(totalVertices, totalIndices);

    auto* vertices = geometry->vertexDataAsColoredPoint2D();
    auto* indices = geometry->indexDataAsUShort();

    // Color with full alpha for fill
    uchar r = static_cast<uchar>(fillColor.red());
    uchar g = static_cast<uchar>(fillColor.green());
    uchar b = static_cast<uchar>(fillColor.blue());
    uchar a = static_cast<uchar>(fillColor.alpha());

    // Center vertex
    vertices[0].set(centerX, centerY, r, g, b, a);

    // Edge vertices (full alpha)
    for (int i = 0; i < edgeVertexCount; ++i) {
        vertices[1 + i].set(edgePoints[static_cast<size_t>(i)].first,
            edgePoints[static_cast<size_t>(i)].second, r, g, b, a);
    }
    // Closing edge vertex
    vertices[1 + edgeVertexCount] = vertices[1];

    // Outer fringe vertices (zero alpha, zero RGB for premultiplied blending)
    int outerStart = 1 + edgeVertexCount + 1;
    for (int i = 0; i < edgeVertexCount; ++i) {
        vertices[outerStart + i].set(outerPoints[static_cast<size_t>(i)].first,
            outerPoints[static_cast<size_t>(i)].second, 0, 0, 0, 0);
    }
    // Closing outer vertex
    vertices[outerStart + edgeVertexCount] = vertices[outerStart];

    int idx = 0;

    // Fill triangles (center -> edge)
    for (int i = 0; i < edgeVertexCount; ++i) {
        indices[idx++] = 0;
        indices[idx++] = static_cast<unsigned short>(1 + i);
        indices[idx++] = static_cast<unsigned short>(1 + i + 1);
    }

    // Fringe triangles (edge -> outer)
    for (int i = 0; i < edgeVertexCount; ++i) {
        int e0 = 1 + i;
        int e1 = 1 + i + 1;
        int o0 = outerStart + i;
        int o1 = outerStart + i + 1;

        // Triangle 1: edge[i], outer[i], edge[i+1]
        indices[idx++] = static_cast<unsigned short>(e0);
        indices[idx++] = static_cast<unsigned short>(o0);
        indices[idx++] = static_cast<unsigned short>(e1);

        // Triangle 2: outer[i], outer[i+1], edge[i+1]
        indices[idx++] = static_cast<unsigned short>(o0);
        indices[idx++] = static_cast<unsigned short>(o1);
        indices[idx++] = static_cast<unsigned short>(e1);
    }
}

QSGNode* MaterialShapeItem::updatePaintNode(
    QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/) {
    if (width() <= 0 || height() <= 0) {
        delete oldNode;
        return nullptr;
    }

    auto* node = dynamic_cast<QSGGeometryNode*>(oldNode);
    if (node == nullptr) {
        node = new QSGGeometryNode();
        node->setFlag(QSGNode::OwnsMaterial);
        node->setFlag(QSGNode::OwnsGeometry);

        auto* material = new QSGVertexColorMaterial();
        node->setMaterial(material);

        auto* geometry = new QSGGeometry(
            QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        node->setGeometry(geometry);

        m_geometryDirty = true;
    }

    if (m_geometryDirty) {
        buildGeometry(node->geometry(), m_color);
        node->markDirty(QSGNode::DirtyGeometry);
        m_geometryDirty = false;
    }

    return node;
}
