cmake_minimum_required(VERSION 3.29)
project(parallel_hash_map)

set(CMAKE_CXX_STANDARD 20)

add_executable(parrallel_hash_map main.cpp
        SequentialHashmap.cpp
        SequentialHashmap.h)
