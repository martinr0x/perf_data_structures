#include <atomic>
#include <cstddef>
#include <ostream>
#include <shared_mutex>
#include <vector>
// circular_index class,
// size_idx;
// wrapped;
// if read_idx != write_idx

template <typename T>
class locking_queue_with_shared_mutex {
  struct circular_index {
    bool wrapped = false;
    size_t size{};
    size_t idx{};
    circular_index& operator++(int inc) {
      idx++;
      if (idx == size) {
        wrapped = !wrapped;
        idx = 0;
      };
      return *this;
    }
    bool operator==(const circular_index& rhs) const {
      return wrapped == rhs.wrapped && idx == rhs.idx;
    }
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
        write_idx{false, size, 0},
        read_idx({false, size, 0}) {
          std::cout << sizeof(circular_index) << std::endl;
        }

  bool try_put(const T& value) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    _data[write_idx.idx] = value;
    write_idx++;
    return true;
  }
  std::optional<T> try_get() {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto local_read_idx{read_idx.load(std::memory_order::relaxed)};
    T val;
    do {
      if (local_read_idx == write_idx)
        return std::nullopt;

      val = _data[local_read_idx.idx];
      //todo: can we solve the explicit copy more elegent?
    } while (read_idx.compare_exchange_weak(
        local_read_idx, circular_index { local_read_idx } ++,
        std::memory_order_release, std::memory_order_relaxed));
    return {val};
  }
};