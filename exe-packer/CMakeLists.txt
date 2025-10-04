find_package(ZLIB REQUIRED)
add_executable(start main.cpp)
target_link_libraries(start ZLIB::ZLIB)