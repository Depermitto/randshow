#include <sys/types.h>

#include <cstddef>
#include <iostream>
#include <random>
#include <unordered_map>

#include "include/randshow.hpp"

const randshow::Xoshiro256PlusPlus rng{};

int main() {
    auto counter = std::unordered_map<int, int>();
    std::poisson_distribution<> dist(10);

    for (size_t i = 0; i < 1000; i++) {
        counter[dist(rng)] += 1;
    }

    for (size_t i = 0; i < counter.size(); i++) {
        std::string count(counter[i], '*');
        std::cout << i << ": " << count << "\n";
    }
}
