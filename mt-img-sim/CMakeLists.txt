add_executable(MtImgSim main.cpp)

add_subdirectory(src/ImageProcessor)
add_subdirectory(src/ArgParser)

target_link_libraries(MtImgSim PRIVATE
        ImageProcessor
        ArgParser
)

target_include_directories(MtImgSim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})