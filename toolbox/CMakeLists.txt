cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

project(toolbox CXX)

set(CMAKE_AUTOMOC TRUE)

find_package(OpenCV 3.0.0 REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(toolbox_SOURCES
    calibration_wizard.cpp
    image_display_widget.cpp
    main.cpp
    toolbox.cpp
    window_image_pair_source.cpp
    window_rectification.cpp
    window_reprojection.cpp
    window_stereo_method.cpp
)

set(toolbox_HEADERS
    calibration_wizard.h
    image_display_widget.h
    toolbox.h
    window_image_pair_source.h
    window_rectification.h
    window_reprojection.h
    window_stereo_method.h
)

add_executable(MVLStereoToolbox ${toolbox_SOURCES} ${toolbox_HEADERS})

target_link_libraries(MVLStereoToolbox Qt5::Widgets Qt5::Concurrent)

target_link_libraries(MVLStereoToolbox opencv_core opencv_calib3d)
if(OPENCV_CUDASTEREO_FOUND)
    target_link_libraries(MVLStereoToolbox opencv_cudastereo)
endif()

target_link_libraries(MVLStereoToolbox mvl_stereo_pipeline)

install(TARGETS MVLStereoToolbox DESTINATION ${CMAKE_INSTALL_BINDIR})
