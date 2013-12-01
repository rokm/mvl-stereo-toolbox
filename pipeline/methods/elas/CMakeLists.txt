cmake_minimum_required(VERSION 2.8)
project(method_elas CXX)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core imgproc)
find_package(Qt5Core REQUIRED)

pkg_check_modules(ELAS REQUIRED libelas)
include_directories(${ELAS_INCLUDE_DIRS})
link_directories(${ELAS_LIBRARY_DIRS})

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
target_link_libraries(${plugin_name} ${OpenCV_LIBS} ${ELAS_LIBRARIES})
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
