cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

project(source_image_file CXX)

find_package(OpenCV 3.0.0 REQUIRED core imgcodecs)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    image_file.cpp
    image_file_widget.cpp
    source.cpp
    source_widget.cpp
    plugin.cpp
)

set(plugin_HEADERS
    image_file.h
    image_file_widget.h
    source.h
    source_widget.h
)

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS})
target_link_libraries(${plugin_name} Qt5::Widgets Qt5::Network)
target_link_libraries(${plugin_name} opencv_core opencv_imgcodecs)
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
