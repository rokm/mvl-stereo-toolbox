cmake_minimum_required(VERSION 2.8)
project(method_opencv_contrib_var CXX)

find_package(OpenCV REQUIRED core imgproc contrib)
find_package(Qt4 REQUIRED)

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    StereoMethodVar.cpp
    StereoMethodVarConfigWidget.cpp
    plugin.cpp
)

set(plugin_HEADERS
    StereoMethodVar.h
    StereoMethodVarConfigWidget.h
)

QT4_WRAP_CPP(plugin_HEADERS_MOC ${plugin_HEADERS})

include_directories(${PROJECT_SOURCE_DIR}/../../pipeline)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS_MOC})
target_link_libraries(${plugin_name} ${QT_LIBRARIES} ${OpenCV_LIBS})

