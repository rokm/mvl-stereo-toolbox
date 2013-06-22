cmake_minimum_required(VERSION 2.8)
project(libmvl_stereo_pipeline CXX)

include(GNUInstallDirs)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -Wextra -Wno-unused-parameter)
endif()

find_package(OpenCV REQUIRED core imgproc)
find_package(Qt4 REQUIRED)

set(pipeline_SOURCES
    ImagePairSource.cpp
    StereoCalibrationPattern.cpp
    StereoMethod.cpp
    StereoPipeline.cpp
    StereoRectification.cpp
)

set(pipeline_HEADERS
    ImagePairSource.h
    PluginFactory.h
    StereoCalibrationPattern.h
    StereoMethod.h
    StereoPipeline.h
    StereoRectification.h
)

QT4_WRAP_CPP(pipeline_HEADERS_MOC ${pipeline_HEADERS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

include_directories(${PROJECT_SOURCE_DIR})

# *** Library ***
add_library(mvl_stereo_pipeline SHARED ${pipeline_SOURCES} ${pipeline_HEADERS_MOC})
target_link_libraries(mvl_stereo_pipeline ${QT_LIBRARIES})

install(TARGETS mvl_stereo_pipeline DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${pipeline_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mvl_stereo_pipeline)

# Pkg-config for library
configure_file(${PROJECT_SOURCE_DIR}/libmvl_stereo_pipeline.pc.in ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc)
install(FILES ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# *** Bundled plugins ***
set(MVL_STEREO_PIPELINE_PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/mvl-stereo-pipeline" CACHE PATH "Path to MVL Stereo Pipeline plugins")
add_definitions(-DMVL_STEREO_PIPELINE_PLUGIN_DIR="${MVL_STEREO_PIPELINE_PLUGIN_DIR}")

# Image source plugins
add_subdirectory(sources/dc1394)
add_subdirectory(sources/image_file)
add_subdirectory(sources/video_file)

# Stereo method plugins
add_subdirectory(methods/elas)
add_subdirectory(methods/opencv_bm)
add_subdirectory(methods/opencv_contrib_var)
add_subdirectory(methods/opencv_gpu_bm)
add_subdirectory(methods/opencv_gpu_bp)
add_subdirectory(methods/opencv_gpu_csbp)
add_subdirectory(methods/opencv_sgbm)
