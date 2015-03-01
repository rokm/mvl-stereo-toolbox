cmake_minimum_required(VERSION 2.8)
project(method_opencv_bm CXX)

find_package(OpenCV 3.0.0 REQUIRED core calib3d imgproc)
find_package(Qt5Core REQUIRED)

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
qt5_use_modules(${plugin_name} Widgets)
target_link_libraries(${plugin_name} opencv_core opencv_calib3d opencv_imgproc)
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})