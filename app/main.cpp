#include <sys/types.h>

#include <iostream>
#include <random>
#include <unordered_map>

#include "include/randshow.hpp"

randshow::PCG32 rng{};

int main() {
    std::unordered_map<int, int> counter{};
    std::poisson_distribution<> dist{10};

    for (int n = 1000; n--;) {
        counter[dist(rng)] += 1;
    }

    for (size_t i = counter.size(); i--;) {
        std::string count(counter[i], '*');
        std::cout << i << ": " << count << "\n";
    }
}
