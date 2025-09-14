import matplotlib.pyplot as plt
import re

# Raw benchmark data
data_mpsc = """
bm_queue_mpmc<lockfree_queue<int>>/100000/1/1                         7.37 ms        0.041 ms         1000
bm_queue_mpmc<lockfree_queue<int>>/100000/2/1                         20.4 ms        0.048 ms         1000
bm_queue_mpmc<lockfree_queue<int>>/100000/4/1                         61.4 ms        0.072 ms          100
bm_queue_mpmc<lockfree_queue<int>>/100000/8/1                          273 ms        0.136 ms           10
bm_queue_mpmc<lockfree_queue<int>>/100000/16/1                         649 ms        0.192 ms           10
bm_queue_mpmc<lockfree_queue<int>>/100000/24/1                        1004 ms        0.266 ms           10
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/1/1                   2.05 ms        0.133 ms         5526
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/2/1                   12.5 ms        0.136 ms         1000
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/4/1                   59.7 ms        0.167 ms          100
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/8/1                    205 ms        0.222 ms          100
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/16/1                   663 ms        0.280 ms           10
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/24/1                   971 ms        0.337 ms           10
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/1/1                     7.52 ms        0.060 ms         1000
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/2/1                     11.5 ms        0.068 ms         1000
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/4/1                     37.9 ms        0.135 ms          100
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/8/1                     70.8 ms        0.180 ms          100
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/16/1                     158 ms        0.272 ms          100
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/24/1                     242 ms        0.344 ms          100
bm_queue_mpmc<locking_queue<int>>/100000/1/1                          1.38 ms        0.029 ms        10000
bm_queue_mpmc<locking_queue<int>>/100000/2/1                          5.14 ms        0.037 ms         1000
bm_queue_mpmc<locking_queue<int>>/100000/4/1                          11.7 ms        0.056 ms         1000
bm_queue_mpmc<locking_queue<int>>/100000/8/1                          15.8 ms        0.081 ms         1000
bm_queue_mpmc<locking_queue<int>>/100000/16/1                         31.0 ms        0.154 ms          100
bm_queue_mpmc<locking_queue<int>>/100000/24/1                         45.2 ms        0.214 ms          100
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/1/1        3.56 ms        0.027 ms         1000
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/2/1        17.6 ms        0.040 ms         1000
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/4/1         136 ms        0.060 ms          100
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/8/1        1396 ms        0.108 ms           10
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/16/1       6711 ms        0.193 ms            1
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/24/1      17955 ms        0.290 ms            1
"""
data_spmc = """
bm_queue_mpmc<lockfree_queue<int>>/100000/1/1                         7.67 ms        0.047 ms         1000 
bm_queue_mpmc<lockfree_queue<int>>/100000/1/2                         9.64 ms        0.045 ms         1000 
bm_queue_mpmc<lockfree_queue<int>>/100000/1/4                         17.5 ms        0.064 ms         1000 
bm_queue_mpmc<lockfree_queue<int>>/100000/1/8                         41.3 ms        0.095 ms          100 
bm_queue_mpmc<lockfree_queue<int>>/100000/1/16                        43.0 ms        0.162 ms          100 
bm_queue_mpmc<lockfree_queue<int>>/100000/1/24                        43.1 ms        0.226 ms          100 
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/1/1                   2.13 ms        0.102 ms         6942 
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/1/2                   3.46 ms        0.093 ms         1000 
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/1/4                   15.0 ms        0.145 ms         1000 
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/1/8                   11.6 ms        0.177 ms         1000 
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/1/16                  30.4 ms        0.249 ms          100 
bm_queue_mpmc<lockfree_queue_fixed<int>>/100000/1/24                  30.4 ms        0.336 ms          100 
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/1/1                     7.48 ms        0.061 ms         1000 
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/1/2                     9.22 ms        0.054 ms         1000 
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/1/4                     24.2 ms        0.096 ms         1000 
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/1/8                     24.4 ms        0.132 ms         1000 
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/1/16                    19.6 ms        0.211 ms         1000 
bm_queue_mpmc<moodycamel_wrapper<int>>/100000/1/24                    19.7 ms        0.278 ms         1000 
bm_queue_mpmc<locking_queue<int>>/100000/1/1                          1.39 ms        0.031 ms        10000 
bm_queue_mpmc<locking_queue<int>>/100000/1/2                          3.48 ms        0.033 ms         1000 
bm_queue_mpmc<locking_queue<int>>/100000/1/4                          6.72 ms        0.064 ms         1000 
bm_queue_mpmc<locking_queue<int>>/100000/1/8                          12.6 ms        0.096 ms         1000 
bm_queue_mpmc<locking_queue<int>>/100000/1/16                         21.3 ms        0.186 ms         1000 
bm_queue_mpmc<locking_queue<int>>/100000/1/24                         21.9 ms        0.275 ms         1000 
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/1/1        3.53 ms        0.026 ms         1000 
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/1/2        17.5 ms        0.036 ms         1000 
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/1/4        14.6 ms        0.063 ms         1000 
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/1/8         126 ms        0.106 ms          100 
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/1/16        873 ms        0.200 ms           10 
bm_queue_mpmc<locking_queue_with_shared_mutex<int>>/100000/1/24       1210 ms        0.262 ms           10 
"""

# Parse the data
benchmarks = {}
for line in data.strip().splitlines():
    match = re.match(r'bm_queue_mpmc<(.+?)>/\d+/(\d+)/(\d+)\s+([\d.]+) ms', line)
    if match:
        queue_type = match.group(1)
        threads = max(int(match.group(2)), int(match.group(3)))
        time_ms = float(match.group(4))
        benchmarks.setdefault(queue_type, []).append((threads, time_ms))

# Plot
plt.figure(figsize=(10,6))
for queue_type, results in benchmarks.items():
    results.sort(key=lambda x: x[0])
    threads, times = zip(*results)
    plt.plot(threads, times, marker='o', label=queue_type)

plt.xlabel("Number of Threads")
plt.ylabel("Time (ms)")
plt.title("Multiple Producer Single Consumer")
plt.yscale("log")  # log scale because times vary a lot
plt.grid(True, which="both", ls="--", linewidth=0.5)
plt.legend()
plt.tight_layout()
plt.savefig("queue_benchmark.png", dpi=300)
