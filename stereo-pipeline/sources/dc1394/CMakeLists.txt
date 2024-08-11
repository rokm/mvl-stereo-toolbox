cmake_minimum_required(VERSION 3.16)

project(source_dc1394 VERSION 2.1.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core imgproc)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

pkg_check_modules(DC1394 REQUIRED libdc1394-2)
include_directories(${DC1394_INCLUDE_DIRS})
link_directories(${DC1394_LIBRARY_DIRS})

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    camera.cpp
    camera_widget.cpp
    feature_widget.cpp
    source.cpp
    source_widget.cpp
    plugin.cpp
)

set(plugin_HEADERS
    camera.h
    camera_widget.h
    feature_widget.h
    source.h
    source_widget.h
)

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS})
target_link_libraries(${plugin_name} PRIVATE Qt5::Widgets)
target_link_libraries(${plugin_name} PRIVATE ${DC1394_LIBRARIES})
target_link_libraries(${plugin_name} PRIVATE opencv_core opencv_imgproc)
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
