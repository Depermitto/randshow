// link: https://www.youtube.com/watch?v=9NvxDAUF_kI
// code inspiration link: https://cse.usf.edu/~kchriste/tools/toolpage.html
// TODO: complete compatibility with
// https://en.cppreference.com/w/cpp/named_req/RandomNumberDistribution

#pragma once
#include <cassert>
#include <random>
#include <stdexcept>

namespace randshow {
// @brief A discrete distribution in which nth entry occurs 1/n times of the
// most common entry.
//
// Attributed to George Zipf, most commonly used to describe frequency of words
// in a text or language.
//
// @ingroup randshow
template <class RealType = double>
class ZipfDistribution {
   public:
    using result_type = RealType;

    ZipfDistribution() = delete;

    ZipfDistribution(size_t population_count, RealType distribution_param = 1.0)
        : n_(population_count), s_(distribution_param) {
        assert(population_count >= 1);
        assert(distribution_param >= 1);

        for (size_t i = 1; i <= n_; i++) {
            c_ += (1.0 / std::pow(i, s_));
        }
        c_ = 1.0 / c_;
    }

    template <class UniformRandomBitGenerator>
    RealType operator()(UniformRandomBitGenerator& g) {
        static std::uniform_real_distribution<> dist(std::nextafter(0.0, 1.0),
                                                     std::nextafter(1.0, 0.0));

        const auto z = dist(g);
        long double sum_prob = 0.0;
        for (size_t x = 1; x <= n_; x++) {
            sum_prob += c_ / std::pow(x, s_);
            if (sum_prob >= z) {
                return x;
            }
        }
        throw std::runtime_error("Unreachable Code");
    }

   private:
    size_t n_;             // population count
    RealType s_;           // distribution parameter
    long double c_ = 0.0;  // normalization constant
};
}  // namespace randshow