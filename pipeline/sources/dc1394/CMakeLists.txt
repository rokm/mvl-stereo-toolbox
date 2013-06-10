cmake_minimum_required(VERSION 2.8)
project(source_dc1394 CXX)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core imgproc)
find_package(Qt4 REQUIRED)

pkg_check_modules(DC1394 REQUIRED libdc1394-2)
include_directories(${DC1394_INCLUDE_DIRS})
link_directories(${DC1394_LIBRARY_DIRS})

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    CameraDC1394.cpp
    CameraDC1394ConfigWidget.cpp
    CameraListModel.cpp
    FeatureWidget.cpp
    SourceDC1394.cpp
    SourceDC1394ConfigWidget.cpp
    plugin.cpp
)
    
set(plugin_HEADERS
    CameraDC1394.h
    CameraDC1394ConfigWidget.h
    CameraListModel.h
    FeatureWidget.h
    SourceDC1394.h
    SourceDC1394ConfigWidget.h
)

QT4_WRAP_CPP(plugin_HEADERS_MOC ${plugin_HEADERS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS_MOC})
target_link_libraries(${plugin_name} ${QT_LIBRARIES} ${OpenCV_LIBS})
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
