set(MODULES
        ScopeTimer
        ThreadPool
        MemoryManager
)

add_executable(memory-manager main.cpp)

foreach (MODULE ${MODULES})
    add_subdirectory(src/${MODULE})
endforeach ()

target_link_libraries(memory-manager PRIVATE ${MODULES})

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()

if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()