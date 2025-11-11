cmake_minimum_required(VERSION 3.16)
project(GraphEditor LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(SRC_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/srcs)
set(UI_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(RES_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/resources)

# Set UI files
set(UI_FILES ${UI_DIR}/mainwindow.ui)

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


)

# Include directories
target_include_directories(GraphEditor PRIVATE
    ${INCLUDE_DIR}
    ${SRC_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(GraphEditor PRIVATE Qt6::Widgets)

set_target_properties(GraphEditor PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
