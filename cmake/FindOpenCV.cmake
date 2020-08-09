# [TEMPORARY CMAKE FILE FOR OpenCV]

# OPENCV_FOUND
# OPENCV_INCLUDE_DIR
# OPENCV_LIBRARY_DIR
# OPENCV_BINARY_DIR

find_path (OPENCV_INCLUDE_DIR
  NAMES
    opencv2/opencv.hpp
  PATHS
    "${OPENCV_ROOT}/include"
  DOC
    "The directory where OpenCV2/opencv.hpp resides"
)

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
  find_path (OPENCV_LIBRARY_DIR
    NAMES
      opencv_core2413.lib
      opencv_highgui2413.lib
      opencv_imgproc2413.lib
      opencv_calib3d2413.lib
      opencv_features2d2413.lib
      opencv_flann2413.lib
      opencv_ml2413.lib
    PATHS
      "${OPENCV_ROOT}/lib/x64"
    DOC
      "The directory where opencv_*2413.lib resides"
  )
else()
	message(WARNING "We do not support this environment yet.")
endif()

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
  find_path (OPENCV_BINARY_DIR
    NAMES
      opencv_core2413.dll
      opencv_highgui2413.dll
      opencv_imgproc2413.dll
      opencv_calib3d2413.dll
      opencv_features2d2413.dll
      opencv_flann2413.dll
      opencv_ml2413.dll
    PATHS
      "${OPENCV_ROOT}/bin64"
    DOC
      "The directory where opencv_*2413.dll resides"
  )
else()
  message(WARNING "We do not support this environment yet.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenCV DEFAULT_MSG OPENCV_LIBRARY_DIR OPENCV_INCLUDE_DIR)

if(OPENCV_FOUND)
  set(OPENCV_INCLUDE_DIR ${OPENCV_INCLUDE_DIR})

  if(NOT OPENCV_LIBRARY_DIR)
    set(OPENCV_LIBRARY_DIR ${OPENCV_LIBRARY_DIR})
  endif()

  if(NOT OPENCV_BINARY_DIR)
    set(OPENCV_BINARY_DIR  ${OPENCV_BINARY_DIR} )
  endif()

  if (NOT TARGET OpenCV::Core)
    add_library(OpenCV::Core MODULE IMPORTED)
    set_target_properties(OpenCV::Core PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE_DIR}"
      IMPORTED_LOCATION "${OPENCV_LIBRARY_DIR}/opencv_core2413.lib")

    add_library(OpenCV::HighGUI MODULE IMPORTED)
    set_target_properties(OpenCV::HighGUI PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE_DIR}"
      IMPORTED_LOCATION "${OPENCV_LIBRARY_DIR}/opencv_highgui2413.lib")

    add_library(OpenCV::ImgProc MODULE IMPORTED)
    set_target_properties(OpenCV::ImgProc PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE_DIR}"
      IMPORTED_LOCATION "${OPENCV_LIBRARY_DIR}/opencv_imgproc2413.lib")

    add_library(OpenCV::Calib3d MODULE IMPORTED)
    set_target_properties(OpenCV::Calib3d PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE_DIR}"
      IMPORTED_LOCATION "${OPENCV_LIBRARY_DIR}/opencv_calib3d2413.lib")

    add_library(OpenCV::Features2d MODULE IMPORTED)
    set_target_properties(OpenCV::Features2d PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE_DIR}"
      IMPORTED_LOCATION "${OPENCV_LIBRARY_DIR}/opencv_features2d2413.lib")

    add_library(OpenCV::Flann MODULE IMPORTED)
    set_target_properties(OpenCV::Flann PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE_DIR}"
      IMPORTED_LOCATION "${OPENCV_LIBRARY_DIR}/opencv_flann2413.lib")

    add_library(OpenCV::ML MODULE IMPORTED)
    set_target_properties(OpenCV::ML PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE_DIR}"
      IMPORTED_LOCATION "${OPENCV_LIBRARY_DIR}/opencv_ml2413.lib")
  endif()

endif()

mark_as_advanced(OPENCV_INCLUDE_DIR OPENCV_LIBRARY_DIR)
