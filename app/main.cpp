#include <iostream>
#include <randshow/engines.hpp>
#include <randshow/zipf_distribution.hpp>
#include <unordered_map>

int main() {
    std::unordered_map<int, int> counter{};
    randshow::ZipfDistribution<> dist(10, 1);

    for (int n = 200; n--;) {
        counter[dist(randshow::DefaultEngine)] += 1;
    }

    for (size_t i = 1; i < counter.size(); i++) {
        std::string count(counter[i], '*');
        std::cout << i << ": " << count << "\n";
    }
}
