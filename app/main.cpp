#include <cstdlib>
#include <iostream>
#include <unordered_map>

#include "include/randshow.hpp"

namespace lcg = randshow::lcg;

int main() {
    const auto engine = lcg::LCG();

    auto map = std::unordered_map<int, int>();
    for (size_t i = 0; i < 10'000'000; i++) {
        map[engine.next64(-100, 100)] += 1;
    }

    for (const auto [k, v] : map) {
        std::cout << k << " " << v << "\n";
    }
}
