cmake_minimum_required(VERSION 2.8)
project(method_opencv_gpu_bp CXX)

find_package(OpenCV REQUIRED core gpu)
find_package(Qt4 REQUIRED)

set(plugin_name ${PROJECT_NAME})

set(plugin_SOURCES
    StereoMethodBeliefPropagationGPU.cpp
    StereoMethodBeliefPropagationGPUConfigWidget.cpp
    plugin.cpp
)

set(plugin_HEADERS
    StereoMethodBeliefPropagationGPU.h
    StereoMethodBeliefPropagationGPUConfigWidget.h
)

QT4_WRAP_CPP(plugin_HEADERS_MOC ${plugin_HEADERS})

include_directories(${PROJECT_SOURCE_DIR}/../../pipeline)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(${plugin_name} SHARED ${plugin_SOURCES} ${plugin_HEADERS_MOC})
target_link_libraries(${plugin_name} ${QT_LIBRARIES} ${OpenCV_LIBS})

