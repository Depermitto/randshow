#include <catch2/catch.hpp>
#include "../include/randshow.hpp"

TEST_CASE("Sanity check") {
    REQUIRE(0 == 0);
}

TEST_CASE("Lcg engine randomization quality") {
    auto engine = randshow::lcg::LCG();
    REQUIRE(engine.coin_flip() <= 1);
}
