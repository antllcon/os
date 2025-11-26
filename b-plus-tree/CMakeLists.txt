set(MODULES MemoryMap Menu BTree)

add_executable(b-plus-tree main.cpp)

foreach (MODULE ${MODULES})
    add_subdirectory(src/${MODULE})
endforeach ()

target_link_libraries(b-plus-tree PRIVATE ${MODULES})

#if (BUILD_TESTING)
#    add_subdirectory(tests)
#endif ()
#
#if (BUILD_BENCHMARKS)
#    add_subdirectory(benchmarks)
#endif ()