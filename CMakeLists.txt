cmake_minimum_required(VERSION 3.16)
project(GraphEditor LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MLPACK REQUIRED mlpack)
qt_standard_project_setup()

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(SRC_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/srcs)
set(UI_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(RES_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/resources)

# Set UI files
set(UI_FILES ${UI_DIR}/MainWindow.ui)

# Set SOURCE files (cpp files)
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/MainWindow.cpp
    ${SRC_DIR}/GridScene.cpp
    ${SRC_DIR}/GridGraphicsView.cpp
)

# Set HEADER files (h files) - THIS IS CRITICAL FOR MOC
set(HEADERS
    ${INCLUDE_DIR}/MainWindow.h
    ${INCLUDE_DIR}/GridScene.h
    ${INCLUDE_DIR}/GridGraphicsView.h
)

set(RESOURCES ${RES_DIR}/resource.qrc)

# Create executable with ALL files
qt_add_executable(GraphEditor
    ${SOURCES}
    ${HEADERS}    # Add headers here for MOC processing
    ${UI_FILES}
    ${RESOURCES}
    includes/model/Vertex.h
    srcs/model/Vertex.cpp
    includes/model/DisjointSet.h
    includes/model/Edge.h
    srcs/model/Edge.cpp
    includes/model/Graph.h
    srcs/model/Graph.cpp
    includes/algorithms/GraphAlgorithms.h
    includes/algorithms/MSTModelSelector.h
    srcs/algorithms/GraphAlgorithms.cpp
    srcs/algorithms/MSTModelSelector.cpp
    includes/GraphEditor.h
    srcs/GraphEditor.cpp
    includes/core/actions/AAction.h
    includes/core/actions/AddVertexAction.h
    srcs/core/actions/AddVertexAction.cpp
    includes/core/UndoManager.h
    srcs/core/UndoManager.cpp
    includes/core/actions/AddEdgeAction.h
    srcs/core/actions/AddEdgeAction.cpp
    includes/core/actions/RemoveVertexAction.h
    srcs/core/actions/RemoveVertexAction.cpp
    includes/core/actions/RemoveEdgeAction.h
    srcs/core/actions/RemoveEdgeAction.cpp
    includes/view/VertexItem.h
    srcs/view/VertexItem.cpp

    includes/view/EdgeItem.h
    srcs/view/EdgeItem.cpp
    includes/algorithms/Triangulation.h
    srcs/algorithms/Triangulation.cpp
    srcs/mst_dataset/FeatureExtractor.cpp
    srcs/mst_dataset/FeatureExtractor.h


)

# Include directories
target_include_directories(GraphEditor PRIVATE
    ${INCLUDE_DIR}
    ${SRC_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MLPACK_INCLUDE_DIRS}
)

target_link_libraries(GraphEditor PRIVATE Qt6::Widgets ${MLPACK_LIBRARIES})

set_target_properties(GraphEditor PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
