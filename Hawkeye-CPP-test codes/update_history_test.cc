#include <iostream>
#include <map>
#include <vector>
#include "hawkeye_predictor.h"
#include "helper_function.h"
#include "optgen.h"
#include "hawkeye_algorithm.cc"

// Print the cache history sampler for testing
void printCacheHistorySampler(const std::vector<std::map<uint64_t, HISTORY>>& cache_history_sampler, unsigned int sample_set) {
    for (const auto& entry : cache_history_sampler[sample_set]) {
        const HISTORY& history = entry.second;
        std::cout << "Address: " << entry.first 
                  << ", PCval: " << history.PCval 
                  << ", PreviousVal: " << history.previousVal 
                  << ", LRU: " << history.lru 
                  << ", Prefetching: " << (history.prefetching ? "Yes" : "No") 
                  << std::endl;
    }
}

int main() {

InitReplacementState();
    // Initializing sample entries for one set (e.g., set 2)
    unsigned int sample_set = 2;
    for(uint64_t i = 0; i < 5; ++i) {
        HISTORY hist;
        hist.init();
        hist.lru = i;  // Set different LRU values
        cache_history_sampler[sample_set][i] = hist;
    }

    // Print the cache history before updating
    std::cout << "Before updating cache history:" << std::endl;
    printCacheHistorySampler(cache_history_sampler, sample_set);

    // Current time/access count
    unsigned int currentVal = 3;

    // Update cache history for set 2
    update_cache_history(sample_set, currentVal);

    // Print the cache history after updating
    std::cout << "\nAfter updating cache history:" << std::endl;
    printCacheHistorySampler(cache_history_sampler, sample_set);

    return 0;
}
