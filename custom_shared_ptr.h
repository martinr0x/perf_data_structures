

#include <atomic>
#include <cstddef>
#include <iostream>
#include <utility>
struct control_block {
  std::atomic<int> refs;
  std::atomic<int> shareds;
};

template <typename T>
struct default_deleter {
  void operator()(T* ptr) { delete ptr; }
};

template <typename T, typename Deleter = default_deleter<T>>
struct custom_shared_ptr {
  T* value;
  control_block* control;
  Deleter deleter{};
  custom_shared_ptr(T* value)
      : value(value), control(new control_block), deleter{} {
    control->shareds.fetch_add(1);
  }
  custom_shared_ptr(T* value, Deleter&& deleter)
      : value(value), control(new control_block), deleter{deleter} {
    control->shareds.fetch_add(1);
  }
  custom_shared_ptr(const custom_shared_ptr& other) {
    std::cout << "copy cnstr\n";

    value = other.value;
    control = other.control;
    if (control != nullptr) {
      control->shareds.fetch_add(1);
    }
  }
  custom_shared_ptr(custom_shared_ptr&& other) {
    std::cout << "move cnstr\n";
    std::swap(value, other.value);
    std::swap(control, other.control);
  }
  custom_shared_ptr& operator=(custom_shared_ptr&& other) {
    std::cout << "move asignment\n";

    decrease_ref_and_delete();
    std::exchange(this->control, other.control);
    std::exchange(this->value, other.value);

    other.value = nullptr;
    other.control = nullptr;
    return *this;
  }
  custom_shared_ptr& operator=(const custom_shared_ptr& other) {
    std::cout << "copy asignment\n";

    decrease_ref_and_delete();
    std::exchange(this->control, other.control);
    std::exchange(this->value, other.value);
    control->shareds.fetch_add(1);
    return *this;
  }

  T& operator*() {
    if (control == nullptr) {
      return *value;
    }
  }
  T* operator->() { return this->value; }
  bool operator==(const custom_shared_ptr& rhs) const {
    return this->control == rhs.control;
  }
  int use_count() const {
    return control == nullptr ? 0 : control->refs.load();
  }

  void decrease_ref_and_delete() {
    if (control == nullptr) {
      return;
    }
    control->shareds.fetch_sub(1);
    if (control->shareds.load() <= 0) {
      deleter(value);
      delete control;
      std::cout << "delete" << std::endl;
    }
  }

  ~custom_shared_ptr() { decrease_ref_and_delete(); }
};