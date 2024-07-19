#include <catch2/catch.hpp>
#include <randshow/engines.hpp>

using randshow::DefaultEngine;

TEST_CASE("Sanity Check") { REQUIRE(1 == 1); }

TEST_CASE("RNG Methods") {
    constexpr size_t N = 1e6;

    SECTION("randshow::Next()") {
        for (size_t i = 0; i < N; i++) {
            auto t = DefaultEngine.Next();
            REQUIRE((DefaultEngine.min() <= t && t <= DefaultEngine.max()));
        }
    }

    SECTION("randshow::operator()") {
        for (size_t i = 0; i < N; i++) {
            auto t = DefaultEngine();
            REQUIRE((DefaultEngine.min() <= t && t <= DefaultEngine.max()));
        }
    }

    SECTION("randshow::Next(result_type)") {
        for (size_t i = 0; i < N; i++) {
            auto t = DefaultEngine.Next(10);
            REQUIRE(t < 10);

            REQUIRE(DefaultEngine.Next(0) == 0);
        }
    }

    SECTION("randshow::operator()(result_type)") {
        for (size_t i = 0; i < N; i++) {
            auto t = DefaultEngine(10);
            REQUIRE(t < 10);

            REQUIRE(DefaultEngine(0) == 0);
        }
    }

    SECTION("randshow::Next(result_type, result_type)") {
        for (size_t i = 0; i < N; i++) {
            auto t = DefaultEngine.Next(0, 10);
            REQUIRE((0 <= t && t < 10));

            t = DefaultEngine.Next(-10, 10);
            REQUIRE((-10 <= t && t < 10));

            REQUIRE(DefaultEngine.Next(5, 5) == 5);
            REQUIRE(DefaultEngine.Next(5, 3) == 5);
        }
    }

    SECTION("randshow::operator()(result_type, result_type)") {
        for (size_t i = 0; i < N; i++) {
            auto t = DefaultEngine(0, 10);
            REQUIRE((0 <= t && t < 10));

            t = DefaultEngine(-10, 10);
            REQUIRE((-10 <= t && t < 10));

            REQUIRE(DefaultEngine(5, 5) == 5);
            REQUIRE(DefaultEngine(5, 3) == 5);
        }
    }

    SECTION("randshow::NextReal") {
        for (size_t i = 0; i < N; i++) {
            auto t = DefaultEngine.NextReal();
            REQUIRE((0.0 < t && t < 1.0));
        }
    }

    SECTION("randshow::NextReal(double, double)") {
        for (size_t i = 0; i < N; i++) {
            auto t = DefaultEngine.NextReal(0.0, 1e-10);
            REQUIRE((0.0 < t && t <= 1e-10));

            t = DefaultEngine.NextReal(-5.0, 3.0);
            REQUIRE((-5.0 < t && t <= 3.0));
        }
    }
}
