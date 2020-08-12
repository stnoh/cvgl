add_executable (example301 "Example301.cpp" "ArUcoCalib.cpp")

target_link_libraries(example301 cvgl RSSDK::Core OpenCV::Calib3d OpenCV::Features2d OpenCV::Flann OpenCV::ML ArUco::Lib)

## copy *.dll for camera calibration
add_custom_command(TARGET example301 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_calib3d2413.dll	${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_features2d2413.dll	${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_flann2413.dll	${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_BINARY_DIR}/opencv_ml2413.dll	${CMAKE_CURRENT_BINARY_DIR}

  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ARUCO_BINARIES} ${CMAKE_CURRENT_BINARY_DIR} ## ArUco has single binary dll
)

copy_3rdparty_dlls(example301)
