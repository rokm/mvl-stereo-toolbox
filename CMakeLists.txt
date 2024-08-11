cmake_minimum_required(VERSION 3.16)

project(MVLStereoToolbox VERSION 2.1.0 LANGUAGES CXX)

option(WITH_GUI "Disable GUI elements (builds only stereo pipeline library)" ON)

include(GNUInstallDirs)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)

# Extra warnings on gcc
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -Wextra -Wno-unused-parameter)
endif()

# Automatically include current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Stereo pipeline library
add_subdirectory(stereo-pipeline)

if(WITH_GUI)
    # Stereo widgets library
    add_subdirectory(stereo-widgets)

    # Toolbox
    add_subdirectory(toolbox)
endif()

