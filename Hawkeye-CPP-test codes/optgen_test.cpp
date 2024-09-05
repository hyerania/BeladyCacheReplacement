#include <iostream>
#include <unordered_map>
#include "optgen.h"  // Ensure this is the correct path to your OPTgen header

void test_OPT_algorithm() {
    uint64_t cache_size = 2;  // Cache size is 2, as per the set associativity
    OPTgen optgen;
    optgen.init(cache_size);
    uint64_t misses = 0;

    // Access sequence as provided
    char accesses[] = {'A', 'B', 'C', 'D', 'A', 'E', 'B', 'C', 'D', 'A', 'E', 'B', 'F', 'D','F','A','A', 'C', 'D', 'A', 'E', 'B', 'B', 'C', 'D', 'A', 'E', 'B', 'F', 'D','F','A','C','C', 'D', 'A', 'E', 'B','A', 'B', 'C', 'D', 'A', 'E', 'B', 'C', 'D', 'A', 'E', 'B', 'F', 'D','F','A','A', 'C', 'D', 'A', 'E', 'B', 'B', 'C', 'D', 'A', 'E', 'B', 'F', 'D','F','A','C','C', 'D', 'A', 'E', 'B'};
    size_t num_accesses = sizeof(accesses) / sizeof(char);

    // Map to keep track of the previous access time for each address
    std::unordered_map<char, uint64_t> last_access_map;

    // Process each access
    for (size_t i = 0; i < num_accesses; ++i) {
        char address = accesses[i];
        uint64_t currentVal = i;  // currentVal is the access index

        // Check if the address has been accessed before or NOT
        if (last_access_map.count(address) > 0) {
            // Address has been accessed before, so call is_cache
            uint64_t previousVal = last_access_map[address];
            bool isHit = optgen.is_cache(currentVal, previousVal);

            std::cout << "Access to address " << address;
            std::cout << " current time " << currentVal << " previous time " << previousVal << (isHit ? " is a hit" : " is a miss") << std::endl;

            if (!isHit) {
                // optgen.set_access(currentVal);
                misses ++;
            }
        } else {
            // First access to this address, treat as a miss
            std::cout << "Access to address " << address;
            std::cout << " current time " << currentVal << " --->(first access)" << std::endl;

            optgen.set_access(currentVal);
        }

        // Update the last access time for this address
        last_access_map[address] = currentVal;
    }

    // Output the total number of hits and missed
    std::cout << "Total number of cache hits: " << optgen.get_optgen_hits() << std::endl;
    std::cout << "Total number of cache misses " <<misses<< std::endl;
}

int main() {
    test_OPT_algorithm();
    return 0;
}
