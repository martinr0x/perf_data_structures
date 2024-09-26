#include <iostream>

#include "SequentialHashmap.h"

int main() {
    SequentialHashmap<int, int> map;
    for (int i = 0; i < 100; i++) {
        map.insert(i, i);
    }

    for (auto &[key,value, empty]: map) {
        std::cout << key << " " << value << "\n";
    }
    return 0;
}
