cmake_minimum_required(VERSION 2.8)
project(MVLStereoToolbox CXX)

# Library
add_subdirectory(pipeline)

# Toolbox
add_subdirectory(toolbox)

# Plugins - image sources
add_subdirectory(sources/dc1394)
add_subdirectory(sources/file)

# Plugins - stereo methods
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/elas)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_bm)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_contrib_var)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_gpu_bm)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_gpu_bp)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_gpu_csbp)
add_subdirectory(${PROJECT_SOURCE_DIR}/methods/opencv_sgbm)
