#include <iostream>
#include <cmath>

#define bitmask(l) (((l) == 64) ? (unsigned long long)(-1LL) : ((1LL << (l))-1LL))
#define bits(x, i, l) (((x) >> (i)) & bitmask(l))
#define SAMPLED_SET(set) (bits(set, 0 , 6) == bits(set, ((unsigned long long)log2(4096) - 6), 6))

int main() {
    // Test the bitmask and bits function
    for (int l = 1; l <= 64; ++l) {
        std::cout << "bitmask(" << l << "): " << bitmask(l) << std::endl;
    }

    unsigned long long test_val = 0xF0F0F0F0F0F0F0F0;  // Sample value for testing
    std::cout << "bits(" << test_val << ", 4, 8): " << bits(test_val, 4, 8) << std::endl;

    // Test the SAMPLED_SET macro
    for (int set = 0; set < 100; ++set) {
        std::cout << "Set " << set << " is " << (SAMPLED_SET(set) ? "" : "not ") << "sampled." << std::endl;
    }

    return 0;
}
