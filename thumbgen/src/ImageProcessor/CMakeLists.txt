add_library(ImageProcessor ImageProcessor.cpp Image.cpp stb_implementatiton.cpp)
target_include_directories(ImageProcessor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})