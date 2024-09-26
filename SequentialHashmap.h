//
// Created by martin on 26/09/24.
//

#ifndef SEQUENTIALHASHMAP_H
#define SEQUENTIALHASHMAP_H
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include<math.h>

template<typename KeyType, typename ValueType, typename HashFunc= std::hash<KeyType> >
class SequentialHashmap {
    struct Entry {
        KeyType key;
        ValueType value;
        bool empty = true;
    };

    size_t entries_;
    size_t size_;
    size_t capacity_;

    double load_factor_;


    std::unique_ptr<Entry[]> table_;

    size_t calc_capacity(const double load_factor, const size_t size) {
        return static_cast<size_t>(std::lround(load_factor * static_cast<double>(size)));
    }

    struct Iterator {
        size_t index_;
        Entry *entry_;
        size_t max_size_;

        Iterator(Entry *entry, const size_t index, const size_t max_size): index_{index}, entry_{entry},
                                                                           max_size_{max_size} {
            if (!entry)return;
            while (max_size_ > index_ && (*(entry_ + index_)).empty)++index_;
            if (index_ >= max_size_) {
                entry_ = nullptr;
            }
        }

        Iterator &operator++() {
            index_++;

            while (max_size_ > index_ && (*(entry_ + index_)).empty)++index_;
            if (index_ >= max_size_) {
                entry_ = nullptr;
            }

            return *this;
        }

        bool operator!=(const Iterator &other) {
            return entry_ != other.entry_;
        }

        bool operator==(const Iterator &other) {
            return entry_ == other.entry_;
        }

        Entry &operator*() {
            return *(entry_ + index_);
        }
    };

public:
    explicit SequentialHashmap(const size_t size = 16, const double load_factor = 0.9): entries_{}, size_{size},
        load_factor_{load_factor}, capacity_{calc_capacity(load_factor, size)},
        table_(std::make_unique<Entry[]>(capacity_)) {
    }

    SequentialHashmap(SequentialHashmap &&other) noexcept: entries_{other.entries_}, size_{other.size_},
                                                           capacity_{other.capacity_},
                                                           load_factor_{other.load_factor_},
                                                           table_{std::move(other.table_)} {
        entries_ = other.entries_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        load_factor_ = other.load_factor_;
        table_ = std::move(other.table_);
    }

    SequentialHashmap &operator=(SequentialHashmap &&other) noexcept {
        entries_ = other.entries_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        load_factor_ = other.load_factor_;
        table_ = std::move(other.table_);
        other.entries_ = 0;
        other.size_ = 0;
        other.capacity_ = 0;
        other.load_factor_ = 0.0;
        return *this;
    }

    SequentialHashmap(const SequentialHashmap &other): entries_{other.entries_}, size_{other.size_},
                                                       capacity_{other.capacity_}, load_factor_{other.load_factor_},
                                                       table_{std::make_unique<Entry[]>(capacity_)} {
        std::copy(other.table_.begin(), other.table_.end(), table_.get());
    }

    SequentialHashmap &operator=(const SequentialHashmap &other) {
        entries_ = other.entries_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        load_factor_ = other.load_factor_;
        table_ = std::make_unique<Entry[]>(capacity_);
        std::copy(other.table_.begin(), other.table_.end(), table_.get());
        return *this;
    }


    void insert(const KeyType &key, const ValueType &value) {
        if (entries_ >= capacity_) {
            grow();
        }

        const size_t index = HashFunc{}(key) % size_;
        size_t dynamic_index{index};
        while (!table_[dynamic_index].empty && !table_[dynamic_index].key != key && index - 1 % size_ !=
               dynamic_index) {
            dynamic_index = (dynamic_index + 1) % size_;
        }

        if (table_[dynamic_index].empty) {
            ++entries_;
        }
        table_[dynamic_index].key = key;
        table_[dynamic_index].value = value;
        table_[dynamic_index].empty = false;
    }

    void grow() {
        if (size_ / 2 >= std::numeric_limits<size_t>::max()) {
            throw std::invalid_argument("SequentialHashmap resizing failed");
        }

        const size_t old_size = size_;
        size_ *= 2;
        capacity_ = calc_capacity(load_factor_, size_);
        entries_ = 0;
        auto old_table = std::make_unique<Entry[]>(size_);
        std::swap(old_table, table_);

        for (size_t i = 0; i < old_size; ++i) {
            const auto &[key,value, empty]{old_table[i]};
            if (empty) continue;
            insert(key, value);
        }
    }

    Iterator begin() {
        return Iterator(table_.get(), 0, size_);
    }

    Iterator end() {
        return Iterator(nullptr, entries_, size_);
    }

    Iterator find(const KeyType &key) {
        const size_t old_index = HashFunc{}(key) % size_;
        size_t index{old_index};

        while (!table_[index].empty && table_[index].key != key && old_index - 1 % size_ != index) {
            index = index + 1 % size_;
        }
        return !table_[index].empty && table_[index].key == key
                   ? Iterator(table_.get(), index, size_)
                   : Iterator(nullptr, size_t{}, size_);
    }

    bool contains(const KeyType &key) {
        return find(key) != end();
    }

    ValueType &at(const KeyType &key) {
        Iterator kv{find(key)};
        if (kv == end()) {
            throw std::out_of_range("Key not found");
        }
        return *kv.value;
    }

    void erase(const KeyType &key) {
        auto kv{find(key)};
        if (kv == end()) {
            return;
        }
        (*kv).empty = true;
        entries_--;
    };

    void clear() {
        for (size_t i = 0; i < size_; ++i) {
            table_[i].empty = true;
        }
    };

    ValueType &operator[](const KeyType &key) {
        Iterator it{find(key)};
        return (*it).value;
    };
};


#endif //SEQUENTIALHASHMAP_H
