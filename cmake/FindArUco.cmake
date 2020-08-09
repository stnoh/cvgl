# [TEMPORARY CMAKE FILE FOR ArUco]

# ARUCO_FOUND
# ARUCO_INCLUDE_DIR
# ARUCO_LIBRARIES
# ARUCO_BINARIES

find_path (ARUCO_INCLUDE_DIR
  NAMES
    aruco/aruco.h
  PATHS
    "${ARUCO_ROOT}/include"
  DOC
    "The directory where aruco.h resides"
)

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
  find_library (ARUCO_LIBRARIES
    NAMES
      aruco306
    PATHS
      "${ARUCO_ROOT}/lib/x64"
    DOC
      "The ArUco library"
  )
else()
  message(WARNING "We do not support this environment yet.")
endif()

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
  find_file (ARUCO_BINARIES
    NAMES
      aruco306.dll
    PATHS
      "${ARUCO_ROOT}/bin64"
    DOC
      "The ArUco prebuilt binary"
  )
else()
  message(WARNING "We do not support this environment yet.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ARUCO DEFAULT_MSG ARUCO_LIBRARIES ARUCO_INCLUDE_DIR)


if(ARUCO_FOUND)
  set(ARUCO_INCLUDE_DIR ${ARUCO_INCLUDE_DIR})

  if(NOT ARUCO_LIBRARIES)
    set(ARUCO_LIBRARIES	${ARUCO_LIBRARIES})
  endif()

  if(NOT ARUCO_BINARIES)
    set(ARUCO_BINARIES ${ARUCO_BINARIES})
  endif()

  if (NOT TARGET ArUco::Lib)
    add_library(ArUco::Lib MODULE IMPORTED)
	set_target_properties(ArUco::Lib PROPERTIES
	  INTERFACE_INCLUDE_DIRECTORIES "${ARUCO_INCLUDE_DIR}"
	  IMPORTED_LOCATION "${ARUCO_LIBRARIES}")
  endif()

endif()

#mark_as_advanced(ARUCO_INCLUDE_DIR ARUCO_LIBRARIES)
