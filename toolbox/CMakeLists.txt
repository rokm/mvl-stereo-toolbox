cmake_minimum_required(VERSION 3.2)

project(toolbox VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC TRUE)

find_package(OpenCV 3.0.0 REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(toolbox_SOURCES
    calibration-wizard/calibration_flags_widget.cpp
    calibration-wizard/camera_parameters_widget.cpp
    calibration-wizard/page_calibration.cpp
    calibration-wizard/page_detection.cpp
    calibration-wizard/page_images.cpp
    calibration-wizard/page_introduction.cpp
    calibration-wizard/page_result.cpp
    calibration-wizard/wizard.cpp
    main.cpp
    toolbox.cpp
    window_image_pair_source.cpp
    window_point_cloud.cpp
    window_rectification.cpp
    window_reprojection.cpp
    window_stereo_method.cpp
)

set(toolbox_HEADERS
    calibration-wizard/calibration_flags_widget.h
    calibration-wizard/camera_parameters_widget.h
    calibration-wizard/page_calibration.h
    calibration-wizard/page_detection.h
    calibration-wizard/page_images.h
    calibration-wizard/page_introduction.h
    calibration-wizard/page_result.h
    calibration-wizard/wizard.h
    toolbox.h
    window_image_pair_source.h
    window_point_cloud.h
    window_rectification.h
    window_reprojection.h
    window_stereo_method.h
)

add_executable(MVLStereoToolbox ${toolbox_SOURCES} ${toolbox_HEADERS})

target_compile_definitions(MVLStereoToolbox PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(MVLStereoToolbox PRIVATE Qt5::Widgets Qt5::Concurrent)

target_link_libraries(MVLStereoToolbox PRIVATE opencv_core opencv_calib3d opencv_imgcodecs)

target_link_libraries(MVLStereoToolbox PRIVATE mvl_stereo_pipeline)
target_link_libraries(MVLStereoToolbox PRIVATE mvl_stereo_widgets)

install(TARGETS MVLStereoToolbox DESTINATION ${CMAKE_INSTALL_BINDIR})
