#include "../include/randshow.hpp"

#include <catch2/catch.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <unordered_map>

TEST_CASE("Sanity check") { REQUIRE(0 == 0); }

bool test_rng(const randshow::RNG<uint32_t>& rng, size_t number_range,
              int numbers_per_bucket, double error_threshold) {
    auto map = std::unordered_map<int, int>();
    for (size_t i = 0; i < number_range * numbers_per_bucket; i++) {
        map[rng.Next(number_range)] += 1;
    }

    const auto acceptable_error = numbers_per_bucket * error_threshold;
    for (const auto [k, v] : map) {
        std::cout << k << " " << v << " "
                  << numbers_per_bucket - acceptable_error << " "
                  << numbers_per_bucket + acceptable_error << "\n";

        if (v < numbers_per_bucket - acceptable_error) return false;
        if (v > numbers_per_bucket + acceptable_error) return false;
    }
    return true;
}

TEST_CASE("Randomization quality") {
    constexpr auto number_range = 2000;
    constexpr auto precision = 1200;
    constexpr auto error_threshold = 0.15;

    CHECK(test_rng(randshow::LCG(), number_range, precision, error_threshold));
    CHECK(test_rng(randshow::PCG(), number_range, precision, error_threshold));
}
