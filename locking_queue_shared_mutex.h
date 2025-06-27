#include <atomic>
#include <cstddef>
#include <shared_mutex>
#include <vector>

template <typename T>
class locking_queue_with_shared_mutex {
 private:
  std::size_t _size{};
  std::atomic<std::size_t> read_idx{};
  std::size_t write_idx{};

  std::vector<T> _data;
  std::shared_mutex mutex;

 public:
  locking_queue_with_shared_mutex(size_t size = 100000)
      : _data(size, T{}), _size{size} {}

  bool try_put(const T& value) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    _data[write_idx] = value;
    write_idx = (write_idx + 1) % _size;
    return true;
  }
  std::optional<T> try_get() {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto local_read_idx{read_idx.load(std::memory_order::relaxed)};
    T val;
    do {
      if (local_read_idx == write_idx)
        return std::nullopt;

      val = _data[local_read_idx];

    } while (read_idx.compare_exchange_weak(local_read_idx,
                                            (local_read_idx + 1) % _size));
    return {val};
  }
};