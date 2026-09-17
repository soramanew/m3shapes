cmake_minimum_required(VERSION 3.16)
project(M3Shapes VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion
    -Wold-style-cast -Wnull-dereference -Wdouble-promotion
    -Wformat=2 -Wfloat-equal -Woverloaded-virtual
    -Wsign-conversion -Wredundant-decls -Wswitch
    -Wunreachable-code
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wunused-lambda-capture)
endif()

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml ShaderTools)

qt_standard_project_setup(REQUIRES 6.8)

# Core library
add_library(m3shapes_core STATIC
    include/m3shapes/core/point.hpp
    include/m3shapes/core/utils.hpp
    include/m3shapes/core/cornerrounding.hpp
    include/m3shapes/core/cubic.hpp
    src/core/cubic.cpp
    include/m3shapes/core/feature.hpp
    src/core/feature.cpp
    include/m3shapes/core/roundedpolygon.hpp
    src/core/roundedpolygon.cpp
)

target_include_directories(m3shapes_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(m3shapes_core PUBLIC
    Qt6::Core
)

# Morph library
add_library(m3shapes_morph STATIC
    include/m3shapes/morph/floatmapping.hpp
    src/morph/floatmapping.cpp
    include/m3shapes/morph/polygonmeasure.hpp
    src/morph/polygonmeasure.cpp
    include/m3shapes/morph/featuremapping.hpp
    src/morph/featuremapping.cpp
    include/m3shapes/morph/morph.hpp
    src/morph/morph.cpp
)

target_include_directories(m3shapes_morph PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(m3shapes_morph PUBLIC
    m3shapes_core
)

# Shapes library
add_library(m3shapes_shapes STATIC
    include/m3shapes/shapes/shapes.hpp
    src/shapes/shapes.cpp
    include/m3shapes/shapes/materialshapes.hpp
    src/shapes/materialshapes.cpp
)

target_include_directories(m3shapes_shapes PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(m3shapes_shapes PUBLIC
    m3shapes_core
)

# QML Plugin
if(NOT DEFINED QT_QML_OUTPUT_DIRECTORY)
    set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")
endif()

qt_add_qml_module(m3shapes
    URI M3Shapes
    VERSION 1.0
    SOURCES
        include/m3shapes/qml/materialshapeitem.hpp
        src/qml/materialshapeitem.cpp
        include/m3shapes/qml/smoothshapematerial.hpp
        src/qml/smoothshapematerial.cpp
)

# Compile the analytic-antialiasing shaders to .qsb via Qt Shader Tools and
# embed them at :/m3shapes/shaders/*.qsb (see SmoothShapeShader).
qt_add_shaders(m3shapes "m3shapes_shaders"
    PREFIX "/m3shapes"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    FILES
        src/shaders/smoothshape.vert
        src/shaders/smoothshape.frag
)

target_include_directories(m3shapes PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# qmltyperegistrar emits the registration include from the header's basename
# alone (<materialshapeitem.hpp>), so the generated TU needs the qml header
# directory directly. Private: consumers use <m3shapes/qml/...> instead.
target_include_directories(m3shapes PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/m3shapes/qml
)

target_link_libraries(m3shapes PUBLIC
    Qt::Quick
    Qt::Qml
    m3shapes_core
    m3shapes_morph
    m3shapes_shapes
)

# Install the backing library and the plugin side by side in the module dir.
set(INSTALL_QMLDIR "${CMAKE_INSTALL_LIBDIR}/qt6/qml" CACHE STRING "QML install dir")

message(STATUS "QML install dir: ${CMAKE_INSTALL_PREFIX}/${INSTALL_QMLDIR}")

qt_query_qml_module(m3shapes
    PLUGIN_TARGET module_plugin_target
    TARGET_PATH module_target_path
    QMLDIR module_qmldir
    TYPEINFO module_typeinfo
)

set(module_dir "${INSTALL_QMLDIR}/${module_target_path}")

install(TARGETS m3shapes
    LIBRARY DESTINATION "${module_dir}"
    RUNTIME DESTINATION "${module_dir}"
)
install(TARGETS "${module_plugin_target}"
    LIBRARY DESTINATION "${module_dir}"
    RUNTIME DESTINATION "${module_dir}"
)
install(FILES "${module_qmldir}" DESTINATION "${module_dir}")
install(FILES "${module_typeinfo}" DESTINATION "${module_dir}")

# Public headers, consumed as <m3shapes/qml/materialshapeitem.hpp>.
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# Example application
option(M3SHAPES_BUILD_EXAMPLES "Build example applications" OFF)

if(M3SHAPES_BUILD_EXAMPLES)
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
        m3shapes
    )

    # Set up import paths
    set_target_properties(shape_gallery PROPERTIES
        QT_QML_IMPORT_PATH "${CMAKE_BINARY_DIR}"
    )
endif()
