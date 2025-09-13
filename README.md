# perf_data_structures
Pet project to explore (concurrent) data structures and implement stuff on my own.
Occasionally running some benchmarks against some proven solutions to see how well the stuff works


## Queues
Implemented 4 different concurrent queues:
* locking_queue: Unique lock for reading and writing
* locking_queue_with_shared_mutex: RW lock + atomic read counter
* lockfree_queue: Using scanning approach + CAS. Works only for small data types
* lockfree_queue_fixed: Poor naming, uses atomic read/write counter + slot sequencing
* moodycamel: Fast lockfree queue just used for comparison

### Benchmarks
Ran on my 12-core Apple M4 PRO (ARM)

Single Producer Multiple Consumer
![alt text](https://github.com/martinr0x/perf_data_structures/blob/master/benchmarks/spmc_results.png?raw=true)
* Locking queue is so fast because its not fixed size, so inserts usually fit in L1 cache.

Multiple Producer Single Consumer
![alt text](https://github.com/martinr0x/perf_data_structures/blob/master/benchmarks/mpsc_results.png?raw=true)
* Moodycamel is so much faster because it can dynamically grow, and does not wait until consumer took values from the queue.
* Same for locking_queue

### Future work
* Dynamically growing lookfree queue
* Reduce contention on lockfree_queue_fixed with sharding scheme




