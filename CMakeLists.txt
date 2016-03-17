cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

project(MVLStereoToolbox C CXX)
set(PROJECT_VERSION 2.0.0)

option(WITH_GUI "Disable GUI elements (builds only stereo pipeline library)" ON)

include(GNUInstallDirs)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
    add_definitions(-Wall -Wextra -Wno-unused-parameter)
endif()

# Project version
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")

# Automatically include current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

# Stereo pipeline library
add_subdirectory(stereo-pipeline)

if(WITH_GUI)
    # Stereo widgets library
    add_subdirectory(stereo-widgets)

    # Toolbox
    add_subdirectory(toolbox)
endif()

