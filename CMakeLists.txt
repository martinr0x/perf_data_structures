cmake_minimum_required(VERSION 3.29)
project(perf_data_structures)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
find_package(benchmark REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_subdirectory(src)
add_subdirectory(benchmarks)
add_subdirectory(tests)
