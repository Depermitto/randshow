#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>

namespace randshow {
class RNG {
   public:
    // Next random 32-bit unsigned int.
    [[nodiscard]] virtual uint32_t next32() const {
        return advance32();
    }

    // Next random 32-bit unsigned int from uniform distribution in [0, n) range.
    [[nodiscard]] virtual uint32_t next32(uint32_t n) const {
        return next32() % n;
    }

    // Next random 32-bit signed int from uniform distribution in [a, b) range.
    [[nodiscard]] virtual int32_t next32(int32_t a, int32_t b) const {
        return b > a ? next32() % (b - a) + a : a;
    }

    // Next random 64-bit unsigned int.
    [[nodiscard]] virtual uint64_t next64() const {
        return advance64();
    }

    // Next random 64-bit unsigned int from uniform distribution in [0, n) range.
    [[nodiscard]] virtual uint64_t next64(uint64_t n) const {
        return next64() % n;
    }

    // Next random 64-bit unsigned int from uniform distribution in [a, b) range.
    [[nodiscard]] virtual int64_t next64(int64_t a, int64_t b) const {
        return b > a ? next64() % (b - a) + a : a;
    }

    // Next floating point number from standard uniform distribution i.e. (0, 1)
    // range.
    [[nodiscard]] virtual double nextfp() const {
        return static_cast<double>(next32()) / UINT32_MAX;
    }

    // Floating point uniform distribution in [a, b) range.
    [[nodiscard]] virtual double nextfp(double a, double b) const {
        return nextfp() * (b - a) + a;
    }

    // Balanced coin flip with 50% chance of heads and tails.
    [[nodiscard]] virtual bool heads() const {
        return nextfp() < 0.5;
    }

    // Weighted coin flip. Parameter weight must be in (0, 1) range.
    // Weight smaller or equal than 0 will always yield false and
    // bigger or equal to 1 always yielding true.
    [[nodiscard]] virtual bool heads(double weight) const {
        return nextfp() < weight;
    }

   private:
    [[nodiscard]] virtual uint32_t advance32() const = 0;
    [[nodiscard]] virtual uint64_t advance64() const = 0;
};

namespace lcg {
// LCG or Linear Congruential Generator has very low footprint and is very fast.
// It has m_modulo limit of generating pseudorandom numbers. LCGs are suitable
// for games or other trivial use-cases, but shouldn't be used for work
// requiring true random numbers.
//
// This LCG implementation requires 32 bytes of memory per instance.
class LCG : public RNG {
   public:
    uint32_t advance32() const override {
        return advance64();
    }

    uint64_t advance64() const override {
        auto x = m_State;
        m_State = (m_Mul * m_State + m_Inc) % m_Mod;
        return x;
    }

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
    [[nodiscard]] uint64_t state() const noexcept {
        return m_State;
    }

    // Getter for state multiplier.
    [[nodiscard]] uint64_t mul() const noexcept {
        return m_Mul;
    }

    // Getter for multiplied state increment.
    [[nodiscard]] uint64_t inc() const noexcept {
        return m_Inc;
    }

    // Getter for incremented state modulo.
    [[nodiscard]] uint64_t mod() const noexcept {
        return m_Mod;
    }

   private:
    mutable uint64_t m_State = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    uint64_t m_Mul = 6458928179451363983ULL;
    uint64_t m_Inc = 0;
    uint64_t m_Mod = ((1ULL << 63ULL) - 25ULL);
};
}  // namespace lcg

namespace pcg {
class PCG : public RNG {
   private:
    // (high_bits XOR low_bits) of x.
    static uint64_t xorshift32(uint64_t x) {
        auto high_bits = static_cast<uint32_t>(x >> 32U);
        auto low_bits = static_cast<uint32_t>(x);
        return high_bits ^ low_bits;
    }

    static uint32_t rotr32(uint64_t x, uint32_t rot) {
        return (x >> rot) | (x << ((-rot) & 31U));
    }

    uint64_t advance() const {
        auto x = m_State;
        m_State = m_Mul * m_State + m_Inc;
        return x;
    }

   public:
    // PCG-XSH-RR
    uint32_t advance32() const override {
        uint64_t x = advance();
        auto rotation = static_cast<uint32_t>(x >> 59U);

        x = xorshift32(x);
        x = rotr32(x, rotation);
        return x;
    }

    // PCG-XSH-RR-RR
    uint64_t advance64() const override {
        uint64_t x = advance();
        auto rotation = static_cast<uint32_t>(x >> 59U);

        // XSH
        auto high_bits = static_cast<uint32_t>(x >> 32U);
        auto low_bits = static_cast<uint32_t>(x);
        x = low_bits ^ high_bits;

        // RR-RR
        auto x_low = rotr32(x, rotation);
        auto x_high = rotr32(high_bits, x & 31U);
        x = (static_cast<uint64_t>(x_high) << 32U) | x_low;
        return x;
    }

    PCG() = default;

    explicit PCG(uint64_t seed) : m_State(seed) {}

    PCG(uint64_t multiplier, uint64_t increment) : m_Mul(multiplier), m_Inc(increment) {}

    PCG(uint64_t seed, uint64_t multiplier, uint64_t increment)
        : m_State(seed), m_Mul(multiplier), m_Inc(increment) {}

    // Getter for state value.
    [[nodiscard]] uint64_t state() const {
        return m_State;
    }

    // Getter for state multiplier.
    [[nodiscard]] uint64_t mul() const {
        return m_Mul;
    }

    // Getter for multiplied state increment.
    [[nodiscard]] uint64_t inc() const {
        return m_Inc;
    }

   private:
    mutable uint64_t m_State = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const uint64_t m_Mul = 6364136223846793005UL;
    const uint64_t m_Inc = 1442695040888963407UL;
};

static const PCG default_engine = PCG();
}  // namespace pcg
}  // namespace randshow
