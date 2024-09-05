
#include <iostream>
#include <map>
#include <vector>
#include "hawkeye_predictor.h"
#include "helper_function.h"
#include "optgen.h"
#include "hawkeye_algorithm.cc"

// Assuming DEMAND is a constant representing the type of access.
// Replace 0 with the actual value if it's different.
const uint32_t DEMAND = 0;

void printState(uint32_t set) {
    // Ensure that LLC_SETS and SAMPLER_SETS are greater than zero
    if (LLC_SETS > 0 && SAMPLER_SETS > 0) {
        // Calculate the index for the sample set array
        uint32_t index = (LLC_SETS * set) / SAMPLER_SETS;
        if (index < cache_history_sampler.size()) {
            auto& sample_set = cache_history_sampler[index];

            std::cout << "RRIP State for set " << set << ": ";
            for (int i = 0; i < LLC_WAYS; ++i) {
                std::cout << rrip[set][i] << " ";
            }
            std::cout << "\nPrefetching State for set " << set << ": ";
            for (int i = 0; i < LLC_WAYS; ++i) {
                std::cout << prefetching[set][i] << " ";
            }
            std::cout << "\nSample Signature for set " << set << ": ";
            for (int i = 0; i < LLC_WAYS; ++i) {
                std::cout << sample_signature[set][i] << " ";
            }
            std::cout << "\nSet Timer for set " << set << ": " << set_timer[set] << "\n";
            
            for (auto& entry : sample_set) {
                std::cout << "Address: " << entry.first << ", History: { PCval: " << entry.second.PCval 
                        << ", PreviousVal: " << entry.second.previousVal << ", LRU: " << entry.second.lru 
                        << ", Prefetching: " << (entry.second.prefetching ? "Yes" : "No") << " }\n";
            }
            std::cout << "\n";
        } else {
            std::cerr << "Invalid set index: " << set << std::endl;
        }
    } else {
        std::cerr << "LLC_SETS or SAMPLER_SETS not properly defined." << std::endl;
    }
}
int main() {
    // Initialize the replacement state
    InitReplacementState();

    uint32_t set = 0;

    std::cout << "Initial State:\n";
    printState(set);

    // Simulate a prefetch miss
    UpdateReplacementState(0, set, 0, 0xABCDEF, 0x12345, 0, PREFETCH, true);
    std::cout << "After Prefetch Hit:\n";
    bool prefetch_prediction = predictor_prefetch->get_prediction(0x12345);
    std::cout << " Prefetch prediction:" << prefetch_prediction << endl;
    printState(set);

    UpdateReplacementState(0, set, 0, 0xABCDEF, 0x12345, 0, PREFETCH, 0);
    std::cout << "After Prefetch Miss:\n";
    prefetch_prediction = predictor_prefetch->get_prediction(0x12345);
    std::cout << " Prefetch prediction:" << prefetch_prediction << endl;
    printState(set); 

    UpdateReplacementState(0, set, 0, 0xABCDEF, 0x12345, 0, PREFETCH, 0);
    std::cout << "After Prefetch Miss:\n";
    prefetch_prediction = predictor_prefetch->get_prediction(0x12345);
    std::cout << " Prefetch prediction:" << prefetch_prediction << endl;
    printState(set);

    // Simulate a demand access hit
    UpdateReplacementState(0, set, 0, 0xABCDEF, 0x12345, 0, DEMAND, 1);
    std::cout << "After Demand Hit:\n";
    // Check prediction for demand
    bool demandPrediction = predictor_demand->get_prediction(0x12345);
    std::cout << "Demand Prediction: " << demandPrediction << "\n";
    printState(set);

    UpdateReplacementState(0, set, 1, 0xABCDEF, 0x12345, 0, DEMAND, 0);
    std::cout << "After Demand Miss:\n";
    // Check prediction for demand
    demandPrediction = predictor_demand->get_prediction(0x12345);
    std::cout << "Demand Prediction: " << demandPrediction << "\n";
    printState(set);

    return 0;
}
