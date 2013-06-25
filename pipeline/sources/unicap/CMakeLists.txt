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
    CameraUnicap.cpp
    CameraUnicapConfigWidget.cpp
    CameraListModel.cpp
    PropertyWidget.cpp
    SourceUnicap.cpp
    SourceUnicapConfigWidget.cpp
    plugin.cpp
)
    
set(plugin_HEADERS
    CameraUnicap.h
    CameraUnicapConfigWidget.h
    CameraListModel.h
    PropertyWidget.h
    SourceUnicap.h
    SourceUnicapConfigWidget.h
)

QT4_WRAP_CPP(plugin_HEADERS_MOC ${plugin_HEADERS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS_MOC})
target_link_libraries(${plugin_name} ${QT_LIBRARIES} ${OpenCV_LIBS})
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
