# [TEMPORARY CMAKE FILE FOR RSSDK]

# RSSDK_FOUND
# RSSDK_INCLUDE_DIR
# RSSDK_LIBRARIES

find_path (RSSDK_INCLUDE_DIR
    NAMES
        pxcsensemanager.h
    PATHS
        "${RSSDK_ROOT}/include/pxcsensemanager.h"
    DOC
        "The directory where pxcsensemanager.h resides"
)

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
  find_library (RSSDK_LIBRARIES
    NAMES
	  libpxcmd
    PATHS
      "${RSSDK_ROOT}/lib/x64"
    DOC
	  "The directory where libpxcmd.lib resides"
  )
else()
  message(WARNING "RealSense SDK is only supported in Windows.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RSSDK DEFAULT_MSG RSSDK_LIBRARIES RSSDK_INCLUDE_DIR)

if(RSSDK_FOUND)
  set(RSSDK_INCLUDE_DIR ${RSSDK_INCLUDE_DIR})
  set(RSSDK_LIBRARIES   ${RSSDK_LIBRARIES})

  if (NOT TARGET RSSDK::Core)
    add_library(RSSDK::Core MODULE IMPORTED)
    set_target_properties(RSSDK::Core PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${RSSDK_INCLUDE_DIR}")
    set_property(TARGET RSSDK::Core APPEND PROPERTY IMPORTED_LOCATION "${RSSDK_LIBRARIES}")

  endif()

endif()

mark_as_advanced(RSSDK_INCLUDE_DIR RSSDK_LIBRARIES)
