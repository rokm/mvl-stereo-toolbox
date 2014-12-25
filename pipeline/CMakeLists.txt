cmake_minimum_required(VERSION 2.8)
project(libmvl_stereo_pipeline CXX)
set(PROJECT_VERSION "1.0.0")

include(GenerateExportHeader)

set(CMAKE_AUTOMOC TRUE)

find_package(PkgConfig QUIET)
find_package(OpenCV REQUIRED)
find_package(Qt5Core REQUIRED)

find_package(CUDA 5.0)

set(pipeline_SOURCES
    plugin_manager.cpp
    stereo_calibration_pattern.cpp
    stereo_pipeline.cpp
    stereo_rectification.cpp
    stereo_reprojection.cpp
)

set(pipeline_HEADERS
    image_pair_source.h
    plugin_factory.h
    plugin_manager.h
    stereo_calibration_pattern.h
    stereo_method.h
    stereo_pipeline.h
    stereo_rectification.h
    stereo_reprojection.h
)

set(pipeline_CUDA_SOURCES
    stereo_reprojection.cu
)

# These are mostly for benefit of methods and sources plugins
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

include_directories(${OpenCV_INCLUDE_DIRS})

# If we have OpenCV with GPU support, enable the following definition
if(OPENCV_GPU_FOUND)
    add_definitions(-DHAVE_OPENCV_GPU)
endif()

if(CUDA_FOUND)
    add_definitions(-DHAVE_CUDA)
    set(CUDA_HOST_COMPILER "${CMAKE_CXX_COMPILER}")
    if (CMAKE_COMPILER_IS_GNUCXX)
        list(APPEND CUDA_NVCC_FLAGS "-Xcompiler -fPIC")
    endif()
    include_directories(${CUDA_TOOLKIT_INCLUDE})
endif()

if (OPENCV_GPU_FOUND AND CUDA_FOUND)
    cuda_compile(pipeline_CUDA_OBJECTS ${pipeline_CUDA_SOURCES})
else ()
    set(pipeline_CUDA_OBJECTS "")
endif()

# *** Library ***
add_library(mvl_stereo_pipeline SHARED ${pipeline_SOURCES} ${pipeline_HEADERS} ${pipeline_CUDA_SOURCES} ${pipeline_CUDA_OBJECTS})
qt5_use_modules(mvl_stereo_pipeline Core Concurrent)
target_link_libraries(mvl_stereo_pipeline ${OpenCV_LIBS})
if(CUDA_FOUND)
    target_link_libraries(mvl_stereo_pipeline ${CUDA_CUDART_LIBRARY})
endif()

generate_export_header(mvl_stereo_pipeline)

install(TARGETS mvl_stereo_pipeline DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${pipeline_HEADERS} ${PROJECT_BINARY_DIR}/mvl_stereo_pipeline_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mvl_stereo_pipeline)

# Pkg-config for library; only if we have pkg-config installed
if(PKG_CONFIG_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/libmvl_stereo_pipeline.pc.in ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# *** Bundled plugins ***
set(MVL_STEREO_PIPELINE_PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/mvl-stereo-pipeline" CACHE PATH "Path to MVL Stereo Pipeline plugins")
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
#add_subdirectory(methods/opencv_sgbm)

#if(OPENCV_CONTRIB_FOUND)
#    add_subdirectory(methods/opencv_contrib_var)
#endif()

# OpenCV GPU stereo methods: build if we have OpenCV with GPU module
#if(OPENCV_GPU_FOUND)
#    add_subdirectory(methods/opencv_gpu_bm)
#    add_subdirectory(methods/opencv_gpu_bp)
#    add_subdirectory(methods/opencv_gpu_csbp)
#endif()
