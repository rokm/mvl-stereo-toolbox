cmake_minimum_required(VERSION 2.8)
project(libmvl_stereo_pipeline CXX)

include(GenerateExportHeader)

set(CMAKE_AUTOMOC TRUE)

find_package(PkgConfig QUIET)
find_package(OpenCV 3.0.0 REQUIRED)
find_package(Qt5Core REQUIRED)

find_package(CUDA 5.0)

set(pipeline_SOURCES
    calibration_pattern.cpp
    pipeline.cpp
    plugin_manager.cpp
    rectification.cpp
    reprojection.cpp
    utils.cpp
)

set(pipeline_HEADERS
    calibration_pattern.h
    image_pair_source.h
    plugin_factory.h
    plugin_manager.h
    pipeline.h
    rectification.h
    reprojection.h
    stereo_method.h
    utils.h
)

set(pipeline_CUDA_SOURCES
    reprojection.cu
)

include_directories(${OpenCV_INCLUDE_DIRS})

# If we have OpenCV with CUDA and CUDA compiler, enable our own CUDA
# sources
if(CUDA_FOUND)
    add_definitions(-DHAVE_CUDA)
    set(CUDA_HOST_COMPILER "${CMAKE_CXX_COMPILER}")
    if (CMAKE_COMPILER_IS_GNUCXX)
        list(APPEND CUDA_NVCC_FLAGS "-Xcompiler -fPIC")
    endif()
    include_directories(${CUDA_TOOLKIT_INCLUDE})
endif()

if (OPENCV_CUDEV_FOUND AND CUDA_FOUND)
    cuda_compile(pipeline_CUDA_OBJECTS ${pipeline_CUDA_SOURCES})
else ()
    set(pipeline_CUDA_OBJECTS "")
endif()

# *** Library ***
add_library(mvl_stereo_pipeline SHARED ${pipeline_SOURCES} ${pipeline_HEADERS} ${pipeline_CUDA_SOURCES} ${pipeline_CUDA_OBJECTS})
qt5_use_modules(mvl_stereo_pipeline Core Concurrent)
target_link_libraries(mvl_stereo_pipeline opencv_core)
if(CUDA_FOUND)
    target_link_libraries(mvl_stereo_pipeline ${CUDA_CUDART_LIBRARY})
endif()

generate_export_header(mvl_stereo_pipeline EXPORT_FILE_NAME export.h)

install(TARGETS mvl_stereo_pipeline DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${pipeline_HEADERS} ${PROJECT_BINARY_DIR}/export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MVL/stereo-pipeline)

# Pkg-config for library; only if we have pkg-config installed
if(PKG_CONFIG_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/libmvl_stereo_pipeline.pc.in ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# *** Bundled plugins ***
set(MVL_STEREO_PIPELINE_PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/MVL/stereo-pipeline" CACHE PATH "Path to MVL Stereo Pipeline plugins")
add_definitions(-DMVL_STEREO_PIPELINE_PLUGIN_DIR="${MVL_STEREO_PIPELINE_PLUGIN_DIR}")

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
