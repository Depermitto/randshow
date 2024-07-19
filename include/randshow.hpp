#pragma once

#include <sys/types.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <limits>
#include <random>

namespace randshow {
namespace detail {
constexpr inline uint32_t Rotr32(uint32_t x, int r) {
    return (x >> r) | (x << (32 - r));
}
constexpr inline uint32_t Rotl32(uint32_t x, int r) {
    return (x << r) | (x >> (32 - r));
}
constexpr inline uint64_t Rotr64(uint64_t x, int r) {
    return (x >> r) | (x << (64 - r));
}
constexpr inline uint64_t Rotl64(uint64_t x, int r) {
    return (x << r) | (x >> (64 - r));
}
}  // namespace detail

// Fully satisfies UniformRandomBitGenerator
template <class T, typename std::enable_if<std::is_integral<T>::value,
                                           bool>::type = true>
class RNG {
   private:
    virtual T Advance() = 0;

   public:
    using result_type = T;
    constexpr static T min() { return std::numeric_limits<T>::min(); }
    constexpr static T max() { return std::numeric_limits<T>::max(); }

    // Random number from [::min, ::max) range.
    T Next() { return Advance(); }
    // Random number from [::min, ::max) range.
    T operator()() { return Next(); }

    // Random number from uniform integer distribution in [0, n) range.
    T Next(T n) { return Next(static_cast<T>(0), n); }
    // Random number from uniform integer distribution in [0, n) range.
    T operator()(T n) { return Next(n); }

    // Random number from uniform integer distribution in [a, b) range.
    template <class U, typename std::enable_if<std::is_integral<T>::value,
                                               bool>::type = true>
    U Next(U a, U b) {
        if (a >= b) return a;

        std::uniform_int_distribution<> dist(a, std::nextafter(b, a));
        return dist(*this);
    }
    // Random number from uniform integer distribution in [a, b) range.
    template <class U, typename std::enable_if<std::is_integral<T>::value,
                                               bool>::type = true>
    U operator()(U a, U b) {
        return Next(a, b);
    }

    // Floating value number from standard uniform distribution i.e. (0, 1)
    // range.
    double NextReal() { return NextReal(std::nextafter(0.0, 1.0), 1.0); }

    // Floating value from uniform real distribution in [a, b) range. For (0, 1)
    // non-inclusive range refer to NextReal().
    double NextReal(double a, double b) {
        if (a >= b) return a;

        std::uniform_real_distribution<double> dist(a, std::nextafter(b, a));
        return dist(*this);
    }

    // Balanced coin flip with 50% chance of heads and tails.
    bool Heads() { return NextReal() < 0.5; }

    // Weighted coin flip. Parameter weight must be in (0, 1) range.
    // Weight smaller or equal 0 will always yield false and
    // bigger or equal to 1 always yield true.
    bool Heads(double weight) { return NextReal() < weight; }

    // Classic Fisher-Yates O(n) shuffle algorithm implementation.
    //
    // Link: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
    template <class Iterator>
    void Shuffle(Iterator begin, Iterator end) noexcept {
        const size_t length = std::distance(begin, end);
        for (size_t i = 0; i < length - 2; i++) {
            std::swap(*(begin + i), *(begin + Next(i, length)));
        }
    }

    // Reservoir Sampling O(k(1 + log(n/k))) algorithm implementation, using the
    // 'L' variant.
    //
    // Note: This function does not perform bound checks. Improper iterator
    // range is considered undefined behaviour.
    //
    // Link: https://en.wikipedia.org/wiki/Reservoir_sampling
    template <class Iterator, class OutIterator>
    void Sample(const Iterator begin, const Iterator end, OutIterator out,
                size_t k) noexcept {
        std::copy(begin, begin + k, out);
        const size_t length = std::distance(begin, end);
        if (k >= length / 2) {
            Shuffle(out, out + k);
        }

        double w = std::exp(std::log(NextReal()) / k);
        size_t i = k - 1;
    loop:
        i += std::floor(std::log(NextReal()) / std::log(1 - w)) + 1;
        if (i < length) {
            *(out + Next(k)) = *(begin + i);
            w *= std::exp(std::log(NextReal()) / k);
            goto loop;
        }
    }

    // Time complexity: O(k)
    template <class Iterator, class OutIterator>
    void SampleWithReplacement(const Iterator begin, const Iterator end,
                               OutIterator out, size_t k) noexcept {
        const size_t length = std::distance(begin, end);
        std::transform(out, out + k, out, [begin, length, this]() {
            return *(begin + Next(length));
        });
    }

   protected:
    std::random_device rd{};
};

// LCG or Linear Congruential Generator is a small and fast RNG. LCGs are
// suitable for games or other trivial use-cases, but generally it is
// unrecommended to use them when compared to some of the other choices in this
// library. Take a look at PCG32 instead.
//
// This LCG implementation requires 32 bytes of memory per instance.
class LCG : public RNG<uint64_t> {
   public:
    // Creates a new LCG engine with a, c, m parameters equal to the default
    // engine. Seed is current time.
    LCG() = default;

    // Creates a new LCG engine with a, c, m parameters equal to the default
    // engine with custom seed value. Meant for reproducibility.
    explicit LCG(uint64_t seed) : state_(seed) {}

    // Creates a new LCG engine with custom a, c, m parameters and seed equal to
    // current time.
    LCG(uint64_t multiplier, uint64_t increment, uint64_t modulo)
        : mul_(multiplier), inc_(increment), mod_(modulo) {}

    // Creates a new LCG engine.
    LCG(uint64_t seed, uint64_t multiplier, uint64_t increment, uint64_t modulo)
        : state_(seed), mul_(multiplier), inc_(increment), mod_(modulo) {}

    result_type Advance() override {
        state_ = (mul_ * state_ + inc_) % mod_;
        return state_;
    }

    // Getter for state value.
    uint64_t GetSeed() const { return state_; }

   private:
    uint64_t state_ = rd();
    const uint64_t mul_ = 6458928179451363983ULL;
    const uint64_t inc_ = 0ULL;
    const uint64_t mod_ = ((1ULL << 63ULL) - 25ULL);
};

// XSH-RR member of the PCG family. 64-bit state and 32-bit output. Great and
// recommeneded for all purposes.
class PCG32 : public RNG<uint32_t> {
   public:
    PCG32() { PCG32::Advance(); }

    PCG32(uint64_t seed) : state_(seed) { PCG32::Advance(); }

    result_type Advance() override {
        auto x = state_;
        state_ = 6364136223846793005ULL * state_ + 1442695040888963407ULL;
        uint32_t xorshifted = ((x >> 18U) ^ x) >> 27U;  // XSH
        return detail::Rotr32(xorshifted, x >> 59U);    // RR
    }

    // Getter for state value.
    uint64_t GetSeed() const { return state_; }

   private:
    uint64_t state_ = rd();
};

// XSL-RR member of the PCG family. 128-bit state and 64-bit output.
//
// Note: This variant requires a compiler compatible with '__uint128_t' type
// (GCC/CLANG)
class PCG64 : public RNG<uint64_t> {
   public:
    PCG64() { PCG64::Advance(); }

    explicit PCG64(uint64_t seed) : state_(seed) { PCG64::Advance(); }

    result_type Advance() override {
        auto x = state_;
        state_ = MUL * state_ + INC;
        uint64_t count = x >> 122U;
        return detail::Rotr64(x ^ (x >> 64), count);
    }

    // Getter for state value.
    __uint128_t GetSeed() const { return state_; }

   private:
    constexpr static __uint128_t MUL =
        (__uint128_t(2549297995355413924ULL) << 64) + 4865540595714422341ULL;
    constexpr static __uint128_t INC =
        (__uint128_t(6364136223846793005ULL) << 64) + 1442695040888963407ULL;
    __uint128_t state_ = (__uint128_t(rd()) << 64U) + rd();
};

// Very fast and "good enough" for many random number needs. Used for
// initialization the state of Xoshiro generators.
//
// Link: https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
class SplitMix64 : public RNG<uint64_t> {
   public:
    SplitMix64() { SplitMix64::Advance(); };

    explicit SplitMix64(uint64_t seed) : state_(seed) { SplitMix64::Advance(); }

    result_type Advance() override {
        uint64_t result = (state_ += 0x9E3779B97f4A7C15);
        result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
        result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
        return result ^ (result >> 31);
    }

    // Getter for state value.
    uint64_t GetSeed() const { return state_; }

   private:
    uint64_t state_ = rd();
};

// Recommended for all purposes. Great speed and a state space
// large enough for any parallel application, although it is not synchronized in
// its implementation. Any parallel calls should be synchronized from the
// outside.
//
// Link: https://prng.di.unimi.it/xoshiro256plusplus.c
class Xoshiro256PlusPlus : public RNG<uint64_t> {
   public:
    Xoshiro256PlusPlus() : Xoshiro256PlusPlus(SplitMix64{}) {
        Xoshiro256PlusPlus::Advance();
    }

    Xoshiro256PlusPlus(uint64_t seed) : Xoshiro256PlusPlus(SplitMix64{seed}) {
        Xoshiro256PlusPlus::Advance();
    }

    template <class UniformRandomBitGenerator>
    Xoshiro256PlusPlus(UniformRandomBitGenerator&& g) {
        uint64_t t = g();
        s_[0] = t;
        s_[1] = t >> 32;

        t = g();
        s_[2] = t;
        s_[3] = t >> 32;
    }

    result_type Advance() override {
        const uint64_t result = detail::Rotl64(s_[0] + s_[3], 23) + s_[0];
        const uint64_t t = s_[1] << 17;

        s_[2] ^= s_[0];
        s_[3] ^= s_[1];
        s_[1] ^= s_[2];
        s_[0] ^= s_[3];

        s_[2] ^= t;
        s_[3] = detail::Rotl64(s_[3], 45);

        return result;
    }

   private:
    uint64_t s_[4] = {0};
};

static PCG32 DefaultEngine{};
}  // namespace randshow
