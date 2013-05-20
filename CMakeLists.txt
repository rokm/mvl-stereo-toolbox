cmake_minimum_required(VERSION 2.8)
project(MVLStereoToolbox CXX)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core imgproc gpu highgui calib3d)
find_package(Qt4 REQUIRED)

# Library subdir
add_subdirectory(pipeline)

# Sources
add_subdirectory(sources/dc1394)
add_subdirectory(sources/file)

# Methods
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/elas)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_bm)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_contrib_var)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_gpu_bm)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_gpu_bp)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_gpu_csbp)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_sgbm)

include_directories(${PROJECT_SOURCE_DIR}/pipeline)

# Toolbox
include_directories(${PROJECT_SOURCE_DIR}/gui)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

set(toolbox_SOURCES
    main.cpp
    gui/GuiImageSource.cpp
    gui/GuiCalibration.cpp
    gui/ImageDisplayWidget.cpp
    gui/GuiStereoMethod.cpp
    gui/ImagePairDisplayWidget.cpp
)

set(toolbox_HEADERS
    gui/GuiImageSource.h
    gui/GuiCalibration.h
    gui/ImageDisplayWidget.h
    gui/GuiStereoMethod.h
    gui/ImagePairDisplayWidget.h
)

QT4_WRAP_CPP(toolbox_HEADERS_MOC ${toolbox_HEADERS})

add_executable(MVLStereoToolbox ${toolbox_SOURCES} ${toolbox_HEADERS_MOC})

target_link_libraries(MVLStereoToolbox ${QT_LIBRARIES})
target_link_libraries(MVLStereoToolbox ${OpenCV_LIBS})
target_link_libraries(MVLStereoToolbox stereo_pipeline)
