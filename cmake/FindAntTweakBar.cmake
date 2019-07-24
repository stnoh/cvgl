# [TEMPORARY CMAKE FILE FOR AntTweakBar]

find_path (ANTTWEAKBAR_INCLUDE_DIR
    NAMES
        anttweakbar.h
    PATHS
        "${ANTTWEAKBAR_ROOT}/include"
    DOC
        "The directory where AntTweakBar.h resides"
)

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
    find_library (ANTTWEAKBAR_LIBRARIES
        NAMES
            anttweakbar64
        PATHS
            "${ANTTWEAKBAR_ROOT}/lib/x64"
        DOC
            "The AntTweakBar library"
    )
else()
	message(WARNING "We do not support this environment yet.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ANTTWEAKBAR DEFAULT_MSG ANTTWEAKBAR_LIBRARIES ANTTWEAKBAR_INCLUDE_DIR)


if(ANTTWEAKBAR_FOUND)
  set(ANTTWEAKBAR_INCLUDE_DIR ${ANTTWEAKBAR_INCLUDE_DIR})

  if(NOT ANTTWEAKBAR_LIBRARIES)
    set(ANTTWEAKBAR_LIBRARIES ${ANTTWEAKBAR_LIBRARIES})
  endif()

  if (NOT TARGET AntTweakBar::Lib)
    add_library(AntTweakBar::Lib MODULE IMPORTED)
	set_target_properties(AntTweakBar::Lib PROPERTIES
	  INTERFACE_INCLUDE_DIRECTORIES "${ANTTWEAKBAR_INCLUDE_DIR}"
	  IMPORTED_LOCATION "${ANTTWEAKBAR_LIBRARIES}")

  endif()
endif()

#mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARIES)
