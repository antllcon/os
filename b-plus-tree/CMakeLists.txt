set(MODULES MemoryMap Menu BTree)

foreach (MODULE ${MODULES})
    add_subdirectory(src/${MODULE})
endforeach ()

add_executable(start main.cpp)
target_link_libraries(start PRIVATE ${MODULES})
set_target_properties(start PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/res")

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()

#if (BUILD_BENCHMARKS)
#    add_subdirectory(benchmarks)
#endif ()