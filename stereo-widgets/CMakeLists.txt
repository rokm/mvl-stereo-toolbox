cmake_minimum_required(VERSION 3.2)

project(libmvl_stereo_widgets VERSION 2.1.0 LANGUAGES CXX)

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
set(CMAKE_AUTORCC TRUE)

find_package(PkgConfig QUIET)
find_package(OpenCV REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(widgets_SOURCES
    calibration_pattern_display_widget.cpp
    disparity_display_widget.cpp
    image_display_widget.cpp
    image_pair_display_widget.cpp
    reprojection_display_widget.cpp

    point_cloud_visualization_widget.cpp

    stereo-widgets.qrc
)

set(widgets_HEADERS
    calibration_pattern_display_widget.h
    disparity_display_widget.h
    image_display_widget.h
    image_pair_display_widget.h
    reprojection_display_widget.h

    point_cloud_visualization_widget.h
)


include_directories(${OpenCV_INCLUDE_DIRS})

# *** Library ***
add_library(mvl_stereo_widgets SHARED ${widgets_SOURCES} ${widgets_HEADERS})
target_link_libraries(mvl_stereo_widgets PUBLIC Qt5::Core Qt5::Widgets)
target_link_libraries(mvl_stereo_widgets PUBLIC opencv_core opencv_imgproc)

set_target_properties(mvl_stereo_widgets PROPERTIES
    VERSION ${PROJECT_SOVERSION}
    SOVERSION ${PROJECT_SOVERSION_MAJOR}
)

generate_export_header(mvl_stereo_widgets EXPORT_FILE_NAME export.h)

install(TARGETS mvl_stereo_widgets EXPORT mvl_stereo_widgets_targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${widgets_HEADERS} ${PROJECT_BINARY_DIR}/export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MVL/stereo-widgets)

# Pkg-config for library; only if we have pkg-config installed
if(PKG_CONFIG_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/libmvl_stereo_widgets.pc.in ${PROJECT_BINARY_DIR}/libmvl_stereo_widgets.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/libmvl_stereo_widgets.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# CMake config file
include(CMakePackageConfigHelpers)

set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/MVL)

configure_package_config_file(libmvl_stereo_widgets-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_widgets-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_widgets
    PATH_VARS LIB_INSTALL_DIR INCLUDE_INSTALL_DIR)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_widgets-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_widgets-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libmvl_stereo_widgets-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_widgets)

install(
    EXPORT mvl_stereo_widgets_targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmvl_stereo_widgets
    FILE libmvl_stereo_widgets-targets.cmake
)

