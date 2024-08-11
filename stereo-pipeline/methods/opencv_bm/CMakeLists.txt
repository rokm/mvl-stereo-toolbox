cmake_minimum_required(VERSION 3.16)

project(method_opencv_bm VERSION 2.1.0 LANGUAGES CXX)

find_package(OpenCV REQUIRED core calib3d imgproc)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    method.cpp
    method_widget.cpp
    plugin.cpp
)

set(plugin_HEADERS
    method.h
    method_widget.h
)

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS})
target_link_libraries(${plugin_name} PRIVATE Qt5::Widgets)
target_link_libraries(${plugin_name} PRIVATE opencv_core opencv_calib3d opencv_imgproc)
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
