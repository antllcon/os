add_library(FileDesc STATIC "")

target_sources(FileDesc PRIVATE FileDesc.cpp)
target_link_libraries(FileDesc PRIVATE util)

target_include_directories(FileDesc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})