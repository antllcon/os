add_executable(tests Tree.test.cpp)
target_link_libraries(tests PRIVATE BTree GTest::gmock_main BTree MemoryMap)

include(GoogleTest)
gtest_discover_tests(tests)