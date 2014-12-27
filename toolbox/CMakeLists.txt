cmake_minimum_required(VERSION 2.8)
project(toolbox CXX)

set(CMAKE_AUTOMOC TRUE)

find_package(OpenCV REQUIRED)
find_package(Qt5Core REQUIRED)

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

qt5_use_modules(MVLStereoToolbox Widgets Concurrent)

target_link_libraries(MVLStereoToolbox ${OpenCV_LIBS})
target_link_libraries(MVLStereoToolbox mvl_stereo_pipeline)

install(TARGETS MVLStereoToolbox DESTINATION ${CMAKE_INSTALL_BINDIR})
