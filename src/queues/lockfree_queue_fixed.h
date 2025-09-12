#include <atomic>
#include <cstddef>
#include <sys/resource.h>
#include <memory>
#include <vector>

// slot has an index and sequence num
// atomic acquire_idx atomic read_idx

// writer:
// write_idx - read_idx >= size_: queue full
// increment acquire_idx
// when successful, write data to slot, set slot idx to writer idx
// reader:
// read_idx < write_idx: no data
// read_idx > slot_idx: data not ready yet.
// fetch data, try increase idx, if succesful, return.
template <typename T> class lockfree_queue_fixed {
  struct slot {
    std::atomic<std::size_t> sequence_idx{size_t{0}};
    T data = {};
    slot()= default;
  };

private:
  std::size_t _size{};
  std::atomic<std::size_t> read_idx;
  std::atomic<std::size_t> write_idx;

  std::unique_ptr<slot[]> _data;

public:
  lockfree_queue_fixed(size_t size = 100000)
      :  _size{size}, write_idx{size_t{0}},
        read_idx{size_t{0}} {
      _data = std::make_unique<slot[]>(size);
  }

  bool try_put(const T &value) {

    auto local_write_idx{write_idx.load(std::memory_order::relaxed)};
    do {
      const auto local_read_idx{read_idx.load(std::memory_order::acquire)};
      if (local_write_idx - local_read_idx >= _size)
        return false;

    } while (!write_idx.compare_exchange_weak(
        local_write_idx, local_write_idx + 1, std::memory_order_release,
        std::memory_order_relaxed));

    auto &slot{_data[local_write_idx % _size]};
    slot.data = value;
    slot.sequence_idx.store(local_write_idx + 1, std::memory_order_release);
    return true;
  }
  std::optional<T> try_get() {
    auto local_read_idx{read_idx.load(std::memory_order::relaxed)};
    T val;
    do {
      auto &slot{_data[local_read_idx % _size]};
      const auto local_sequence_idx =
          slot.sequence_idx.load(std::memory_order_acquire);
      if ((local_read_idx + 1) != local_sequence_idx) {
        return std::nullopt;
      }

      val = slot.data;

    } while (!read_idx.compare_exchange_weak(local_read_idx, local_read_idx + 1,
                                             std::memory_order_release,
                                             std::memory_order_relaxed));
    return {val};
  }
};
