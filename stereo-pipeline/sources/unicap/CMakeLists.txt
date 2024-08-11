cmake_minimum_required(VERSION 3.16)

project(source_unicap VERSION 2.1.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

pkg_check_modules(UNICAP REQUIRED libunicap)
include_directories(${UNICAP_INCLUDE_DIRS})
link_directories(${UNICAP_LIBRARY_DIRS})

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
target_link_libraries(${plugin_name} PRIVATE mvl_stereo_pipeline)
target_link_libraries(${plugin_name} PRIVATE Qt5::Widgets)
target_link_libraries(${plugin_name} PRIVATE ${UNICAP_LIBRARIES})
target_link_libraries(${plugin_name} PRIVATE opencv_core)
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
