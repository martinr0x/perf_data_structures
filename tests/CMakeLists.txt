add_executable(tests queue_test.cpp)
target_link_libraries(tests gtest_main data_structures)
add_test(NAME all_tests COMMAND tests)