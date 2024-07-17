#include <iostream>

#include "include/randshow.hpp"

int main() {
    randshow::PCG32 rng{};
    // PractRand raw random data in binary format
    while (1) {
        auto value = rng();
        std::cout.write(reinterpret_cast<char*>(&value), sizeof(value));
    }
}
