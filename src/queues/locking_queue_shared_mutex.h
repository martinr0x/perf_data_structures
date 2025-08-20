#include <atomic>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <shared_mutex>
#include <vector>
// we dont need circular index, its enough to have continous counter
template <typename T>
class locking_queue_with_shared_mutex {
  struct circular_index {
    size_t size{};
    size_t idx{};
    circular_index operator++(int inc) {
      auto old = circular_index{size, idx};
      idx += 1;
      return old;
    }
    bool operator==(const circular_index& rhs) const { return idx == rhs.idx; }
    bool operator>=(const circular_index& rhs) const { return idx >= rhs.idx; }
    int operator-(const circular_index& rhs) const { return idx - rhs.idx; }
    size_t operator*() const { return idx % size; }
  };

 private:
  std::size_t _size{};
  std::atomic<circular_index> read_idx;
  circular_index write_idx;

  std::vector<T> _data;
  std::shared_mutex mutex;

 public:
  locking_queue_with_shared_mutex(size_t size = 100000)
      : _data(size, T{}),
        _size{size},
        write_idx{size, size_t{0}},
        read_idx{{size, size_t{0}}} {}

  bool try_put(const T& value) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    if (write_idx - read_idx >= _size)
      return false;
    _data[*write_idx] = value;
    write_idx++;
    return true;
  }
  std::optional<T> try_get() {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto local_read_idx{read_idx.load(std::memory_order::relaxed)};
    circular_index old_read{};
    T val;
    do {
      if (local_read_idx >= write_idx) {
        return std::nullopt;
      }

      val = _data[*local_read_idx];

      old_read = local_read_idx;
      old_read++;
    } while (read_idx.compare_exchange_weak(local_read_idx, old_read,
                                            std::memory_order_release,
                                            std::memory_order_relaxed));
    return {val};
  }
};