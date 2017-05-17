cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

project(libmvl_stereo_pipeline CXX)

include(GenerateExportHeader)

set(CMAKE_AUTOMOC TRUE)

find_package(PkgConfig QUIET)
find_package(OpenCV 3.0.0 REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)

set(pipeline_SOURCES
    calibration_pattern.cpp
    disparity_visualization.cpp
    pipeline.cpp
    plugin_manager.cpp
    rectification.cpp
    reprojection.cpp
    utils.cpp
    pipeline-async/element.cpp
    pipeline-async/method_element.cpp
    pipeline-async/source_element.cpp
)

set(pipeline_HEADERS
    calibration_pattern.h
    disparity_visualization.h
    image_pair_source.h
    plugin_factory.h
    plugin_manager.h
    pipeline.h
    rectification.h
    reprojection.h
    stereo_method.h
    utils.h
    pipeline-async/element.h
    pipeline-async/method_element.h
    pipeline-async/source_element.h
)

include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(pipeline-async)

# Define plugin path
set(MVL_STEREO_PIPELINE_PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/MVL/stereo-pipeline" CACHE PATH "Path to MVL Stereo Pipeline plugins")
add_definitions(-DMVL_STEREO_PIPELINE_PLUGIN_DIR="${MVL_STEREO_PIPELINE_PLUGIN_DIR}")

# *** Library ***
add_library(mvl_stereo_pipeline SHARED ${pipeline_SOURCES} ${pipeline_HEADERS})
target_link_libraries(mvl_stereo_pipeline PUBLIC Qt5::Core PRIVATE Qt5::Concurrent)
target_link_libraries(mvl_stereo_pipeline PUBLIC opencv_core PRIVATE opencv_calib3d)
if(OPENCV_CUDASTEREO_FOUND)
    target_link_libraries(mvl_stereo_pipeline PRIVATE opencv_cudastereo)
endif()

generate_export_header(mvl_stereo_pipeline EXPORT_FILE_NAME export.h)

install(TARGETS mvl_stereo_pipeline EXPORT mvl_stereo_pipeline_targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${pipeline_HEADERS} ${PROJECT_BINARY_DIR}/export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MVL/stereo-pipeline)

# Pkg-config for library; only if we have pkg-config installed
if(PKG_CONFIG_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/libmvl_stereo_pipeline.pc.in ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# CMake config file
include(CMakePackageConfigHelpers)

set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/MVL)

configure_package_config_file(libvml_stereo_pipeline-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_pipeline
    PATH_VARS LIB_INSTALL_DIR INCLUDE_INSTALL_DIR MVL_STEREO_PIPELINE_PLUGIN_DIR)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_pipeline)

install(
    EXPORT mvl_stereo_pipeline_targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_pipeline
    FILE libmvl_stereo_pipeline-targets.cmake
)

# *** Bundled plugins ***
# Image file pair source: always build
add_subdirectory(sources/image_file)

# Video file source: always build
add_subdirectory(sources/video_file)

# OpenCV Cam image pair source: always build
add_subdirectory(sources/opencv_cam)

# Plugins that require pkg-config (linux-only)
if(PKG_CONFIG_FOUND)
    # DC1394 image pair source: build if we have libdc1394-2
    pkg_check_modules(DC1394 libdc1394-2)
    if(DC1394_FOUND)
        add_subdirectory(sources/dc1394)
    endif()

    # Unicap image pair source: build if we have libunicap
    pkg_check_modules(UNICAP libunicap)
    if(UNICAP_FOUND)
        add_subdirectory(sources/unicap)
    endif()

    # ELAS stereo method: build if we have libelas
    pkg_check_modules(ELAS libelas)
    if(ELAS_FOUND)
        add_subdirectory(methods/elas)
    endif()
endif()

# OpenCV CPU stereo methods: always build
add_subdirectory(methods/opencv_bm)
add_subdirectory(methods/opencv_sgbm)

# OpenCV CUDA stereo methods: build if corresponding module is available
if(OPENCV_CUDASTEREO_FOUND)
    add_subdirectory(methods/opencv_cuda_bm)
    add_subdirectory(methods/opencv_cuda_bp)
    add_subdirectory(methods/opencv_cuda_csbp)
endif()
