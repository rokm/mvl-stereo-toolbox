cmake_minimum_required(VERSION 2.8)
project(toolbox CXX)

set(TOOLBOX_VERSION 1.0.0)

find_package(OpenCV REQUIRED)
find_package(Qt4 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/../pipeline)

add_definitions(-DTOOLBOX_VERSION="${TOOLBOX_VERSION}")

if(OPENCV_GPU_FOUND)
    add_definitions(-DHAVE_OPENCV_GPU)
endif()

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

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

QT4_WRAP_CPP(toolbox_HEADERS_MOC ${toolbox_HEADERS})

add_executable(MVLStereoToolbox ${toolbox_SOURCES} ${toolbox_HEADERS_MOC})

target_link_libraries(MVLStereoToolbox ${QT_LIBRARIES})
target_link_libraries(MVLStereoToolbox ${OpenCV_LIBS})
target_link_libraries(MVLStereoToolbox mvl_stereo_pipeline)
