cmake_minimum_required(VERSION 3.28.3)
project(os)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build the tests" OFF)
option(BUILD_BENCHMARKS "Build the benchmarks" OFF)

include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip)
FetchContent_Declare(benchmark GIT_REPOSITORY https://github.com/google/benchmark.git GIT_TAG v1.9.4)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest benchmark)
enable_testing()

add_subdirectory(b-plus-tree)