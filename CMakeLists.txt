cmake_minimum_required(VERSION 3.29)
project(parallel_hash_map)

set(CMAKE_CXX_STANDARD 20)

add_executable(parallel_hash_map main.cpp
        sequential_hashmap.cpp
        sequential_hashmap.h)
