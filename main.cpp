#include <iostream>

#include "sequential_hashmap.h"

int main() {
    sequential_hashmap<int, int> map;

    for (int i = 0; i < 100; i++) {
        map.insert(i, i);
    }

    for (auto &[key,value, empty]: map) {
        std::cout << key << " " << value << "\n";
    }
    return 0;
}
