cmake_minimum_required(VERSION 3.16)

project(source_image_file VERSION 2.1.0 LANGUAGES CXX)

find_package(OpenCV REQUIRED core imgcodecs)
find_package(Qt5 COMPONENTS Network Widgets REQUIRED)

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
target_link_libraries(${plugin_name} PRIVATE mvl_stereo_pipeline)
target_link_libraries(${plugin_name} PRIVATE Qt5::Widgets Qt5::Network)
target_link_libraries(${plugin_name} PRIVATE opencv_core opencv_imgcodecs)
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
