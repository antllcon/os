add_subdirectory(SysInfo)
add_executable(start main.cpp)
target_link_libraries(start PRIVATE sys_info_lib)