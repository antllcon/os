find_package(Boost REQUIRED COMPONENTS thread system)

set(MODULES
        ArgParser
        DirectoryScanner
        ImageProcessor
        ResultFilter
)

add_executable(MtImgSim main.cpp)

foreach (MODULE ${MODULES})
    add_subdirectory(src/${MODULE})
endforeach ()

target_link_libraries(MtImgSim PRIVATE
        ${MODULES}
        Boost::thread
        Boost::system
)