cmake_minimum_required(VERSION 2.8)
project(source_video_file CXX)

find_package(OpenCV 3.0.0 REQUIRED core videoio)
find_package(Qt5Core REQUIRED)

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    source.cpp
    source_widget.cpp
    plugin.cpp
)

set(plugin_HEADERS
    source.h
    source_widget.h
)

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS})
qt5_use_modules(${plugin_name} Widgets Concurrent)
target_link_libraries(${plugin_name} opencv_core opencv_videoio)
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
