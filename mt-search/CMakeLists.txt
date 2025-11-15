set(MODULES
        FileSystem
        Application
        CommandParser
        InvertedIndex
        ScopeTimer
        SearchService
        TextParser
        ThreadPool
)

add_executable(mt-search main.cpp)

foreach (MODULE ${MODULES})
    add_subdirectory(src/${MODULE})
endforeach ()

# find_package(Boost REQUIRED COMPONENTS thread system)
target_link_libraries(mt-search PRIVATE
        ${MODULES}
        ##  Boost::thread
        ##  Boost::system
)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()

if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()