#include <iostream>
#include "hawkeye_predictor.h"
#include "helper_function.h"
#include "optgen.h"
#include "hawkeye_algorithm.cc"



int main() {
    // Initialize state
    InitReplacementState();

    // Optionally print the state after initialization
    // for example, print rrip array
    for (int i = 0; i < LLC_SETS; ++i) {
        for (int j = 0; j < LLC_WAYS; ++j) {
            std::cout << rrip[i][j] << " ";
        }
        std::cout << std::endl;
    }
    rrip[0][0] = 3;
    rrip[0][1] = 6;
    rrip[0][2] = 0;
    rrip[0][3] = 5;

    // Test get_victim_in_set function
    uint32_t cpu = 0;
    uint32_t set = 0; // Example set number
    uint64_t PC = 12345; // Example PC value
    uint64_t paddr = 0xABCD; // Example physical address
    uint32_t type = 0; // Define based on your access type constants

    // Print rrip before finding victim
    std::cout << "rrip before finding victim:" << std::endl;
    for (int i = 0; i < LLC_WAYS; ++i) {
        std::cout << rrip[set][i] << " ";
    }
    std::cout << std::endl;

    // Call get_victim_in_set
    uint32_t victim = GetVictimInSet(cpu, set, nullptr, PC, paddr, type);
    std::cout << "Victim way in set " << set << ": " << victim << std::endl;

    // Print rrip after finding victim
    std::cout << "rrip after finding victim:" << std::endl;
    for (int i = 0; i < LLC_WAYS; ++i) {
        std::cout << rrip[set][i] << " ";
    }
    std::cout << std::endl;

    return 0;
}