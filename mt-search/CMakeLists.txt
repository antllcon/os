find_package(Boost REQUIRED COMPONENTS thread system)

set(MODULES
        ScopeTimer
)

add_executable(mt-search main.cpp)

foreach (MODULE ${MODULES})
    add_subdirectory(src/${MODULE})
endforeach ()

target_link_libraries(mt-search PRIVATE
        ${MODULES}
        Boost::thread
        Boost::system
)