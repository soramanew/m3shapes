cmake_minimum_required(VERSION 3.16)
project(M3Shapes VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml)

qt_policy(SET QTP0001 NEW)

# Core library
add_library(m3shapes_core STATIC
    src/core/Point.hpp
    src/core/Utils.hpp
    src/core/CornerRounding.hpp
    src/core/Cubic.hpp
    src/core/Cubic.cpp
    src/core/Feature.hpp
    src/core/Feature.cpp
    src/core/RoundedPolygon.hpp
    src/core/RoundedPolygon.cpp
)

target_include_directories(m3shapes_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(m3shapes_core PUBLIC
    Qt6::Core
)

# Morph library
add_library(m3shapes_morph STATIC
    src/morph/FloatMapping.hpp
    src/morph/FloatMapping.cpp
    src/morph/PolygonMeasure.hpp
    src/morph/PolygonMeasure.cpp
    src/morph/FeatureMapping.hpp
    src/morph/FeatureMapping.cpp
    src/morph/Morph.hpp
    src/morph/Morph.cpp
)

target_include_directories(m3shapes_morph PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(m3shapes_morph PUBLIC
    m3shapes_core
)

# Shapes library
add_library(m3shapes_shapes STATIC
    src/shapes/Shapes.hpp
    src/shapes/Shapes.cpp
    src/shapes/MaterialShapes.hpp
    src/shapes/MaterialShapes.cpp
)

target_include_directories(m3shapes_shapes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(m3shapes_shapes PUBLIC
    m3shapes_core
)

# QML Plugin
qt_add_qml_module(m3shapesplugin
    URI M3Shapes
    VERSION 1.0
    SOURCES
        src/qml/MaterialShapeItem.hpp
        src/qml/MaterialShapeItem.cpp
    OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/M3Shapes
)

target_include_directories(m3shapesplugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qml
)

target_link_libraries(m3shapesplugin PUBLIC
    Qt::Quick
    Qt::Qml
    m3shapes_core
    m3shapes_morph
    m3shapes_shapes
)

# Example application
qt_add_executable(shape_gallery
    examples/main.cpp
)

qt_add_qml_module(shape_gallery
    URI ShapeGalleryApp
    VERSION 1.0
    QML_FILES
        examples/ShapeGallery.qml
)

target_link_libraries(shape_gallery PRIVATE
    Qt6::Quick
    Qt6::Qml
    m3shapesplugin
)

# Set up import paths
set_target_properties(shape_gallery PROPERTIES
    QT_QML_IMPORT_PATH "${CMAKE_BINARY_DIR}"
)
