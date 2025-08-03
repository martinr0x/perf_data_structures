#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <unordered_set>
#include "queues/locking_queue_circular_buffer.h"
#include "queues/locking_queue_shared_mutex.h"
#include "queues/parallel_queue.h"
template <typename T>
class QueueTest : public ::testing::Test {
 protected:
  std::unique_ptr<T> queue;
  void SetUp() override { queue = std::make_unique<T>(2000); }
};

using QueueTypes =
    ::testing::Types<parallel_queue<int>, locking_queue_with_shared_mutex<int>,
                     locking_queue_with_circular_buffer<int>>;

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
    EXPECT_TRUE(removed.count(x) == 1);
  }
  for (int x : data2) {
    EXPECT_TRUE(removed.count(x) == 1);
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
  EXPECT_TRUE(removed.size() == 2 * N);
  for (int x : data1) {
    EXPECT_TRUE(removed.count(x) == 1);
  }
  for (int x : data2) {
    EXPECT_TRUE(removed.count(x) == 1);
  }
}
