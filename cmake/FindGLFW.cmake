# [TEMPORARY CMAKE FILE FOR GLFW]

# GLFW_FOUND
# GLFW_INCLUDE_DIR
# GLFW_LIBRARIES

find_path (GLFW_INCLUDE_DIR
    NAMES
        GLFW/glfw3.h
    PATHS
        "${GLFW_ROOT}/include"
    DOC
        "The directory where GLFW/glfw.h resides"
)

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
    find_library (GLFW_LIBRARIES
        NAMES
            glfw3dll
        PATHS
            "${GLFW_ROOT}/lib/x64"
        DOC
            "The GLFW library"
    )
else()
	message(WARNING "We do not support this environment yet.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW DEFAULT_MSG GLFW_LIBRARIES GLFW_INCLUDE_DIR)


if(GLFW_FOUND)
  set(GLFW_INCLUDE_DIR ${GLFW_INCLUDE_DIR})

  if(NOT GLFW_LIBRARIES)
    set(GLFW_LIBRARIES ${GLFW_LIBRARIES})
  endif()

  if (NOT TARGET GLFW::GLFW)
    add_library(GLFW::GLFW UNKNOWN IMPORTED)
    set_target_properties(GLFW::GLFW PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}")
    set_property(TARGET GLFW::GLFW APPEND PROPERTY IMPORTED_LOCATION "${GLFW_LIBRARIES}")
  endif()
endif()

#mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARIES)
