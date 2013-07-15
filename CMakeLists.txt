cmake_minimum_required(VERSION 2.8)
project(MVLStereoToolbox CXX)

include(GNUInstallDirs)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -Wextra -Wno-unused-parameter)
endif()

# Library
add_subdirectory(pipeline)

# Toolbox
add_subdirectory(toolbox)

