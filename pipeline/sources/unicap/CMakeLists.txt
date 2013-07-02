cmake_minimum_required(VERSION 2.8)
project(source_unicap CXX)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core imgproc)
find_package(Qt4 REQUIRED)

pkg_check_modules(UNICAP REQUIRED libunicap)
include_directories(${UNICAP_INCLUDE_DIRS})
link_directories(${UNICAP_LIBRARY_DIRS})

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    camera.cpp
    camera_widget.cpp
    camera_list.cpp
    property_widget.cpp
    source.cpp
    source_widget.cpp
    plugin.cpp
)
    
set(plugin_HEADERS
    camera.h
    camera_widget.h
    camera_list.h
    property_widget.h
    source.h
    source_widget.h
)

QT4_WRAP_CPP(plugin_HEADERS_MOC ${plugin_HEADERS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS_MOC})
target_link_libraries(${plugin_name} ${QT_LIBRARIES} ${OpenCV_LIBS} ${UNICAP_LIBRARIES})
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
