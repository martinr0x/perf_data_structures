cmake_minimum_required(VERSION 3.29)
project(parallel_hash_map)

set(CMAKE_CXX_STANDARD 20)
find_package(benchmark REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()


add_executable(parallel_hash_map hash_map_benchmark.cpp
        queue_benchmark.cpp
        sequential_hashmap.h
        bytell_hash_map.h
        flat_hash_map.h
)
target_compile_options(parallel_hash_map PRIVATE "-O2")
target_link_libraries(parallel_hash_map benchmark::benchmark)

add_executable(tests queue_test.cpp)
target_link_libraries(tests gtest_main)
add_test(NAME all_tests COMMAND tests)