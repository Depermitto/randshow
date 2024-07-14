#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

namespace randshow {
class RNG {
   public:
    // Next random 32-bit unsigned int.
    [[nodiscard]] virtual uint32_t Next32() const { return Advance32(); }

    // Next random 32-bit unsigned int from uniform distribution in [0, n)
    // range.
    [[nodiscard]] virtual uint32_t Next32(uint32_t n) const {
        return Next32() % n;
    }

    // Next random 32-bit signed int from uniform distribution in [a, b) range.
    [[nodiscard]] virtual int32_t Next32(int32_t a, int32_t b) const {
        return b > a ? Next32() % (b - a) + a : a;
    }

    // Next random 64-bit unsigned int.
    [[nodiscard]] virtual uint64_t Next64() const { return Advance64(); }

    // Next random 64-bit unsigned int from uniform distribution in [0, n)
    // range.
    [[nodiscard]] virtual uint64_t Next64(uint64_t n) const {
        return Next64() % n;
    }

    // Next random 64-bit unsigned int from uniform distribution in [a, b)
    // range.
    [[nodiscard]] virtual int64_t Next64(int64_t a, int64_t b) const {
        return b > a ? Next64() % (b - a) + a : a;
    }

    // Next floating point number from standard uniform distribution i.e. (0, 1)
    // range.
    [[nodiscard]] virtual double NextFloating() const {
        return static_cast<double>(Next32()) / UINT32_MAX;
    }

    // Floating point uniform distribution in [a, b) range.
    [[nodiscard]] virtual double NextFloating(double a, double b) const {
        return NextFloating() * (b - a) + a;
    }

    // Balanced coin flip with 50% chance of heads and tails.
    [[nodiscard]] virtual bool Heads() const { return NextFloating() < 0.5; }

    // Weighted coin flip. Parameter weight must be in (0, 1) range.
    // Weight smaller or equal than 0 will always yield false and
    // bigger or equal to 1 always yielding true.
    [[nodiscard]] virtual bool Heads(double weight) const {
        return NextFloating() < weight;
    }

   private:
    [[nodiscard]] virtual uint32_t Advance32() const = 0;
    [[nodiscard]] virtual uint64_t Advance64() const = 0;
};

// Classic Fisher-Yates shuffle algorithm implementation
// Link: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
template <class Iterator>
void Shuffle(const RNG& rng, Iterator begin, Iterator end) {
    if (begin >= end) return;

    const size_t length = end - begin;
    for (size_t i = 0; i != length - 2; i++) {
        const auto j = rng.Next64(i, length);
        std::swap(*(begin + i), *(begin + j));
    }
}

// Reservoir Sampling algorithm implementation, currently Algoritm R variant
// Link: https://en.wikipedia.org/wiki/Reservoir_sampling
template <class Iterator>
std::vector<Iterator> Sample(const RNG& rng, Iterator begin, Iterator end,
                             size_t n) {
    const size_t size = end - begin;

    if (size <= n) {
        std::vector<Iterator> reservoir(size);
        std::iota(reservoir.begin(), reservoir.end(), begin);
        Shuffle(rng, reservoir.begin(), reservoir.end());
        return reservoir;
    }

    std::vector<Iterator> reservoir(n);
    std::iota(reservoir.begin(), reservoir.end(), begin);

    for (size_t i = n + 1; i <= size; i++) {
        const auto j = rng.Next64(i);
        if (j < n) {
            reservoir[j] = begin + i;
        }
    }

    return reservoir;
}

// LCG or Linear Congruential Generator has very low footprint and is very fast.
// It has m_modulo limit of generating pseudorandom numbers. LCGs are suitable
// for games or other trivial use-cases, but shouldn't be used for work
// requiring true random numbers.
//
// This LCG implementation requires 32 bytes of memory per instance.
class LCG : public RNG {
   private:
    uint32_t Advance32() const override { return Advance64(); }

    uint64_t Advance64() const override {
        const auto x = m_State;
        m_State = (m_Mul * m_State + m_Inc) % m_Mod;
        return x;
    }

   public:
    // Creates a new LCG engine with a, c, m parameters equal to the default
    // engine. Seed is current time.
    LCG() = default;

    // Creates a new LCG engine with a, c, m parameters equal to the default
    // engine with custom seed value. Meant for reproducibility.
    explicit LCG(uint64_t seed) : m_State(seed) {}

    // Creates a new LCG engine with custom a, c, m parameters and seed equal to
    // current time.
    LCG(uint64_t multiplier, uint64_t increment, uint64_t modulo)
        : m_Mul(multiplier), m_Inc(increment), m_Mod(modulo) {}

    // Creates a new LCG engine.
    LCG(uint64_t seed, uint64_t multiplier, uint64_t increment, uint64_t modulo)
        : m_State(seed), m_Mul(multiplier), m_Inc(increment), m_Mod(modulo) {}

    // Getter for state value.
    [[nodiscard]] uint64_t GetState() const noexcept { return m_State; }

    // Getter for state multiplier.
    [[nodiscard]] uint64_t GetMul() const noexcept { return m_Mul; }

    // Getter for multiplied state increment.
    [[nodiscard]] uint64_t GetInc() const noexcept { return m_Inc; }

    // Getter for incremented state modulo.
    [[nodiscard]] uint64_t GetMod() const noexcept { return m_Mod; }

   private:
    mutable uint64_t m_State =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    uint64_t m_Mul = 6458928179451363983ULL;
    uint64_t m_Inc = 0;
    uint64_t m_Mod = ((1ULL << 63ULL) - 25ULL);
};

class PCG : public RNG {
   private:
    // (high_bits XOR low_bits) of x.
    static uint64_t Xorshift32(uint64_t x) {
        auto high_bits = static_cast<uint32_t>(x >> 32U);
        auto low_bits = static_cast<uint32_t>(x);
        return high_bits ^ low_bits;
    }

    static uint32_t Rotr32(uint64_t x, uint32_t rot) {
        return (x >> rot) | (x << ((-rot) & 31U));
    }

    uint64_t Advance() const {
        auto x = m_State;
        m_State = m_Mul * m_State + m_Inc;
        return x;
    }

   public:
    // PCG-XSH-RR
    uint32_t Advance32() const override {
        uint64_t x = Advance();
        auto rotation = static_cast<uint32_t>(x >> 59U);

        x = Xorshift32(x);
        x = Rotr32(x, rotation);
        return x;
    }

    // PCG-XSH-RR-RR
    uint64_t Advance64() const override {
        uint64_t x = Advance();
        uint32_t rotation = x >> 59U;

        // XSH
        uint32_t high_bits = x >> 32U;
        uint32_t low_bits = x;
        x = low_bits ^ high_bits;

        // RR-RR
        uint32_t x_low = Rotr32(x, rotation);
        uint64_t x_high = Rotr32(high_bits, x & 31U);
        x = (x_high << 32U) | x_low;

        return x;
    }

    PCG() = default;

    explicit PCG(uint64_t seed) : m_State(seed) {}

    PCG(uint64_t multiplier, uint64_t increment)
        : m_Mul(multiplier), m_Inc(increment) {}

    PCG(uint64_t seed, uint64_t multiplier, uint64_t increment)
        : m_State(seed), m_Mul(multiplier), m_Inc(increment) {}

    // Getter for state value.
    [[nodiscard]] uint64_t GetState() const { return m_State; }

    // Getter for state multiplier.
    [[nodiscard]] uint64_t GetMul() const { return m_Mul; }

    // Getter for multiplied state increment.
    [[nodiscard]] uint64_t GetInc() const { return m_Inc; }

   private:
    mutable uint64_t m_State =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const uint64_t m_Mul = 6364136223846793005UL;
    const uint64_t m_Inc = 1442695040888963407UL;
};
}  // namespace randshow
