add_executable(benchmarks ThreadPool.benchmark.cpp)
target_link_libraries(benchmarks PRIVATE ThreadPool benchmark::benchmark)