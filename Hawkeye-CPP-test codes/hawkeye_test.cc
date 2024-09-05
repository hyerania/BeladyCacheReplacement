#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <random>
#include "hawkeye_algorithm.cc"  // Include your Hawkeye algorithm header file

#define DEMAND 0
// const uint32_t PREFETCH = 1;  // Define the PREFETCH constant

struct CacheAccess {
    uint32_t set;
    uint32_t way;
    uint32_t type;
    bool is_hit;
    uint64_t paddr;
    uint64_t PC;
};

// void simulate_cache_access(const CacheAccess& access) {
//     UpdateReplacementState(0, access.set, access.way, access.paddr, access.PC, 0, access.type, access.is_hit);
// }


void print_cache_state(uint32_t set) {
    // Implement this based on your Hawkeye algorithm structure
}

// CacheAccess parse_access(const std::string& line) {
//     std::istringstream iss(line);
//     CacheAccess access;
//     char comma;
//     std::string accessType;

//     iss >> access.set >> comma >> access.way >> comma >> accessType >> comma 
//         >> access.is_hit >> comma >> std::hex >> access.paddr >> comma >> access.PC;
//     std::dec(iss); // Switch back to decimal mode for subsequent reads

//     access.type = (accessType == "PREFETCH") ? PREFETCH : DEMAND;

//     return access;
// }

// int main() {
//     std::string filePath = "/mnt/e/F3CAS/Python-code/Hawkeye/cache_accesses.txt";
//     std::ifstream file(filePath);
//     std::string line;
//     std::vector<CacheAccess> cache_access_sequence;

//     if (!file) {
//         std::cerr << "Unable to open file at " << filePath << std::endl;
//         return 1;
//     }

//     while (std::getline(file, line)) {
//         cache_access_sequence.push_back(parse_access(line));
//     }

//     InitReplacementState();

//     for (const auto& access : cache_access_sequence) {
//         std::cout << "Simulating Cache Access: Set=" << access.set 
//                   << ", Way=" << access.way 
//                   << ", Type=" << (access.type == PREFETCH ? PREFETCH : DEMAND)
//                   << ", Hit=" << (access.is_hit ? "True" : "False") 
//                   << ", Addr=" << std::hex << access.paddr 
//                   << ", PC=" << std::dec << access.PC << std::endl;

//         simulate_cache_access(access);
//         print_cache_state(access.set);
//     }

//     // Replace these with the correct function calls if available.
//     PrintStats_Heartbeat();
//     PrintStats();

//     return 0;
// }


// int main() {
//     // std::string filePath = "/mnt/e/F3CAS/Python-code/Hawkeye/cache_accesses.txt";
//     std::ifstream inFile("/mnt/e/F3CAS/Python-code/Hawkeye/cache_accesses.txt");
//     if (!inFile.is_open()) {
//             std::cerr << "Failed to open the file for reading." << std::endl;
//             return 1;
//         }


//     InitReplacementState();
//     std::string line;
//     while (std::getline(inFile, line)) {
//         std::istringstream iss(line);
//         uint32_t set, way;
//         std::string accessType, hit;
//         uint64_t paddr, PC;

//         iss >> set >> way;
//         iss.ignore(2); // Ignore the comma and space
//         iss >> accessType >> hit >> std::hex >> paddr >> std::hex >> PC;

//         bool is_hit = (hit == "True") ? true : false;
//         uint32_t type = (accessType == "PREFETCH") ? PREFETCH : DEMAND;

//         std::cout << "Simulating Cache Access: Set=" << set 
//                   << ", Way=" << way 
//                   << ", Type=" << (accessType == "PREFETCH" ? PREFETCH : DEMAND)
//                   << ", Hit=" << (is_hit ? "True" : "False") 
//                   << ", Addr=" << std::hex << paddr 
//                   << ", PC=" << std::dec << PC << std::endl;

//         simulate_cache_access(set, way, type, is_hit, paddr, PC);
//         print_cache_state(set);
//     }

//     inFile.close();

//     // Replace these with the correct function calls if available.
//     PrintStats_Heartbeat();
//     PrintStats();

//     return 0;
// }

int main() {
    // Seed the random number generator
    std::mt19937 generator(42);
    std::uniform_int_distribution<uint32_t> set_distribution(0, LLC_SETS -1 );  // Assuming LLC_SETS = 2048
    std::uniform_int_distribution<uint32_t> way_distribution(0, LLC_WAYS -1 );    // Assuming LLC_WAYS = 16
    std::uniform_int_distribution<uint32_t> type_distribution(0, 3);    // 0 for DEMAND, 2 for PREFETCH, 3 for WRITEBACK
    std::uniform_int_distribution<uint32_t> hit_distribution(0, 1);     // 0 for False, 1 for True
    std::uniform_int_distribution<uint64_t> addr_distribution(0x1000, 0xFFFF);
     std::uniform_int_distribution<uint64_t> PC_distribution(0x1000, 0xFFFF);

    // Open a file for writing
    std::ofstream outFile("cache_accesses.txt");
    if (!outFile.is_open()) {
        std::cerr << "Failed to open the cache_accesses for writing." << std::endl;
        return 1;
    }
    // VictimWay file
        // Open a file for writing
    std::ofstream VictimFile("VictimWays.txt");
    if (!VictimFile.is_open()) {
        std::cerr << "Failed to open the file for writing." << std::endl;
        return 1;
    }

    InitReplacementState();

    // Generate and write random cache accesses
    for (int i = 0; i < 9000; ++i) {
        CacheAccess access;
        access.set = set_distribution(generator);
        access.way = way_distribution(generator);
        access.type = type_distribution(generator);     // == 0 ? DEMAND : PREFETCH;
        access.is_hit = hit_distribution(generator) == 0 ? false : true;
        access.paddr = addr_distribution(generator);
        access.PC = PC_distribution(generator);

        // Write to file
        outFile << access.set << ", " << access.way << ", " 
                << access.type << ", " 
                << (access.is_hit ? "True" : "False") << ", " 
                << access.paddr << ", " << access.PC  << "\n";

        // Print to console
        // std::cout << "Simulating Cache Access: Set=" << access.set 
        //           << ", Way=" << access.way 
        //           << ", Type=" << (access.type == PREFETCH ? "PREFETCH" : "DEMAND")
        //           << ", Hit=" << (access.is_hit ? "True" : "False") 
        //           << ", Addr=0x" << std::hex << access.paddr 
        //           << ", PC=0x" << access.PC << std::dec << std::endl;

        // simulate_cache_access(access);
        
        print_cache_state(access.set);
        int victim_way = GetVictimInSet(0, access.set, 0, access.PC, access.paddr, access.type);
        UpdateReplacementState(0, access.set, victim_way, access.paddr, access.PC, 0, access.type, access.is_hit);
        // //SM
        // cout << "rrip array " << endl;
        // for (uint32_t i = 0; i < LLC_WAYS; i++) {
        //     cout << rrip[access.set][i] << " ";
        // }
        // cout << endl;  // Newline for each set
   
        //  //endSM
        // cout<<"Victim way:"<< victim_way<<endl;
        VictimFile << "0x"<< access.paddr << " -> " << "VictimWay: " << victim_way << ", set: " << access.set <<endl;
        
    }

    // Close the file
    outFile.close();
    VictimFile.close();

    std::cout << "Cache access sequence has been written to cache_accesses.txt" << std::endl;

    PrintStats_Heartbeat();
    PrintStats();

    return 0;
}
// int main() {
//     std::vector<CacheAccess> cache_access_sequence = {
//         {0, 0, PREFETCH, false, 0x1000, 0x2000},
//         {0, 0, PREFETCH, true, 0x1000, 0x2000},
//         {0, 0, DEMAND, false, 0x1000, 0x2000},
//         {0, 0, PREFETCH, false, 0x1000, 0x2000},
//         {0, 0, PREFETCH, false, 0x1000, 0x4000},
//         {0, 0, DEMAND, false, 0x1000, 0x4000},
//         {0, 0, PREFETCH, false, 0x1000, 0x2000},
//         {0, 0, PREFETCH, true, 0x1000, 0x2000},
//         {0, 0, DEMAND, false, 0x1000, 0x2000},
//         {0, 0, PREFETCH, false, 0x1000, 0x2000},
//         {0, 0, PREFETCH, false, 0x1000, 0x4000},
//         {0, 0, PREFETCH, false, 0x1000, 0x4000},
//         {0, 0, DEMAND, true, 0x1000, 0x4000},
//         {0, 0, DEMAND, false, 0x1000, 0x4000},
//         {0, 0, DEMAND, true, 0x1000, 0x4000},
//         {0, 0, DEMAND, false, 0x1000, 0x4000},
//         // ... add more accesses as needed
//     };

//     InitReplacementState();

//     for (const auto& access : cache_access_sequence) {
//         std::cout << "Simulating Cache Access: Set=" << access.set 
//                   << ", Way=" << access.way 
//                   << ", Type=" << (access.type == PREFETCH ? "PREFETCH" : "DEMAND")
//                   << ", Hit=" << (access.is_hit ? "true" : "false") 
//                   << ", Addr=" << std::hex << access.paddr 
//                   << ", PC=" << std::dec << access.PC << std::endl;

//         simulate_cache_access(access);
//         print_cache_state(access.set);
//     }

//     // Replace these with the correct function calls if available.
//     PrintStats_Heartbeat();
//     PrintStats();

//     return 0;
// }
