cmake_minimum_required(VERSION 2.8)
project(libmvl_stereo_pipeline CXX)

include(GNUInstallDirs)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -Wextra -Wno-unused-parameter)
endif()

find_package(PkgConfig)
find_package(OpenCV REQUIRED)
find_package(Qt4 REQUIRED)

set(pipeline_SOURCES
    image_pair_source.cpp
    stereo_calibration_pattern.cpp
    stereo_method.cpp
    stereo_pipeline.cpp
    stereo_rectification.cpp
    stereo_reprojection.cpp
)

set(pipeline_HEADERS
    image_pair_source.h
    plugin_factory.h
    stereo_calibration_pattern.h
    stereo_method.h
    stereo_pipeline.h
    stereo_rectification.h
    stereo_reprojection.h
)

QT4_WRAP_CPP(pipeline_HEADERS_MOC ${pipeline_HEADERS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

include_directories(${PROJECT_SOURCE_DIR})

# If we have OpenCV with GPU support, enable the following definition
if(OPENCV_GPU_FOUND)
    add_definitions(-DHAVE_OPENCV_GPU)
endif()

# *** Library ***
add_library(mvl_stereo_pipeline SHARED ${pipeline_SOURCES} ${pipeline_HEADERS_MOC})
target_link_libraries(mvl_stereo_pipeline ${QT_LIBRARIES})
target_link_libraries(mvl_stereo_pipeline ${OpenCV_LIBS})

install(TARGETS mvl_stereo_pipeline DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${pipeline_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mvl_stereo_pipeline)

# Pkg-config for library; only if we have pkg-config installed
if(PKG_CONFIG_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/libmvl_stereo_pipeline.pc.in ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc)
    install(FILES ${PROJECT_BINARY_DIR}/libmvl_stereo_pipeline.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# *** Bundled plugins ***
set(MVL_STEREO_PIPELINE_PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/mvl-stereo-pipeline" CACHE PATH "Path to MVL Stereo Pipeline plugins")
add_definitions(-DMVL_STEREO_PIPELINE_PLUGIN_DIR="${MVL_STEREO_PIPELINE_PLUGIN_DIR}")

# File image pair source: always build
add_subdirectory(sources/image_file)

# Plugins that require pkg-config (linux-only)
if(PKG_CONFIG_FOUND)
    # DC1394 image pair source: build if we have libdc1394-2
    pkg_check_modules(DC1394 libdc1394-2)
    if(DC1394_FOUND)
        add_subdirectory(sources/dc1394)
    endif()

    # Unicap image pair source: build if we have libunicap
    pkg_check_modules(UNICAP libunicap)
    if (UNICAP_FOUND)
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
add_subdirectory(methods/opencv_contrib_var)
add_subdirectory(methods/opencv_sgbm)

# OpenCV GPU stereo methods: build if we have OpenCV with GPU module
if(OPENCV_GPU_FOUND)
    add_subdirectory(methods/opencv_gpu_bm)
    add_subdirectory(methods/opencv_gpu_bp)
    add_subdirectory(methods/opencv_gpu_csbp)
endif()
