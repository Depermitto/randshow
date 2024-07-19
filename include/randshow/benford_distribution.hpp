#pragma once
#include <cassert>
#include <random>
#include <stdexcept>
#include <type_traits>

namespace randshow {
template <class UIntType = uint8_t,
          typename std::enable_if<std::is_unsigned<UIntType>::value,
                                  bool>::type = true>
class BenfordDistribution {
   public:
    using result_type = UIntType;

    BenfordDistribution() = default;

    BenfordDistribution(UIntType base) : base_(base) { assert(base > 2); }

    template <class UniformRandomBitGenerator>
    UIntType operator()(UniformRandomBitGenerator& g) {
        static std::uniform_real_distribution<> dist(std::nextafter(0.0, 1.0),
                                                     std::nextafter(1.0, 0.0));

        const auto z = dist(g);
        long double sum_prob = 0.0;
        for (size_t d = 1; d <= base_; d++) {
            sum_prob += std::log(1.0 + 1.0 / d) / std::log(base_);
            if (sum_prob >= z) {
                return d;
            }
        }
        throw std::runtime_error("Unreachable Code");
    }

   private:
    UIntType base_ = 10;
};
}  // namespace randshow
