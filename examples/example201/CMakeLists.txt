add_executable (example201 "Example201.cpp")

target_link_libraries(example201 cvgl RSSDK::Core)

copy_3rdparty_dlls(example201)
