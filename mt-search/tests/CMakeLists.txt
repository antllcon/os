add_executable(tests ThreadPool.test.cpp)
target_link_libraries(tests PRIVATE ThreadPool GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(tests)