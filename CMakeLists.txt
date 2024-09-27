cmake_minimum_required(VERSION 3.29)
project(parallel_hash_map)

set(CMAKE_CXX_STANDARD 20)
find_package(benchmark REQUIRED)

add_executable(parallel_hash_map benchmark.cpp
        sequential_hashmap.cpp
        sequential_hashmap.h
        bytell_hash_map.h
        flat_hash_map.h
        unordered_map.h
)
target_compile_options(parallel_hash_map PRIVATE "-O2")
target_link_libraries(parallel_hash_map benchmark::benchmark)
