#pragma once

#include <cmath>
#include <cstdint>
#include <random>
#include <type_traits>
#include <vector>

namespace randshow {
// Fully satisfies UniformRandomBitGenerator
template <class T = uint32_t, T MIN = 0, T MAX = UINT32_MAX,
          std::enable_if_t<std::is_integral_v<T>, bool> = true>
class RNG {
   public:
    // UniformRandomBitGenerator C++11 named requirement implementation
    constexpr static T min() { return MIN; }
    constexpr static T max() { return MAX; }
    using result_type = T;

    virtual T Next() const { return Advance(); }
    virtual T operator()() const { return Next(); }

    // Next random number from uniform distribution in [0, n) range.
    virtual T Next(T n) const { return Next(0, n); }
    virtual T operator()(T n) const { return Next(n); }

    // Next random number from uniform distribution in [a, b) range.
    virtual T Next(int64_t a, int64_t b) const {
        if (a > b - 1) return a;

        std::uniform_int_distribution<int64_t> dist(a, b - 1);
        return dist(*this);
    }
    virtual T operator()(int64_t a, int64_t b) const { return Next(a, b); }

    // Next floating point number from standard uniform distribution i.e. (0, 1)
    // range.
    virtual double NextFloating() const {
        return static_cast<double>(Next()) / max();
    }

    // Floating point uniform distribution in [a, b) range.
    virtual double NextFloating(double a, double b) const {
        return NextFloating() * (b - a) + a;
    }

    // Balanced coin flip with 50% chance of heads and tails.
    virtual bool Heads() const { return NextFloating() < 0.5; }

    // Weighted coin flip. Parameter weight must be in (0, 1) range.
    // Weight smaller or equal than 0 will always yield false and
    // bigger or equal to 1 always yielding true.
    virtual bool Heads(double weight) const { return NextFloating() < weight; }

   private:
    virtual uint32_t Advance() const = 0;

   protected:
    std::random_device rd{};
};

// Classic Fisher-Yates O(n) shuffle algorithm implementation.
//
// Link: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
template <class T, T MIN, T MAX, class Iterator>
void Shuffle(const RNG<T, MIN, MAX>& rng, Iterator begin, Iterator end) {
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
template <class T, T MIN, T MAX, class Iterator>
std::vector<Iterator> Sample(const RNG<T, MIN, MAX>& rng, const Iterator begin,
                             const Iterator end, size_t k) {
    const size_t n = end - begin;
    std::vector<Iterator> reservoir{std::min(n, k)};
    std::iota(reservoir.begin(), reservoir.end(), begin);

    if (n <= k) {
        std::vector<Iterator> reservoir{n};
        Shuffle(rng, reservoir.begin(), reservoir.end());
        return reservoir;
    }

    double w = std::exp(std::log(rng.NextFloating()) / k);
    size_t i = k;
loop:
    i += std::floor(std::log(rng.NextFloating()) / std::log(1 - w)) + 1;
    if (i <= n) {
        reservoir[rng.Next(k)] = begin + i;
        w *= std::exp(std::log(rng.NextFloating()) / k);
        goto loop;
    }
    return reservoir;
}

template <class T, T MIN, T MAX, class Iterator>
std::vector<Iterator> SampleWithReplacement(const RNG<T, MIN, MAX>& rng,
                                            const Iterator begin,
                                            const Iterator end, size_t k) {
    const size_t n = end > begin ? end - begin : 0;
    std::vector<Iterator> pool(k, begin);
    for (Iterator& it : pool) {
        it += rng.Next(n);
    }
    return pool;
}

// LCG or Linear Congruential Generator has very low footprint and is very fast.
// It has m_modulo limit of generating pseudorandom numbers. LCGs are suitable
// for games or other trivial use-cases, but shouldn't be used for work
// requiring true random numbers.
//
// This LCG implementation requires 32 bytes of memory per instance.
class LCG : public RNG<> {
   private:
    uint32_t Advance() const override {
        const auto x = seed_;
        seed_ = (mul_ * seed_ + inc_) % mod_;
        return x;
    }

   public:
    // Creates a new LCG engine with a, c, m parameters equal to the default
    // engine. Seed is current time.
    LCG() = default;

    // Creates a new LCG engine with a, c, m parameters equal to the default
    // engine with custom seed value. Meant for reproducibility.
    explicit LCG(uint64_t seed) : seed_(seed) {}

    // Creates a new LCG engine with custom a, c, m parameters and seed equal to
    // current time.
    LCG(uint64_t multiplier, uint64_t increment, uint64_t modulo)
        : mul_(multiplier), inc_(increment), mod_(modulo) {}

    // Creates a new LCG engine.
    LCG(uint64_t seed, uint64_t multiplier, uint64_t increment, uint64_t modulo)
        : seed_(seed), mul_(multiplier), inc_(increment), mod_(modulo) {}

    // Getter for state value.
    uint64_t GetSeed() const { return seed_; }

    // Getter for state multiplier.
    uint64_t GetMul() const { return mul_; }

    // Getter for multiplied state increment.
    uint64_t GetInc() const { return inc_; }

    // Getter for incremented state modulo.
    uint64_t GetMod() const { return mod_; }

   private:
    mutable uint64_t seed_ = rd();
    uint64_t mul_ = 6458928179451363983ULL;
    uint64_t inc_ = 0ULL;
    uint64_t mod_ = ((1ULL << 63ULL) - 25ULL);
};

class PCG : public RNG<> {
   private:
    static uint32_t Rotr32(uint32_t x, uint32_t r) {
        return (x >> r) | (x << (-r & 31));
    }

    // XSH-RR
    uint32_t Advance() const override {
        auto x = seed_;
        seed_ = mul_ * seed_ + inc_;

        uint32_t count = x >> 59U;

        x ^= (x >> 18);              // XSH
        x = Rotr32(x >> 27, count);  // RR
        return x;
    }

   public:
    // XSH-RR-RR
    uint64_t Next64() const {
        auto x = seed_;
        seed_ = mul_ * seed_ + inc_;

        uint32_t count = x >> 59U;

        // XSH
        uint32_t high_bits = x >> 32U;
        uint32_t low_bits = x;
        x = low_bits ^ high_bits;

        uint32_t x_low = Rotr32(x, count);             // RR
        uint64_t x_high = Rotr32(high_bits, x & 31U);  // RR
        x = (x_high << 32U) | x_low;

        return x;
    }

    PCG() { Next(); }

    explicit PCG(uint64_t seed) : seed_(seed) { Next(); }

    PCG(uint64_t multiplier, uint64_t increment)
        : mul_(multiplier), inc_(increment) {
        Next();
    }

    PCG(uint64_t seed, uint64_t multiplier, uint64_t increment)
        : seed_(seed), mul_(multiplier), inc_(increment) {
        Next();
    }

    // Getter for state value.
    uint64_t GetSeed() const { return seed_; }

    // Getter for state multiplier.
    uint64_t GetMul() const { return mul_; }

    // Getter for multiplied state increment.
    uint64_t GetInc() const { return inc_; }

   private:
    mutable uint64_t seed_ = rd();
    const uint64_t mul_ = 6364136223846793005ULL;
    const uint64_t inc_ = 1442695040888963407ULL;
};

class C_RNG : public RNG<> {
   private:
    uint32_t Advance() const override { return std::rand(); }

   public:
    C_RNG() { std::srand(rd()); }

    explicit C_RNG(uint64_t seed) { std::srand(seed); }
};
}  // namespace randshow
