# Create interface library for headers
add_library(data_structures INTERFACE)
target_include_directories(data_structures INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(hash_maps)
add_subdirectory(queues)