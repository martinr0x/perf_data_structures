#include <cstddef>
#include <mutex>
#include <queue>

template <typename T>
class locking_queue {
 private:
  std::size_t size{};
  std::size_t read_idx{};
  std::size_t write_idx{};

  std::queue<T> _data;
  std::mutex mutex;

 public:
  locking_queue(size_t size = 100000) {}

  bool try_put(const T& value) {
    std::unique_lock<std::mutex> lock(mutex);

    _data.push(value);

    return true;
  }
  std::optional<T> try_get() {
    std::unique_lock<std::mutex> lock(mutex);
    if (!_data.empty()) {
      auto val = _data.back();
      _data.pop();
      return {val};
    }

    return std::nullopt;
  }
};