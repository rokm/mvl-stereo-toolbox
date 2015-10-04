cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

project(libmvl_stereo_widgets CXX)

include(GenerateExportHeader)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC TRUE)

find_package(PkgConfig QUIET)
find_package(OpenCV 3.0.0 REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(widgets_SOURCES
    calibration_pattern_display_widget.cpp
    disparity_display_widget.cpp
    image_display_widget.cpp
    image_pair_display_widget.cpp
    reprojection_display_widget.cpp
)

set(widgets_HEADERS
    calibration_pattern_display_widget.h
    disparity_display_widget.h
    image_display_widget.h
    image_pair_display_widget.h
    reprojection_display_widget.h
)


include_directories(${OpenCV_INCLUDE_DIRS})

# *** Library ***
add_library(mvl_stereo_widgets SHARED ${widgets_SOURCES} ${widgets_HEADERS})
target_link_libraries(mvl_stereo_widgets Qt5::Core Qt5::Widgets)
target_link_libraries(mvl_stereo_widgets opencv_core)

generate_export_header(mvl_stereo_widgets EXPORT_FILE_NAME export.h)

install(TARGETS mvl_stereo_widgets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${widgets_HEADERS} ${PROJECT_BINARY_DIR}/export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MVL/stereo-widgets)

# Pkg-config for library; only if we have pkg-config installed
if(PKG_CONFIG_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/libmvl_stereo_widgets.pc.in ${PROJECT_BINARY_DIR}/libmvl_stereo_widgets.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/libmvl_stereo_widgets.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
