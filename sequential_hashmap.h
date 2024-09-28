//
// Created by martin on 26/09/24.
//

#ifndef SEQUENTIALHASHMAP_H
#define SEQUENTIALHASHMAP_H
#include <math.h>
#include <functional>
#include <iostream>
#include <memory>
#include <span>
namespace hashmap {

namespace details {
[[nodiscard]] inline size_t compute_capacity(const double load_factor,
                                             const size_t max_size) {
  return static_cast<size_t>(
      std::lround(load_factor * static_cast<double>(max_size)));
}

[[nodiscard]] inline bool should_shrink(const size_t capacity,
                                        const size_t size) {
  return capacity > 0 && size / 2 < capacity;
}
[[nodiscard]] inline bool should_grow(const size_t capacity,
                                      const size_t size) {
  return size > capacity && capacity < std::numeric_limits<size_t>::max() / 2;
}
template <typename KeyType, typename ValueType,
          typename HashFunc = std::hash<KeyType>>
class sequential_hashmap {

  using key_type = KeyType;
  using value_type = ValueType;
  using hash_func = HashFunc;

  struct Node {
    key_type key;
    value_type value;
    bool empty = true;
  };

  size_t size_;
  size_t max_size_;
  size_t capacity_;
  double load_factor_;
  std::unique_ptr<Node[]> table_;

  [[nodiscard]] size_t compute_hash_index(const key_type& key) const {
    return hash_func{}(key) & (max_size_ - 1);
  }

  struct Iterator {
    //todo: expose pair and not struct
    using iterator_category = std::forward_iterator_tag;
    using value_type = Node;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    explicit Iterator(std::span<Node> entry) : index_{0}, entry_{entry} {
      if (!entry.empty()) {
        return;
      }
      while (entry_.size() > index_ && entry_[index_].empty)
        ++index_;
      if (index_ >= entry_.size()) {
        entry_ = std::span<Node>{};
      }
    }

    Iterator& operator++() {
      index_++;

      while (entry_.size() > index_ && entry_[index_].empty)
        ++index_;
      if (index_ >= entry_.size()) {
        entry_ = std::span<Node>{};
      }

      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++*this;
      return tmp;
    }
    bool operator!=(Iterator& other) { return !(*this == other); }

    bool operator==(Iterator& other) {
      return entry_.data() == other.entry_.data();
    }

    reference operator*() { return entry_[index_]; }
    pointer operator->() { return &entry_[index_]; }

   private:
    size_t index_;
    std::span<Node> entry_;
  };

 public:
  explicit sequential_hashmap(const size_t start_size = 16,
                              const double load_factor = 0.7)
      : size_{},
        max_size_{start_size},
        load_factor_{load_factor},
        table_(std::make_unique<Node[]>(max_size_)),
        capacity_{compute_capacity(load_factor, start_size)} {}

  sequential_hashmap(sequential_hashmap&& other) noexcept
      : size_{other.size_},
        max_size_{other.max_size_},
        capacity_{other.capacity_},
        load_factor_{other.load_factor_},
        table_{std::move(other.table_)} {
    size_ = other.size_;
    max_size_ = other.max_size_;
    capacity_ = other.capacity_;
    load_factor_ = other.load_factor_;
    table_ = std::move(other.table_);
  }

  sequential_hashmap& operator=(sequential_hashmap&& other) noexcept {
    size_ = other.size_;
    max_size_ = other.max_size_;
    capacity_ = other.capacity_;
    load_factor_ = other.load_factor_;
    table_ = std::move(other.table_);
    other.size_ = 0;
    other.max_size_ = 0;
    other.capacity_ = 0;
    other.load_factor_ = 0.0;
    return *this;
  }

  sequential_hashmap(const sequential_hashmap& other)
      : size_{other.size_},
        max_size_{other.max_size_},
        capacity_{other.capacity_},
        load_factor_{other.load_factor_},
        table_{std::make_unique<Node[]>(capacity_)} {
    std::copy(other.table_.begin(), other.table_.end(), table_.get());
  }
  sequential_hashmap& operator=(const sequential_hashmap& other) {
    size_ = other.size_;
    max_size_ = other.max_size_;
    capacity_ = other.capacity_;
    load_factor_ = other.load_factor_;
    table_ = std::make_unique<Node[]>(capacity_);
    std::copy(other.table_.begin(), other.table_.end(), table_.get());
    return *this;
  }

  ~sequential_hashmap() = default;

  void rehash(const size_t new_max_size) {

    //todo: iterate less hacky.
    auto it_begin{begin()};
    auto it_end{end()};
    auto old_table = std::move(table_);

    max_size_ = new_max_size;
    table_ = std::make_unique<Node[]>(max_size_);
    capacity_ = compute_capacity(load_factor_, max_size_);
    size_ = 0;

    for (auto it{it_begin}; it != it_end; ++it) {
      auto& [key, value, empty] = *it;
      insert(key, value);
    }
  }
  //todo: variadic emplace
  void emplace(const KeyType& key, const value_type& value) {
    insert(key, value);
  }
  void insert(const KeyType& key, const value_type& value) {
    if (should_grow(capacity_, size_)) {
      rehash(2 * max_size_);
    }
    if (size_ >= capacity_) {
      throw std::out_of_range("sequential_hashmap: capacity exceeded");
    }

    size_t index{compute_hash_index(key)};
    while (!table_[index].empty && table_[index].key != key) {
      index = (index + 1) % max_size_;
    }

    if (table_[index].empty) {
      ++size_;
    }
    table_[index].key = key;
    table_[index].value = value;
    table_[index].empty = false;
  }

  Iterator begin() {
    return Iterator(std::span<Node>{table_.get(), max_size_});
  }
  Iterator end() { return Iterator(std::span<Node>{}); }

  Iterator find(const key_type& key) {
    size_t index{compute_hash_index(key)};
    while (!table_[index].empty && table_[index].key != key) {
      index = index + 1 & (max_size_ - 1);
    }

    return !table_[index].empty && table_[index].key == key
               ? Iterator({table_.get() + index, max_size_ - index})
               : end();
  }

  bool contains(const key_type& key) { return find(key) != end(); }

  value_type& at(const key_type& key) {
    Iterator kv{find(key)};
    if (kv == end()) {
      throw std::out_of_range("Key not found");
    }
    return kv->value;
  }

  void erase(const key_type& key) {
    auto kv{find(key)};
    if (kv == end()) {
      return;
    }
    *kv = Node{};
    size_--;

    if (should_shrink(capacity_, size_)) {
      rehash(max_size_ / 2);
    }
  };

  void clear() { table_ = std::make_unique<Node[]>(max_size_); };

  value_type& operator[](const key_type& key) {
    Iterator it{find(key)};
    if (it == end()) {
      insert(key, value_type{});
      return find(key);
    }
    return *it;
  };
  [[nodiscard]] size_t size() const { return size_; }
};
}  // namespace details

template <typename Key, typename Value>
using sequential = details::sequential_hashmap<Key, Value>;
}  // namespace hashmap

#endif  // SEQUENTIALHASHMAP_H
