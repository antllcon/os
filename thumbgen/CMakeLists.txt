find_package(Boost REQUIRED COMPONENTS thread system)

set(MODULES
        ArgParser
        DirectoryScanner
        ImageProcessor
)

add_executable(thumbgen main.cpp)

foreach (MODULE ${MODULES})
    add_subdirectory(src/${MODULE})
endforeach ()

target_link_libraries(thumbgen PRIVATE
        ${MODULES}
        Boost::thread
        Boost::system
)