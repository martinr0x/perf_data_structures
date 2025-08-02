#include <cstddef>
#include <mutex>
#include <queue>

template <typename T>
class locking_queue {
 private:
  std::queue<T> _data;
  std::mutex mutex;

 public:
  locking_queue([[maybe_unused]] size_t size) {}

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