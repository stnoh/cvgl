##############################################################################
# CMake-based build system for cvgl
# Author: Seung-Tak Noh (seungtak.noh@gmail.com)
##############################################################################
cmake_minimum_required (VERSION 3.1)
project ("cvgl")

########################################
# Handling 3rd party libraries
########################################
find_package(OpenGL REQUIRED)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glew-cmake)


###############################################################################
# Main library (cvgl)
###############################################################################
set(CVGL_HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/cvgl)
set(CVGL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB
  headers
  "${CVGL_HEADER_DIR}/*.h"
)
file(GLOB
  sources
  "${CVGL_SOURCE_DIR}/*.cpp"
)

########################################
## cvgl build
########################################
add_library(cvgl STATIC ${headers} ${sources})

## add header-only libraries
target_include_directories(cvgl PUBLIC
  ${CMAKE_SOURCE_DIR}/external/glm
  ${CMAKE_SOURCE_DIR}/external/RGFW
  ${CMAKE_SOURCE_DIR}/external/glew-cmake/include
)

## link *.lib
target_link_libraries(cvgl
  OpenGL::GLU
  libglew_static
)

##############################################################################
# Sub-directories: add new project as subdirectory
##############################################################################
add_subdirectory(apps/app111)
