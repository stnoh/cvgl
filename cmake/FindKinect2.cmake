# [TEMPORARY CMAKE FILE FOR Kinect2]

# KINECT2_FOUND
# KINECT2_INCLUDE_DIR
# KINECT2_LIBRARY_DIR

find_path (KINECT2_INCLUDE_DIR
    NAMES
        Kinect.h
    PATHS
        "${KINECT2_ROOT}/inc"
    DOC
        "The directory where Kinect.h resides"
)

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
    find_path (KINECT2_LIBRARY_DIR
        NAMES
			Kinect20.lib
			Kinect20.Fusion.lib
        PATHS
			"${KINECT2_ROOT}/Lib/x64"
        DOC
			"The directory where Kinect20.*.lib resides"
    )
else()
	message(WARNING "Kinect2-SDK is only supported in Windows.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Kinect2 DEFAULT_MSG KINECT2_LIBRARY_DIR KINECT2_INCLUDE_DIR)

if(KINECT2_FOUND)
  set(KINECT2_INCLUDE_DIR ${KINECT2_INCLUDE_DIR})
  set(KINECT2_LIBRARY_DIR ${KINECT2_LIBRARY_DIR})

  if (NOT TARGET Kinect2::Core)
    add_library(Kinect2::Core MODULE IMPORTED)
    set_target_properties(Kinect2::Core PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${KINECT2_INCLUDE_DIR}"
      IMPORTED_LOCATION "${KINECT2_LIBRARY_DIR}/Kinect20.lib")

  endif()

endif()

mark_as_advanced(KINECT2_INCLUDE_DIR KINECT2_LIBRARY_DIR)
