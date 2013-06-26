cmake_minimum_required(VERSION 2.8)
project(toolbox CXX)

find_package(OpenCV REQUIRED)
find_package(Qt4 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/../pipeline)

if(OPENCV_GPU_FOUND)
    add_definitions(-DHAVE_OPENCV_GPU)
endif()

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

set(toolbox_SOURCES
    main.cpp
    CalibrationPatternDisplayWidget.cpp
    CalibrationWizard.cpp
    GuiImagePairSource.cpp
    GuiRectification.cpp
    GuiStereoMethod.cpp
    GuiReprojection.cpp
    ImageDisplayWidget.cpp
    ImagePairDisplayWidget.cpp
    ReprojectedImageDisplayWidget.cpp
    Toolbox.cpp
)

set(toolbox_HEADERS
    CalibrationPatternDisplayWidget.h
    CalibrationWizard.h
    GuiImagePairSource.h
    GuiRectification.h
    GuiStereoMethod.h
    GuiReprojection.h
    ImageDisplayWidget.h
    ImagePairDisplayWidget.h
    ReprojectedImageDisplayWidget.h
    Toolbox.h
)

QT4_WRAP_CPP(toolbox_HEADERS_MOC ${toolbox_HEADERS})

add_executable(MVLStereoToolbox ${toolbox_SOURCES} ${toolbox_HEADERS_MOC})

target_link_libraries(MVLStereoToolbox ${QT_LIBRARIES})
target_link_libraries(MVLStereoToolbox ${OpenCV_LIBS})
target_link_libraries(MVLStereoToolbox mvl_stereo_pipeline)
