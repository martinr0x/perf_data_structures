# perf_data_structures
This is a **pet project** to learn and experiment with concurrent data structures.  
Not production-ready — performance and correctness are the main focus.

Includes comparisons against proven solutions (e.g., [moodycamel/concurrentqueue](https://github.com/cameron314/concurrentqueue)).

---

## Implemented Queues

- **locking_queue**  
  Simple queue with a unique lock for reads/writes.

- **locking_queue_shared_mutex**  
  RW lock + atomic read counter. Allows concurrent reads but suffers from writer starvation.

- **lockfree_queue**  
  Lock-free queue using scanning + CAS. Limited to small data types.

- **lockfree_queue_fixed**  
  Lock-free queue with atomic read/write counters + slot sequencing. (Naming TBD.)

- **moodycamel**  
  Reference implementation; fast lock-free queue.

---

## Benchmarks

Environment: **Apple M4 Pro (12 cores, ARM)**  

### Single Producer, Multiple Consumers (SPMC)
![SPMC Results](https://github.com/martinr0x/perf_data_structures/blob/master/benchmarks/spmc_results.png?raw=true)

- `locking_queue` appears very fast due to non-fixed size (fits in L1 cache).  
- `locking_queue_shared_mutex` is slow: writer starves because consumers rarely release shared locks.

---

### Multiple Producers, Single Consumer (MPSC)
![MPSC Results](https://github.com/martinr0x/perf_data_structures/blob/master/benchmarks/mpsc_results.png?raw=true)

- `moodycamel` & `locking_queue` outperform others because they do not have a size limit. Therefore they can continue to put elements and do not have to wait for the reader to free elements. 

---

## Future Work

- Lock-free queue with dynamic growth.  
- Sharded design for `lockfree_queue_fixed` to reduce contention.  

---
