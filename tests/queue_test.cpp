#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <barrier> 
#include <unordered_set>
#include "queues/locking_queue_circular_buffer.h"
#include "queues/locking_queue_shared_mutex.h"
#include "queues/lockfree_queue.h"
#include "queues/lockfree_queue_fixed.h"
template <typename T>
class QueueTest : public ::testing::Test {
 protected:
  std::unique_ptr<T> queue;
  void SetUp() override { queue = std::make_unique<T>(2000); }
};

using QueueTypes =
    ::testing::Types<lockfree_queue<int>, locking_queue_with_shared_mutex<int>,
                     locking_queue_with_circular_buffer<int>, lockfree_queue_fixed<int>>;

TYPED_TEST_SUITE(QueueTest, QueueTypes);

TYPED_TEST(QueueTest, two_producer_one_consumer) {
  const int N = 1000;
  auto& q = *this->queue;
  std::vector<int> data1, data2;
  for (int i = 0; i < N; ++i) {
    data1.push_back(i);
    data2.push_back(i + N);
  }

  // Insert from two threads
  std::thread t1([&]() {
    for (int x : data1) {
      while (!q.try_put(x)) {
        // Optionally yield or sleep to avoid tight spin
        std::this_thread::yield();
      }
    }
  });

  std::thread t2([&]() {
    for (int x : data2) {
      while (!q.try_put(x)) {
        std::this_thread::yield();
      }
    }
  });

  t1.join();
  t2.join();
  std::size_t tries = 10;
  // Remove from one thread
  std::unordered_set<int> removed;
  while (removed.size() <= 2 * N && tries > 0) {
    auto val{q.try_get()};
    if (val.has_value()) {
      removed.insert(val.value());
    } else {
      tries--;
    }
  }
  // Verify all expected values were removed
  for (int x : data1) {
    ASSERT_TRUE(removed.count(x) == 1);
  }
  for (int x : data2) {
    ASSERT_TRUE(removed.count(x) == 1);
  }
}
TYPED_TEST(QueueTest, two_producers_two_consumers) {
  const int N = 1000;
  auto& q = *this->queue;
  std::vector<int> data1, data2;
  for (int i = 0; i < N; ++i) {
    data1.push_back(i);
    data2.push_back(i + N);
  }
  std::atomic<bool> writers_done = false;

  std::thread writer1([&]() {
    for (int x : data1) {
      while (!q.try_put(x)) {
        std::this_thread::yield();
      }
    }
  });

  std::thread writer2([&]() {
    for (int x : data2) {
      while (!q.try_put(x)) {
        std::this_thread::yield();
      }
    }
  });

  std::unordered_set<int> removed;
  std::mutex removed_mutex;

  // Readers
  auto reader = [&]() {
    while (true) {
      auto opt = q.try_get();
      if (opt) {
        std::lock_guard<std::mutex> lock(removed_mutex);
        removed.insert(*opt);
      } else {
        if (writers_done) {
          // No more writers, no more data
          break;
        }
        std::this_thread::yield();
      }
    }
  };

  std::thread reader1(reader);
  std::thread reader2(reader);

  writer1.join();
  writer2.join();
  writers_done = true;

  reader1.join();
  reader2.join();
  ASSERT_TRUE(removed.size() == 2 * N);
  for (int x : data1) {
    ASSERT_TRUE(removed.count(x) == 1);
  }
  for (int x : data2) {
    ASSERT_TRUE(removed.count(x) == 1);
  }
}
TYPED_TEST(QueueTest, stress_randomized_with_timeout_and_spinlimit) {
  const int producers = 4;
  const int consumers = 4;
  const int N = 5000; // items per producer
  auto& q = *this->queue;

  std::atomic<int> produced_count{0};
  std::atomic<int> consumed_count{0};
  std::barrier start_barrier(producers + consumers);

  std::mutex consumed_mutex;
  std::unordered_set<int> consumed_values;

  // Global watchdog
  std::atomic<bool> done = false;
  std::thread watchdog([&] {
    using namespace std::chrono_literals;
    auto deadline = std::chrono::steady_clock::now() + 10s;
    while (!done && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(100ms);
    }
    if (!done) {
      ADD_FAILURE() << "Test timed out — possible deadlock in queue!";
      std::terminate();
    }
  });

  // Producers
  auto producer = [&](int id) {
    start_barrier.arrive_and_wait();

    for (int i = 0; i < N; i++) {
      int value = id * N + i;
      int spins = 0;
      while (!q.try_put(value)) {
        if (++spins > 1'000'000) {
          FAIL() << "Producer " << id << " spin limit exceeded at value " << value;
        }
        std::this_thread::yield();
      }
      produced_count++;
    }
  };

  // Consumers
  auto consumer = [&](int id) {
    start_barrier.arrive_and_wait();

    while (consumed_count < producers * N) {
      auto opt = q.try_get();
      if (opt) {
        {
          std::lock_guard<std::mutex> lock(consumed_mutex);
          auto [_, inserted] = consumed_values.insert(*opt);
          ASSERT_TRUE(inserted) << "Duplicate value detected: " << *opt;
        }
        consumed_count++;
      } else {
        static thread_local int spins = 0;
        if (++spins > 1'000'000) {
          FAIL() << "Consumer " << id << " spin limit exceeded";
        }
        std::this_thread::yield();
      }
    }
  };

  // Launch all threads
  std::vector<std::thread> threads;
  for (int i = 0; i < producers; i++) threads.emplace_back(producer, i);
  for (int i = 0; i < consumers; i++) threads.emplace_back(consumer, i);
  for (auto& t : threads) t.join();

  done = true;
  watchdog.join();

  // Final verification
  EXPECT_EQ(consumed_count, producers * N);
  EXPECT_EQ(consumed_values.size(), producers * N);
  for (int i = 0; i < producers * N; i++) {
    ASSERT_TRUE(consumed_values.count(i) == 1);
  }
}
