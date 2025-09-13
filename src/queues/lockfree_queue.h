#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>
#include <atomic>
//write index, read index
//circular buffer
// put:
//  increase write
//  write data
// get:
//  increase read
//  read data
template <typename T>
class lockfree_queue {
 private:
  std::size_t size{};
  std::size_t read_idx{};
  std::size_t write_idx{};
  struct Node {
    T val;
    bool empty = true;
  };

  std::vector<Node> _data;

  size_t calc_space(size_t read, size_t write) {
    if (read == write)
      return size;
    if (read > write) {
      return read - 1 - write;
    }
    return size - write - 1;
  }

 public:
  lockfree_queue(size_t size = 100000) : size{size}, _data(size, Node{}) {}

  bool try_put(const T& value) {
    Node desired{value, false};
    for (size_t i{0}; i < size; i++) {
      std::size_t curr_write_idx{(write_idx + i) % size};
      std::atomic_ref<Node> ref{_data[curr_write_idx]};
      auto expected{ref.load(std::memory_order::relaxed)};

      if (expected.empty) {
        if (ref.compare_exchange_weak(expected, desired,std::memory_order_release, std::memory_order_relaxed)) {
          write_idx = curr_write_idx + 1;
          return true;
        }
      }
    }
    // std::cout << "queue is full or unlucky \n";
    return false;
  }
  std::optional<T> try_get() {
    Node node{};
    for (size_t i{0}; i < size; i++) {
      std::size_t curr_read_idx{(read_idx + i) % size};
      std::atomic_ref<Node> ref{_data[curr_read_idx]};
      auto expected{ref.load(std::memory_order::relaxed)};

      if (!expected.empty) {
        // std::cout << std::format("attempt to read at idx {}\n", curr_read_idx);
        if (ref.compare_exchange_weak(expected, node,std::memory_order_release, std::memory_order_relaxed)) {
          read_idx = curr_read_idx + 1;
          return {expected.val};
        }
      }
    }

    return std::nullopt;
  }
};
