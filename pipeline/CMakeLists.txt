cmake_minimum_required(VERSION 2.8)
project(libstereo_pipeline CXX)

find_package(OpenCV REQUIRED core imgproc)
find_package(Qt4 REQUIRED)

set(pipeline_SOURCES
    ImageSource.cpp
    StereoCalibration.cpp
    StereoMethod.cpp
    StereoPipeline.cpp
)

set(pipeline_HEADERS
    ImageSource.h
    StereoCalibration.h
    StereoMethod.h
    StereoPipeline.h
)

QT4_WRAP_CPP(pipeline_HEADERS_MOC ${pipeline_HEADERS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(stereo_pipeline SHARED ${pipeline_SOURCES} ${pipeline_HEADERS_MOC})
target_link_libraries(stereo_pipeline ${QT_LIBRARIES})

