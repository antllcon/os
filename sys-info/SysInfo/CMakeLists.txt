add_library(sys_info_lib STATIC "")

if (WIN32)
    target_sources(sys_info_lib PRIVATE SysInfoWin.cpp)
    target_link_libraries(sys_info_lib PRIVATE ws2_32.lib psapi.lib user32.lib advapi32.lib)
elseif(UNIX)
    target_sources(sys_info_lib PRIVATE SysInfoLinux.cpp)
    target_link_libraries(sys_info_lib PRIVATE util)
endif()

target_include_directories(sys_info_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})