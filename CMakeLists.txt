cmake_minimum_required (VERSION 2.8)
project (concurrent)

# The version number.
set (Functional_VERSION_MAJOR 0)
set (Functional_VERSION_MINOR 1)
 
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/ConcConfig.h.in"
  "${PROJECT_BINARY_DIR}/ConcConfig.h"
  )
 
# add the binary tree to the search path for include files
# so that we will find ConcConfig.h
include_directories("${PROJECT_BINARY_DIR}")

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(Concurrent Main.cpp)