cmake_minimum_required(VERSION 2.8)
project(source_dc1394 CXX)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core)
find_package(Qt4 REQUIRED)

pkg_check_modules(DC1394 REQUIRED libdc1394-2)
include_directories(${DC1394_INCLUDE_DIRS})
link_directories(${DC1394_LIBRARY_DIRS})

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    CameraDC1394.cpp
    CameraListModel.cpp
    ImageSourceDC1394.cpp
    plugin.cpp
)
    
set(plugin_HEADERS
    CameraDC1394.h
    CameraListModel.h
    ImageSourceDC1394.h
)

QT4_WRAP_CPP(plugin_HEADERS_MOC ${plugin_HEADERS})

include_directories(${PROJECT_SOURCE_DIR}/../../pipeline)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS_MOC})
target_link_libraries(${plugin_name} ${QT_LIBRARIES} ${OpenCV_LIBS})

