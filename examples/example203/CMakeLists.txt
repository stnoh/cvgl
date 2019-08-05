add_executable (example203 "Example203.cpp")

target_link_libraries(example203 cvgl KinectFusionEx)

add_custom_command(TARGET example203 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KINECT2_ROOT}/Redist/Fusion/x64/Kinect20.Fusion.dll ${CMAKE_CURRENT_BINARY_DIR}
)

copy_3rdparty_dlls(example203)
