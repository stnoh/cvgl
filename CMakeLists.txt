##############################################################################
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

########################################
# Main library (cvgl)
########################################
file(GLOB
  headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cvgl/*.h
)
file(GLOB
  sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
add_library(cvgl STATIC ${headers} ${sources})

## add header-only library
target_include_directories(cvgl PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/external/glm
)

########################################
## 3rd party libraries 
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
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLFW_BINARIES}	${CMAKE_CURRENT_BINARY_DIR} ## GLFW
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ANTTWEAKBAR_BINARIES}  ${CMAKE_CURRENT_BINARY_DIR}  ## AntTweakBar

    ## OpenCV (minimum)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_core2413.dll   	${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_highgui2413.dll	${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_imgproc2413.dll	${CMAKE_CURRENT_BINARY_DIR}
  )
endmacro()


##############################################################################
# Sub-directories: [TODO] add new project as subdirectory
##############################################################################
add_subdirectory(examples/example101)
add_subdirectory(examples/example102)
add_subdirectory(examples/example103)
