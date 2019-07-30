add_executable (example202 "Example202.cpp")

target_link_libraries(example202 cvgl Kinect2::Core)

copy_3rdparty_dlls(example202)
