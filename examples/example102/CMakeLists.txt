include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable (example102 "AppGL.cpp")

target_link_libraries(example102 cvgl)

copy_3rdparty_dlls(example102)
