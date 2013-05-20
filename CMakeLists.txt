cmake_minimum_required(VERSION 2.8)
project(MVLStereoToolbox CXX)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core imgproc gpu highgui contrib calib3d)
find_package(Qt4 REQUIRED)

pkg_check_modules(ELAS REQUIRED libelas)

# Library subdir
add_subdirectory(pipeline)



include_directories(${PROJECT_SOURCE_DIR}/gui)
include_directories(${PROJECT_SOURCE_DIR}/pipeline)
include_directories(${PROJECT_SOURCE_DIR}/methods/opencv_sgbm)
include_directories(${PROJECT_SOURCE_DIR}/methods/opencv_bm)
include_directories(${PROJECT_SOURCE_DIR}/methods/opencv_bm_gpu)
include_directories(${PROJECT_SOURCE_DIR}/methods/opencv_var)
include_directories(${PROJECT_SOURCE_DIR}/methods/elas)
include_directories(${PROJECT_SOURCE_DIR}/methods/opencv_csbp_gpu)
include_directories(${PROJECT_SOURCE_DIR}/methods/opencv_bp_gpu)
include_directories(${PROJECT_SOURCE_DIR}/sources/dc1394)
include_directories(${PROJECT_SOURCE_DIR}/sources/file)

include_directories(${ELAS_INCLUDE_DIRS})
link_directories(${ELAS_LIBRARY_DIRS})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

set(toolbox_SOURCES
    main.cpp
    gui/GuiImageSource.cpp
    gui/GuiCalibration.cpp
    gui/ImageDisplayWidget.cpp
    gui/GuiStereoMethod.cpp
    gui/ImagePairDisplayWidget.cpp
methods/opencv_sgbm/StereoMethodSemiGlobalBlockMatching.cpp
methods/opencv_bm/StereoMethodBlockMatching.cpp
methods/opencv_bm_gpu/StereoMethodBlockMatchingGPU.cpp
methods/opencv_var/StereoMethodVar.cpp
methods/elas/StereoMethodELAS.cpp
methods/opencv_csbp_gpu/StereoMethodConstantSpaceBeliefPropagationGPU.cpp
methods/opencv_bp_gpu/StereoMethodBeliefPropagationGPU.cpp
    sources/dc1394/ImageSourceDC1394.cpp
    sources/file/ImageSourceFile.cpp
)

set(toolbox_HEADERS
    gui/GuiImageSource.h
    gui/GuiCalibration.h
    gui/ImageDisplayWidget.h
    gui/GuiStereoMethod.h
    gui/ImagePairDisplayWidget.h
methods/opencv_sgbm/StereoMethodSemiGlobalBlockMatching.h
methods/opencv_bm/StereoMethodBlockMatching.h
methods/opencv_bm_gpu/StereoMethodBlockMatchingGPU.h
methods/opencv_var/StereoMethodVar.h
methods/elas/StereoMethodELAS.h
methods/opencv_csbp_gpu/StereoMethodConstantSpaceBeliefPropagationGPU.h
methods/opencv_bp_gpu/StereoMethodBeliefPropagationGPU.h
    sources/dc1394/ImageSourceDC1394.h
    sources/file/ImageSourceFile.h

)

QT4_WRAP_CPP(toolbox_HEADERS_MOC ${toolbox_HEADERS})



add_executable(MVLStereoToolbox ${toolbox_SOURCES} ${toolbox_HEADERS_MOC})
target_link_libraries(MVLStereoToolbox ${QT_LIBRARIES})
target_link_libraries(MVLStereoToolbox ${OpenCV_LIBS})
target_link_libraries(MVLStereoToolbox stereo_pipeline)
target_link_libraries(MVLStereoToolbox ${ELAS_LIBRARIES})
