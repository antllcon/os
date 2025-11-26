add_library(BTree BTree.cpp PageManager.cpp)
target_include_directories(BTree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})