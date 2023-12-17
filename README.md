# Random number generators for **C++** in a single header library
Every random number generator in this library has 64-bit state and can output either 32-bit and 64-bit unsigned integers, as well as floating point numbers. Even the most basic *LCG* random engine will perform better than the **rand** function from **C**.

### Engines
> Links point to wikipedia articles
- Popular, basic and fast, albeit not statistically impressive, the [Linear Congruential Generator](https://en.wikipedia.org/wiki/Linear_congruential_generator?useskin=vector)
- Much improved version of LCG, the [Permuted Congruential Generator](https://en.wikipedia.org/wiki/Permuted_congruential_generator?useskin=vector) *RECOMMENDED*

# Usage
Simply download the contents of *include* directory and `#include "randshow.hpp"`. Includes the Random Number Generator - the `RNG` interface if you wish to extend the functionality of the library.

### Examples
```C++
auto engine = randshow::pcg::PCG(17);  // Custom seed
uint32_t num = engine.next32();  // Completely random 32-bit unsigned integer
uint32_t num_4_7 = engine.next32_range(4, 7);  // Random number between 4 and 7
double_t num_0_1 = engine.nextfp();  // Standard uniform distribution
bool is_heads = engine.coin_flip();  // Both balanced and weighted coin flip methods are included
```

```C++
// Create a 100 element array filled with random 64-bit unsigned integers
auto nums = std::array<uint64_t, 100>();
for (auto &elt : nums) {
    elt = randshow::pcg::default_engine.next64_range(0, 10);  // You can use default engine without instanciating your own
}
```

# License
Licensed under the MIT license
