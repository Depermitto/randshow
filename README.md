# Single-header library with many RNGs for **C++**

> Quality tested using [PractRand](https://pracrand.sourceforge.net/)

# Capabilities

Randshow aims to be smoother in use and 'more random' that engines found in the _\<random\>_ header of C++11. It ensures compatibility with [`UniformRandomBitGenerator`](https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator) and C++11 _\<random\>_ distributions.

### Engines

- [PCG](https://www.pcg-random.org/) with 64-bit state and 32-bit output as well as a variant with 128-bit state and 64-bit output.
- [Xoshiro256++](https://prng.di.unimi.it/)
- [SplitMix64](https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64#bodyContent)
- [LCG](https://en.wikipedia.org/wiki/Linear_congruential_generator?useskin=vector)

# Usage

Download the contents of _include_ directory and include the `"randshow.hpp"` header in your code.

### Examples

```C++
randshow::PCG32 rng{17}                 // Custom seed
uint32_t num = rng.Next()               // Random 32-bit unsigned integer
uint32_t num_4_17 = rng.Next(4, 17);    // Equivalent to creating a new std::uniform_int_distribution
double_t num_0_1 = rng.NextReal();      // Random number in (0.0, 1.0) range
```

```C++
// Create a histogram in Poisson distribution
randshow::PCG32 rng{};
std::unordered_map<int, int> counter{};
std::poisson_distribution<> dist{10};

for (int n = 1000; n--;) {
    counter[dist(rng)] += 1;
}

for (size_t i = counter.size(); i--;) {
    std::string count(counter[i], '*');
    std::cout << i << ": " << count << "\n";
}
```

```C++
randshow::Shuffle(v.begin(), v.end(), rng);     // Very similar to std::shuffle
const auto samples_wihout_replacement = randshow::Sample(v.begin(), v.end(), 10, rng);     // Note: This function allocates an std::vector with capacity equal 10.
```

# License

Licensed under the MIT license
