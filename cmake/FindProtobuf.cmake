# [TEMPORARY CMAKE FILE FOR Kinect2]

# PROTOBUF_FOUND
# PROTOBUF_INCLUDE_DIR
# PROTOBUF_LIBRARY_DIR

find_path (PROTOBUF_INCLUDE_DIR
    NAMES
        google/protobuf/stubs/common.h
    PATHS
        "${PROTOBUF_ROOT}/include"
    DOC
        "The directory where common.h resides"
)

if (WIN32 AND MSVC14 OR (${MSVC_VERSION} EQUAL 1900))
    find_path (PROTOBUF_LIBRARY_DIR
        NAMES
			libprotobuf.lib
			libprotobuf-lite.lib
			libprotoc.lib
        PATHS
			"${PROTOBUF_ROOT}/Lib/x64"
        DOC
			"The directory where libproto*.lib resides"
    )
else()
	message(WARNING "We only provide prebuilt binary in Windows.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(protobuf DEFAULT_MSG PROTOBUF_LIBRARY_DIR PROTOBUF_INCLUDE_DIR)

if(PROTOBUF_FOUND)
  set(PROTOBUF_INCLUDE_DIR ${PROTOBUF_INCLUDE_DIR})
  set(PROTOBUF_LIBRARY_DIR ${PROTOBUF_LIBRARY_DIR})

  if (NOT TARGET protobuf::libprotobuf)
    add_library(protobuf::libprotobuf MODULE IMPORTED)
    set_target_properties(protobuf::libprotobuf PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${PROTOBUF_INCLUDE_DIR}"
      IMPORTED_LOCATION "${PROTOBUF_LIBRARY_DIR}/libprotobuf.lib")

  endif()

endif()

mark_as_advanced(PROTOBUF_INCLUDE_DIR PROTOBUF_LIBRARY_DIR)
