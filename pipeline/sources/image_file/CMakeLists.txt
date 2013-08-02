cmake_minimum_required(VERSION 2.8)
project(source_image_file CXX)

find_package(OpenCV REQUIRED core highgui imgproc)
find_package(Qt4 REQUIRED QtNetwork)

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

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS})
target_link_libraries(${plugin_name} ${QT_LIBRARIES} ${OpenCV_LIBS})
set_target_properties(${plugin_name} PROPERTIES PREFIX "")

install(TARGETS ${plugin_name} DESTINATION ${MVL_STEREO_PIPELINE_PLUGIN_DIR})
