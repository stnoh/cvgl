##############################################################################
# CMake-based build system for KinectFusionExplorer-D2D
# Author: Seung-Tak Noh (seungtak.noh@gmail.com)
##############################################################################
file(GLOB
  KinectFusionEx_headers
  "${CMAKE_SOURCE_DIR}/optional/KinectFusionEx/*.h"
)
file(GLOB
  KinectFusionEx_sources
  "${CMAKE_SOURCE_DIR}/optional/KinectFusionEx/*.cpp" "${CMAKE_SOURCE_DIR}/optional/KinectFusionEx/*.cc"
)
add_library(KinectFusionEx STATIC ${KinectFusionEx_headers} ${KinectFusionEx_sources})

target_include_directories(KinectFusionEx PUBLIC
  ${CMAKE_SOURCE_DIR}/optional/KinectFusionEx
  ${CMAKE_SOURCE_DIR}/external/glm
  ${CMAKE_SOURCE_DIR}/external/pugixml/src
)
add_definitions(-DPUGIXML_HEADER_ONLY)

target_link_libraries(KinectFusionEx Kinect2::Fusion protobuf::libprotobuf)
