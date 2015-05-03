cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

project(MVLStereoToolbox CXX)
set(PROJECT_VERSION 2.0.0)

include(GNUInstallDirs)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++0x)
    add_definitions(-Wall -Wextra -Wno-unused-parameter)
endif()

# Project version
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")

# Automatically include current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

# Library
add_subdirectory(stereo-pipeline)

# Toolbox
add_subdirectory(toolbox)

