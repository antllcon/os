add_library(MemoryMap MemoryMappedFile.cpp)
target_include_directories(MemoryMap PUBLIC .. BTree)