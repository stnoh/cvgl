add_executable (example101 "example101.cpp")

target_link_libraries(example101 cvgl)

copy_3rdparty_dlls(example101)
