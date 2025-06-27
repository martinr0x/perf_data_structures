#include <cstddef>
#include <mutex>
#include <vector>

template <typename T>
class locking_queue_with_circular_buffer {
 private:
  std::size_t _size{};
  std::size_t read_idx{};
  std::size_t write_idx{};

  std::vector<T> _data;
  std::mutex mutex;

 public:
  locking_queue_with_circular_buffer(size_t size = 100000) : _data(size, T{}), _size{size} {}

  bool try_put(const T& value) {
    std::unique_lock<std::mutex> lock(mutex);
    _data[write_idx] = value;
    write_idx = (write_idx + 1) % _size;
    return true;
  }
  std::optional<T> try_get() {
    std::unique_lock<std::mutex> lock(mutex);
    if (read_idx != write_idx) {
      auto val = _data[read_idx];

      read_idx = (read_idx + 1) % _size;
      return {val};
    }

    return std::nullopt;
  }
};