cmake_minimum_required(VERSION 3.16)

project(libmvl_stereo_pipeline VERSION 2.1.0 LANGUAGES CXX)

# Semantic versioning 2.0.0 (semver.org):
# MAJOR is increased when backwards-incompatible changes are made to
# the API (e.g., removal of interfaces). In this case, MINOR and PATCH
# should be reset to zero.
# MINOR is increased when features are added to the API (e.g., addition
# of interfaces). In this case, PATCH should be reset to zero.
# PATCH is increased for all other changes such as bug-fixes.
set(PROJECT_SOVERSION_MAJOR 0)
set(PROJECT_SOVERSION_MINOR 0)
set(PROJECT_SOVERSION_PATCH 0)
set(PROJECT_SOVERSION ${PROJECT_SOVERSION_MAJOR}.${PROJECT_SOVERSION_MINOR}.${PROJECT_SOVERSION_PATCH})


include(GenerateExportHeader)

set(CMAKE_AUTOMOC TRUE)

find_package(PkgConfig QUIET)
find_package(OpenCV REQUIRED)
find_package(Qt5 COMPONENTS Core Concurrent REQUIRED)

set(pipeline_SOURCES
    calibration_pattern.cpp
    disparity_visualization.cpp
    exception.cpp
    pipeline.cpp
    plugin_manager.cpp
    rectification.cpp
    reprojection.cpp
    utils.cpp
    pipeline-async/element.cpp
    pipeline-async/method_element.cpp
    pipeline-async/rectification_element.cpp
    pipeline-async/reprojection_element.cpp
    pipeline-async/source_element.cpp
    pipeline-async/visualization_element.cpp
)

set(pipeline_HEADERS
    calibration_pattern.h
    disparity_visualization.h
    exception.h
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
    pipeline-async/rectification_element.h
    pipeline-async/reprojection_element.h
    pipeline-async/source_element.h
    pipeline-async/visualization_element.h
)

# Define plugin path
set(MVL_STEREO_PIPELINE_PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/MVL/stereo-pipeline" CACHE PATH "Path to MVL Stereo Pipeline plugins")
add_definitions(-DMVL_STEREO_PIPELINE_PLUGIN_DIR="${MVL_STEREO_PIPELINE_PLUGIN_DIR}")

message(STATUS "MVL stereo pipeline plug-in dir: ${MVL_STEREO_PIPELINE_PLUGIN_DIR}")

# *** Library ***
add_library(mvl_stereo_pipeline SHARED ${pipeline_SOURCES} ${pipeline_HEADERS})
target_link_libraries(mvl_stereo_pipeline PUBLIC Qt5::Core PRIVATE Qt5::Concurrent)
target_link_libraries(mvl_stereo_pipeline PUBLIC opencv_core PRIVATE opencv_calib3d)
if(OPENCV_CUDASTEREO_FOUND)
    target_link_libraries(mvl_stereo_pipeline PRIVATE opencv_cudastereo)
endif()

set_target_properties(mvl_stereo_pipeline PROPERTIES
    VERSION ${PROJECT_SOVERSION}
    SOVERSION ${PROJECT_SOVERSION_MAJOR}
)

generate_export_header(mvl_stereo_pipeline EXPORT_FILE_NAME export.h)

target_include_directories(mvl_stereo_pipeline PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/..>
    $<INSTALL_INTERFACE:include/MVL>
)

install(TARGETS mvl_stereo_pipeline EXPORT mvl_stereo_pipeline_targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${pipeline_HEADERS} ${PROJECT_BINARY_DIR}/export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MVL/stereo-pipeline)

# Pkg-config for library; only if we have pkg-config installed
if(PKG_CONFIG_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/libmvl_stereo_pipeline.pc.in ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# CMake config file
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

export(EXPORT mvl_stereo_pipeline_targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-targets.cmake
)

configure_package_config_file(libvml_stereo_pipeline-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_pipeline
    PATH_VARS MVL_STEREO_PIPELINE_PLUGIN_DIR)

install(
    EXPORT mvl_stereo_pipeline_targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_pipeline
    FILE libmvl_stereo_pipeline-targets.cmake
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_pipeline-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_pipeline)

# *** Bundled plugins ***
# Image file pair source: always build
add_subdirectory(sources/image_file)

# Video file source: always build
add_subdirectory(sources/video_file)

# OpenCV Cam image pair source: always build
add_subdirectory(sources/opencv_cam)

# MPO file pair source: always build
add_subdirectory(sources/mpo_file)

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

# OpenCV binary-descriptor-based stereo methods: build if corresponding module is available
if(OPENCV_STEREO_FOUND)
    add_subdirectory(methods/opencv_binary_bm)
endif()
