add_executable(perf_data_structures 
    hash_map_benchmark.cpp
    queue_benchmark.cpp
)
target_compile_options(perf_data_structures PRIVATE "-O2")
target_link_libraries(perf_data_structures benchmark::benchmark data_structures)