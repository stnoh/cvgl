﻿##############################################################################
# CMake-based build system for cvgl
# Author: Seung-Tak Noh (seungtak.noh@gmail.com)
##############################################################################
cmake_minimum_required (VERSION 3.1)
project ("cvgl")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# to suppress warning on {package}_ROOT in cmake
if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

## for checking unicode
#[[
if(MSVC)
  add_definitions(-DUNICODE -D_UNICODE)
endif()
##]]

## common path for data
set(SHARED_DATA_PATH ${CMAKE_SOURCE_DIR}/data CACHE PATH "location of shared data")
add_definitions(-DSHARED_DATA_PATH=\"${SHARED_DATA_PATH}\")


########################################
# Handling 3rd party libraries
########################################
find_package(OpenGL REQUIRED)

## GLEW (rely on built-in CMAKE module)
set(GLEW_ROOT    ${CMAKE_SOURCE_DIR}/3rdParty/GLEW)
set(GLEW_LIBRARY ${GLEW_ROOT}/lib/x64/glew32.lib)
find_package(GLEW REQUIRED)

## GLFW
set(GLFW_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/GLFW)
find_package(GLFW REQUIRED)

## AntTweakBar
set(ANTTWEAKBAR_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/AntTweakBar)
find_package(ANTTWEAKBAR REQUIRED)

## OpenCV
set(OPENCV_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/OpenCV)
find_package(OPENCV REQUIRED)

## ArUco
set(ARUCO_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/ArUco)
find_package(ARUCO)

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
## Optional libraries
########################################

## RSSDK
set(RSSDK_ROOT "C:/Program Files (x86)/Intel/RSSDK")
find_package(RSSDK)

if(NOT RSSDK_FOUND)
  list(REMOVE_ITEM headers ${CVGL_HEADER_DIR}/RSCamera.h)
  list(REMOVE_ITEM sources ${CVGL_SOURCE_DIR}/RSCamera.cpp)
endif()

## Kinect2
set(KINECT2_ROOT "C:/Program Files/Microsoft SDKs/Kinect/v2.0_1409")
find_package(KINECT2)

if(NOT KINECT2_FOUND)
  list(REMOVE_ITEM headers ${CVGL_HEADER_DIR}/Kinect2.h)
  list(REMOVE_ITEM sources ${CVGL_SOURCE_DIR}/Kinect2.cpp)
endif()

## protocol buffer
set(PROTOBUF_ROOT ${CMAKE_SOURCE_DIR}/optional/protobuf)
find_package(PROTOBUF)

########################################
## cvgl build
########################################
add_library(cvgl STATIC ${headers} ${sources})

## add header-only library
target_include_directories(cvgl PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/external/glm
  ${CMAKE_SOURCE_DIR}/external/dirent/include
)

########################################
## 3rd party libraries (mandatory)
########################################
## link *.lib
target_link_libraries(cvgl
  OpenGL::GLU GLEW::GLEW GLFW::GLFW AntTweakBar::Lib
  OpenCV::Core OpenCV::HighGUI OpenCV::ImgProc
)

## copy *.dll macro as post build process
macro(copy_3rdparty_dlls APP)
  add_custom_command(TARGET ${APP} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLEW_ROOT}/bin64/glew32.dll	${CMAKE_CURRENT_BINARY_DIR}	## GLEW
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLFW_BINARIES}        ${CMAKE_CURRENT_BINARY_DIR} ## GLFW
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ANTTWEAKBAR_BINARIES} ${CMAKE_CURRENT_BINARY_DIR} ## AntTweakBar

    ## OpenCV (minimum)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_core2413.dll   	${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_highgui2413.dll	${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_imgproc2413.dll	${CMAKE_CURRENT_BINARY_DIR}
  )
endmacro()

## RSSDK
if(RSSDK_FOUND)
  target_include_directories(cvgl PUBLIC
    ${RSSDK_INCLUDE_DIR}
  )
endif()

## Kinect2
if(KINECT2_FOUND)
  target_include_directories(cvgl PUBLIC
    ${KINECT2_INCLUDE_DIR}
  )
endif()

########################################
## KinectFusion extension
########################################
if(KINECT2_FOUND AND PROTOBUF_FOUND)
  add_subdirectory(optional/KinectFusionEx)
endif()

##############################################################################
# Sub-directories: [TODO] add new project as subdirectory
##############################################################################
add_subdirectory(examples/example101)
add_subdirectory(examples/example102)
add_subdirectory(examples/example103)

if(RSSDK_FOUND)
  add_subdirectory(examples/example201)
  if(ARUCO_FOUND)
	add_subdirectory(examples/example301)
  endif()
endif()

if(KINECT2_FOUND)
  add_subdirectory(examples/example202)
endif()

if(KINECT2_FOUND AND PROTOBUF_FOUND)
  add_subdirectory(examples/example203)
endif()

if(KINECT2_FOUND AND PROTOBUF_FOUND AND RSSDK_FOUND)
  add_subdirectory(optional/KinectFusionExplorer-D2D)
endif()
