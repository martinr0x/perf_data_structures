#include <atomic>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <shared_mutex>
#include <vector>
// we dont need circular index, its enough to have continous counter
template <typename T>
class locking_queue_with_shared_mutex {
 private:
  std::size_t _size{};
  std::atomic<std::size_t> read_idx;
  std::size_t write_idx;

  std::vector<T> _data;
  std::shared_mutex mutex;

 public:
  locking_queue_with_shared_mutex(size_t size = 100000)
      : _data(size, T{}),
        _size{size},
        write_idx{size_t{0}},
        read_idx{size_t{0}} {}

  bool try_put(const T& value) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    auto local_read_idx{read_idx.load(std::memory_order::relaxed)};
    if (write_idx - local_read_idx >= _size)
      return false;
    _data[write_idx% _size] = value;
    write_idx++;
    return true;
  }
  std::optional<T> try_get() {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto local_read_idx{read_idx.load(std::memory_order::relaxed)};
    T val;
    do {
      if (local_read_idx >= write_idx) {
        return std::nullopt;
      }

      val = _data[local_read_idx%_size];

    } while (read_idx.compare_exchange_weak(local_read_idx, local_read_idx +1, 
                                            std::memory_order_release,
                                            std::memory_order_relaxed));
    return {val};
  }
};
