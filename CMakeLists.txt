cmake_minimum_required(VERSION 3.29)
project(parallel_hash_map)

set(CMAKE_CXX_STANDARD 20)
find_package(benchmark REQUIRED)

add_executable(parallel_hash_map benchmark.cpp
        sequential_hashmap.cpp
        sequential_hashmap.h
      )
target_link_libraries(parallel_hash_map benchmark::benchmark)
