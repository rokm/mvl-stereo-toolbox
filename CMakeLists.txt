cmake_minimum_required(VERSION 2.8)
project(MVLStereoToolbox CXX)

include(GNUInstallDirs)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -Wextra -Wno-unused-parameter)
endif()

# Automatically include current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Library
add_subdirectory(pipeline)

# Toolbox
add_subdirectory(toolbox)

