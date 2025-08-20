target_sources(data_structures INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrentqueue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/locking_queue_circular_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/locking_queue_shared_mutex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/locking_queue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lockfree_queue.h
)