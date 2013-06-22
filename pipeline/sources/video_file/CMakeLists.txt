cmake_minimum_required(VERSION 2.8)
project(source_video_file CXX)

find_package(OpenCV REQUIRED core highgui)
find_package(Qt4 REQUIRED)

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    VideoFileCapture.cpp
    VideoFileCaptureConfigWidget.cpp
    SourceVideoFile.cpp
    SourceVideoFileConfigWidget.cpp
    plugin.cpp
)
    
set(plugin_HEADERS
    VideoFileCapture.h
    VideoFileCaptureConfigWidget.h
    SourceVideoFile.h
    SourceVideoFileConfigWidget.h
)

QT4_WRAP_CPP(plugin_HEADERS_MOC ${plugin_HEADERS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

include_directories(${ELAS_INCLUDE_DIRS})
link_directories(${ELAS_LIBRARY_DIRS})

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS_MOC})
target_link_libraries(${plugin_name} ${QT_LIBRARIES} ${OpenCV_LIBS})
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
