cmake_minimum_required(VERSION 3.2)

project(source_opencv_cam VERSION 2.1.0 LANGUAGES CXX)

find_package(OpenCV REQUIRED core videoio)
find_package(Qt5 COMPONENTS Concurrent Widgets REQUIRED)

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    camera.cpp
    camera_widget.cpp
    property_widget.cpp
    source.cpp
    source_widget.cpp
    plugin.cpp
)

set(plugin_HEADERS
    camera.h
    camera_widget.h
    property_widget.h
    source.h
    source_widget.h
)

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS})
target_link_libraries(${plugin_name} PRIVATE Qt5::Widgets Qt5::Concurrent)
target_link_libraries(${plugin_name} PRIVATE opencv_core opencv_videoio)
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
