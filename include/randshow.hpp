#pragma once

#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

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
template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
class RNG {
   private:
    virtual T Advance() const = 0;

   public:
    using result_type = T;
    constexpr static T min() { return std::numeric_limits<T>::min(); }
    constexpr static T max() { return std::numeric_limits<T>::max(); }

    // Random number from [::min, ::max) range.
    virtual T Next() const { return Advance(); }
    // Random number from [::min, ::max) range.
    virtual T operator()() const { return Next(); }

    // Random number from uniform integer distribution in [0, n) range.
    virtual T Next(T n) const { return Next(0, n); }
    // Random number from uniform integer distribution in [0, n) range.
    virtual T operator()(T n) const { return Next(n); }

    // Random number from uniform integer distribution in [a, b) range.
    virtual T Next(T a, T b) const {
        if (a > b - 1) return a;

        std::uniform_int_distribution<> dist(a, b - 1);
        return dist(*this);
    }
    // Random number from uniform integer distribution in [a, b) range.
    virtual T operator()(T a, T b) const { return Next(a, b); }

    // Floating value number from standard uniform distribution i.e. [0, 1]
    // range.
    virtual double NextReal() const { return NextReal(0.0, 1.0); }

    // Floating value from uniform real distribution in [a, b] range.
    virtual double NextReal(double a, double b) const {
        std::uniform_real_distribution<double> dist(a, b);
        return dist(*this);
    }

    // Balanced coin flip with 50% chance of heads and tails.
    virtual bool Heads() const { return NextReal() < 0.5; }

    // Weighted coin flip. Parameter weight must be in (0, 1) range.
    // Weight smaller or equal than 0 will always yield false and
    // bigger or equal to 1 always yielding true.
    virtual bool Heads(double weight) const { return NextReal() < weight; }

   protected:
    std::random_device rd{};
};

// Classic Fisher-Yates O(n) shuffle algorithm implementation.
//
// Link: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
template <class T, class Iterator>
void Shuffle(const RNG<T>& rng, Iterator begin, Iterator end) {
    if (begin >= end) return;

    const size_t length = end - begin;
    for (size_t i = 0; i != length - 2; i++) {
        const auto j = rng.Next(i, length);
        std::swap(*(begin + i), *(begin + j));
    }
}

// Reservoir Sampling O(k(1 + log(n/k))) algorithm implementation, using the 'L'
// variant.
//
// Link: https://en.wikipedia.org/wiki/Reservoir_sampling
template <class T, class Iterator>
std::vector<Iterator> Sample(const RNG<T>& rng, const Iterator begin,
                             const Iterator end, size_t k) {
    const size_t n = end - begin;
    std::vector<Iterator> reservoir{std::min(n, k)};
    std::iota(reservoir.begin(), reservoir.end(), begin);

    if (n <= k) {
        std::vector<Iterator> reservoir{n};
        Shuffle(rng, reservoir.begin(), reservoir.end());
        return reservoir;
    }

    double w = std::exp(std::log(rng.NextReal()) / k);
    size_t i = k;
loop:
    i += std::floor(std::log(rng.NextReal()) / std::log(1 - w)) + 1;
    if (i <= n) {
        reservoir[rng.Next(k)] = begin + i;
        w *= std::exp(std::log(rng.NextReal()) / k);
        goto loop;
    }
    return reservoir;
}

template <class T, class Iterator>
std::vector<Iterator> SampleWithReplacement(const RNG<T>& rng,
                                            const Iterator begin,
                                            const Iterator end, size_t k) {
    const size_t n = end > begin ? end - begin : 0;
    std::vector<Iterator> pool{k, begin};
    for (Iterator& it : pool) {
        it += rng.Next(n);
    }
    return pool;
}

// LCG or Linear Congruential Generator has very low footprint and is very fast.
// It has a limit of generating pseudorandom numbers. LCGs are suitable
// for games or other trivial use-cases, but shouldn't be used for work
// requiring true random numbers.
//
// This LCG implementation requires 32 bytes of memory per instance.
class LCG : public RNG<uint32_t> {
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

    // Getter for state value.
    uint64_t GetSeed() const { return state_; }

   private:
    result_type Advance() const override {
        state_ = (mul_ * state_ + inc_) % mod_;
        return state_;
    }

    mutable uint64_t state_ = rd();
    const uint64_t mul_ = 6458928179451363983ULL;
    const uint64_t inc_ = 0ULL;
    const uint64_t mod_ = ((1ULL << 63ULL) - 25ULL);
};

// XSH-RR PCG
class PCG32 : public RNG<uint32_t> {
   public:
    PCG32() { Next(); }

    explicit PCG32(uint64_t seed) : state_(seed) { Next(); }

    PCG32(uint64_t multiplier, uint64_t increment)
        : mul_(multiplier), inc_(increment) {
        Next();
    }

    PCG32(uint64_t seed, uint64_t multiplier, uint64_t increment)
        : state_(seed), mul_(multiplier), inc_(increment) {
        Next();
    }

    // Getter for state value.
    uint64_t GetSeed() const { return state_; }

   private:
    result_type Advance() const override {
        auto x = state_;
        state_ = mul_ * state_ + inc_;

        uint32_t count = x >> 59U;

        x ^= (x >> 18);                      // XSH
        x = detail::Rotr32(x >> 27, count);  // RR
        return x;
    }

    mutable uint64_t state_ = rd();
    const uint64_t mul_ = 6364136223846793005ULL;
    const uint64_t inc_ = 1442695040888963407ULL;
};

// XSH-RR-RR PCG
class PCG64 : public RNG<uint64_t> {
   public:
    PCG64() { Advance(); }

    explicit PCG64(uint64_t seed) : state_(seed) { Advance(); }

    PCG64(uint64_t multiplier, uint64_t increment)
        : mul_(multiplier), inc_(increment) {
        Advance();
    }

    PCG64(uint64_t seed, uint64_t multiplier, uint64_t increment)
        : state_(seed), mul_(multiplier), inc_(increment) {
        Advance();
    }

    // Getter for state value.
    uint64_t GetSeed() const { return state_; }

   private:
    result_type Advance() const override {
        auto x = state_;
        state_ = mul_ * state_ + inc_;

        uint32_t count = x >> 59U;

        // XSH
        uint32_t high_bits = x >> 32U;
        uint32_t low_bits = x;
        x = low_bits ^ high_bits;

        uint32_t x_low = detail::Rotr32(x, count);             // RR
        uint64_t x_high = detail::Rotr32(high_bits, x & 31U);  // RR
        x = (x_high << 32U) | x_low;

        return x;
    }

    mutable uint64_t state_ = rd();
    const uint64_t mul_ = 6364136223846793005ULL;
    const uint64_t inc_ = 1442695040888963407ULL;
};

// Very fast and "good enough" for many random number needs. Used for
// initialization the state of Xoshiro generators.
//
// Link: https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
class SplitMix64 : public RNG<uint64_t> {
   public:
    SplitMix64() = default;

    explicit SplitMix64(uint64_t seed) : state_(seed) {}

   private:
    result_type Advance() const override {
        uint64_t result = (state_ += 0x9E3779B97f4A7C15);
        result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
        result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
        return result ^ (result >> 31);
    }

    mutable uint64_t state_ = rd();
};

// Xoshiro256++ RNG implementation
//
// Link: https://prng.di.unimi.it/xoshiro256plusplus.c
class Xoshiro256 : public RNG<uint64_t> {
   public:
    Xoshiro256() : Xoshiro256(SplitMix64{}) {}

    Xoshiro256(uint64_t seed) : Xoshiro256(SplitMix64{seed}) {}

   private:
    Xoshiro256(const RNG<uint64_t>& rng) {
        uint64_t t = rng.Next();
        s_[0] = t;
        s_[1] = t >> 32;

        t = rng.Next();
        s_[2] = t;
        s_[3] = t >> 32;
    }

    result_type Advance() const override {
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

    mutable uint64_t s_[4] = {0};
};
}  // namespace randshow
