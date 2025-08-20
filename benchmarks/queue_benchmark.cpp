#include <benchmark/benchmark.h>
#include <optional>
#include <thread>

#include "queues/concurrentqueue.h"
#include "queues/locking_queue.h"
#include "queues/locking_queue_circular_buffer.h"
#include "queues/locking_queue_shared_mutex.h"

#include "queues/lockfree_queue.h"


static void bm_queue_queue_spsc(benchmark::State& state) {
  lockfree_queue<int> q;
  const int N = state.range(0);

  for (auto _ : state) {
    std::atomic<bool> done = false;

    // Start producer inside timing
    std::thread producer([&]() {
      for (int i = 0; i < N; ++i) {
        while (!q.try_put(i)) {
          std::this_thread::yield();
        }
      }
      done = true;
    });

    int consumed = 0;
    while (consumed < N) {
      auto opt = q.try_get();
      if (opt) {
        ++consumed;
      } else {
        std::this_thread::yield();
      }
    }

    producer.join();
  }
}
template <typename QUEUE>
static void bm_queue_mpmc(benchmark::State& state) {
  const int N = state.range(0);              // Number of items per producer
  const int num_producers = state.range(1);  // Number of producer threads
  const int num_consumers = state.range(2);  // Number of consumer threads

  for (auto _ : state) {
    QUEUE q(N);
    std::atomic<int> produced_count{0};
    std::atomic<int> consumed_count{0};

    // Producer threads
    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p) {
      producers.emplace_back([&]() {
        for (int i = 0; i < N; ++i) {
          while (!q.try_put(i)) {
            std::this_thread::yield();
          }
          produced_count.fetch_add(1, std::memory_order_relaxed);
        }
      });
    }

    // Consumer threads
    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
      consumers.emplace_back([&]() {
        while (true) {
          auto opt = q.try_get();
          if (opt) {
            int count =
                consumed_count.fetch_add(1, std::memory_order_relaxed) + 1;
            if (count >= N * num_producers) {
              break;
            }
          } else {
            if (consumed_count.load(std::memory_order_relaxed) >=
                N * num_producers) {
              break;
            }
            std::this_thread::yield();
          }
        }
      });
    }

    for (auto& t : producers)
      t.join();
    for (auto& t : consumers)
      t.join();
  }
}
template <typename T>
struct moodycamel_wrapper {
  moodycamel::ConcurrentQueue<T> q;

  moodycamel_wrapper<T>(std::size_t size) : q(size) {}
  bool try_put(const T& val) { return q.try_enqueue(val); }
  std::optional<T> try_get() {
    T val;

    if (q.try_dequeue(val)) {
      return {val};
    }

    return std::nullopt;
  }
};

// Register benchmarks
// Args: N, num_producers, num_consumers
BENCHMARK(bm_queue_mpmc<lockfree_queue<int>>)
    ->ArgsProduct({
        {100000},             // N
        {1},                  // producers
        {1, 2, 4, 8, 16, 24}  // consumers
    });
BENCHMARK(bm_queue_mpmc<moodycamel_wrapper<int>>)
    ->ArgsProduct({
        {100000},             // N
        {1},                  // producers
        {1, 2, 4, 8, 16, 24}  // consumers
    });
BENCHMARK(bm_queue_mpmc<locking_queue<int>>)
    ->ArgsProduct({
        {100000},             // N
        {1},                  // producers
        {1, 2, 4, 8, 16, 24}  // consumers
    });
BENCHMARK(bm_queue_mpmc<locking_queue_with_circular_buffer<int>>)
    ->ArgsProduct({
        {100000},             // N
        {1},                  // producers
        {1, 2, 4, 8, 16, 24}  // consumers
    });
BENCHMARK(bm_queue_mpmc<locking_queue_with_shared_mutex<int>>)
    ->ArgsProduct({
        {100000},             // N
        {1},                  // producers
        {1, 2, 4, 8, 16, 24}  // consumers
    });
BENCHMARK(bm_queue_queue_spsc)->Arg(1000000);

