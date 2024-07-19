#include <iostream>
#include <randshow/engines.hpp>
#include <unordered_map>

int main() {
    std::unordered_map<int, int> counter{};
    std::poisson_distribution<> dist{10};

    for (int n = 1000; n--;) {
        counter[dist(randshow::DefaultEngine)] += 1;
    }

    for (size_t i = counter.size(); i--;) {
        std::string count(counter[i], '*');
        std::cout << i << ": " << count << "\n";
    }
}
