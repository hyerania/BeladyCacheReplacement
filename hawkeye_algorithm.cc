/*  Hawkeye with Belady's Algorithm Replacement Policy
    Code for Hawkeye configurations of 1 and 2 in Champsim */

#include "./inc/champsim_crc2.h"
#include <map>
#include <math.h>
#include <fstream>

std::ofstream VictimFile("VictimWaysfinal1.txt", std::ios::out);
std::ofstream file("functioncall1.txt", std::ios::out);

#define NUM_CORE 1
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16

//3-bit RRIP counter
#define MAXRRIP 7
uint32_t rrip[LLC_SETS][LLC_WAYS];

#include "hawkeye_predictor.h"
#include "optgen.h"
#include "helper_function.h"

//Hawkeye predictors for demand and prefetch requests
Hawkeye_Predictor* predictor_demand;    //2K entries, 5-bit counter per each entry
Hawkeye_Predictor* predictor_prefetch;  //2K entries, 5-bit counter per each entry

OPTgen optgen_occup_vector[LLC_SETS];   //64 vecotrs, 128 entries each

//Prefectching
bool prefetching[LLC_SETS][LLC_WAYS];

//Sampler components tracking cache history
#define SAMPLER_ENTRIES 2800
#define SAMPLER_HIST 8
#define SAMPLER_SETS (SAMPLER_ENTRIES/SAMPLER_HIST)
vector<map<uint64_t, HISTORY>> cache_history_sampler;  //2800 entries, 4-bytes per each entry
uint64_t sample_signature[LLC_SETS][LLC_WAYS];

//History time
#define TIMER_SIZE 1024
uint64_t set_timer[LLC_SETS];   //64 sets, where 1 timer is used for every set

//Mathmatical functions needed for sampling set
#define bitmask(l) (((l) == 64) ? (unsigned long long)(-1LL) : ((1LL << (l))-1LL))
#define bits(x, i, l) (((x) >> (i)) & bitmask(l))
#define SAMPLED_SET(set) (bits(set, 0 , 6) == bits(set, ((unsigned long long)log2(LLC_SETS) - 6), 6) )  //Helper function to sample 64 sets for each core


// Initialize replacement state
void InitReplacementState()
{
    cout << "Initialize Hawkeye replacement policy state" << endl;

    if (file.is_open()) {
        file <<  "Initialize Hawkeye replacement policy state" << endl;
    } else {
        std::cerr << "Unable to open the functioncall file for writing." << std::endl;
    }


// ///////////////////////////////////////////////////////////

    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            rrip[i][j] = MAXRRIP;
            sample_signature[i][j] = 0;
            prefetching[i][j] = false;
        }
        set_timer[i] = 0;
        optgen_occup_vector[i].init(LLC_WAYS-2);
    }

    cache_history_sampler.resize(SAMPLER_SETS);
    for(int i = 0; i < SAMPLER_SETS; i++){
        cache_history_sampler[i].clear();
    }

    predictor_prefetch = new Hawkeye_Predictor();
    predictor_demand = new Hawkeye_Predictor();

    cout << "Finished initializing Hawkeye replacement policy state" << endl;
    // std::ofstream file("functioncall.txt", std::ios::out);
    if (file.is_open()) {
        file <<  "Finished initializing Hawkeye replacement policy state" << endl;
    } else {
        std::cerr << "Unable to open the functioncall file for writing." << std::endl;
    }
}

// Find replacement victim
// Return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set , uint64_t PC, uint64_t paddr, uint32_t type)
{



    if (!VictimFile.is_open()) {
        std::cerr << "Unable to open the VictimWaysfinal for writing." << std::endl;
        UINT32_MAX;
    }

     if (SAMPLED_SET(set)){
      if (set == 1519){

            // Print the function arguments
    // cout << "Function GetVictimInSet called with arguments:"
    //      << "  cpu: " << cpu << ", "
    //      << "  set: " << set << ", "
    //      << "  current_set: " << current_set <<", "
    //      << "  PC: " << PC << ", "
    //      << "  paddr: " << paddr << ", "
    //      << "  type: " << type << endl;

         
        if (file.is_open()) {
            file << "Function GetVictimInSet called with arguments: "
                << "cpu: " << cpu << ", "
                << "set: " << set << ", "
                << "current_set: " << current_set << ", " // Note: This will print the pointer value
                << "PC: " << PC << ", "
                << "paddr: " << paddr << ", "
                << "type: " << type << std::endl;
            // file.close();
        } else {
            std::cerr << "Unable to open the functioncall file for writing." << std::endl;
        }

      }
     }
// Print the rrip array at the start
    // cout << "Initial rrip array for Set " << set << ":" << endl;
    // for (uint32_t i = 0; i < LLC_WAYS; i++) {
        // cout << rrip[set][i] << " ";
    // }
    // cout << endl;

    // Find the line with RRPV of 7 in that set
    // cout << "Searching for Victim Way with RRPV of 7..." << endl; 
    for(uint32_t i = 0; i < LLC_WAYS; i++){
        if(rrip[set][i] == MAXRRIP){
            // cout << "Victim Way Found (RRPV=7): " << i << ", set: " << set << endl;
        if (SAMPLED_SET(set)){
        if (set == 1519){
            VictimFile << "0x" << paddr << " -> VictimWay: " << i << ", set: " << set << std::endl;
        //  }
            return i;
                 }
              }
        }
    }

   
    // If no RRPV of 7, find the next highest RRPV value (oldest cache-friendly line)

    // cout << "Searching for Victim Way without RRPV of 7..." << endl; 
    uint32_t max_rrpv = 0;
    int32_t victim = -1;
    for(uint32_t i = 0; i < LLC_WAYS; i++){
        if(rrip[set][i] >= max_rrpv){
            max_rrpv = rrip[set][i];
             // cout << "Victim Way (RRPV<7) Found: " << i << ", set: " << set << endl;
                victim = i;
        }
    }

    //Asserting that LRU victim is not -1
    //Predictor will be trained negaively on evictions
    if(SAMPLED_SET(set)){
        if (set == 1519){
            if(prefetching[set][victim]){
                predictor_prefetch->decrease(sample_signature[set][victim]);
            }
            else{
                predictor_demand->decrease(sample_signature[set][victim]);
                // cout << "Decrease in the Victimway: " << victim << ",for PC in the sample_signature: " << sample_signature[set][victim] << "PC:" << PC << "set: " << set << endl;
            }
   
    // Print the rrip array at the point of finding a victim
    // cout << "rrip array at the point of finding a victim for Set " << set << ":" << endl;
    // for (uint32_t i = 0; i < LLC_WAYS; i++) {
        // cout << rrip[set][i] << " ";
    // }
    // cout << endl;
  
    
    
    VictimFile << "0x" << paddr << " -> VictimWay: " << victim << ", set: " << set << std::endl;
  }
    }

    return victim;
}

//Helper function for "UpdateReplacementState" to update cache history
void update_cache_history(unsigned int sample_set, unsigned int currentVal){
    for(map<uint64_t, HISTORY>::iterator it = cache_history_sampler[sample_set].begin(); it != cache_history_sampler[sample_set].end(); it++){

        // if (file.is_open()) {
        
        //      file << "Function update_cache_history called with arguments: "
        //         << "sample_set: " << sample_set << ", "
        //         << "currentVal: " << currentVal << ", "
        //         << " lru: " << it -> first <<": " << (it->second).lru<<  std::endl;
                
        
        //     } else {
        //         std::cerr << "Unable to open the functioncall file for writing." << std::endl;
        //     }
            // cout << "Function update_cache_history called with arguments: "
            //     <<"sample_set: " << sample_set << ", "
            //     << "currentVal: " << currentVal << ", "
            //     << " lru: " << it -> first <<": " << (it->second).lru<< endl;
        if((it->second).lru < currentVal){
            (it->second).lru++;
        }
    }

}

// Called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{

   if (set == 1519){  
      if(SAMPLED_SET(set)){
    //    // Print the function arguments
    // cout << "Function UpdateReplacementState called with arguments:"
    //      << "  cpu: " << cpu << ", "
    //      << "  set: " << set << ", "
    //      << "  way: " << way << ", "
    //      << "  paddr: " << paddr << ", "
    //      << "  PC: " << PC << ", "
    //      << "  victim_addr: " << victim_addr << ", "
    //      << "  type: " << type << ", "  
    //      << "  hit: " << static_cast<int>(hit) << endl;    // type cast for correct printing




    // write the arguments
    if (file.is_open()) {
        file << "Function UpdateReplacementState called with arguments: "
             << "cpu: " << cpu << ", "
             << "set: " << set << ", "
             << "way: " << way << ", "
             << "paddr: " << paddr << ", "
             << "PC: " << PC << ", "
             << "victim_addr: " << victim_addr << ", "
             << "type: " << type << ", "  
             << "hit: " << static_cast<int>(hit) << std::endl;  
        // file.close();
    } else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }




// ////////////////////////////////////////////////////////////
    // cout << "  paddr: " << paddr << endl;
    paddr = (paddr >> 6) << 6;

    //Ignore all types that are writebacks
    if(type == WRITEBACK){
        return;
    }

    if(type == PREFETCH){
        if(!hit){
            prefetching[set][way] = true;
        }
    }
    else{
        prefetching[set][way] = false;
    }

    //Only if we are using sampling sets for OPTgen
    if(SAMPLED_SET(set)){
        uint64_t currentVal = set_timer[set] % OPTGEN_SIZE;
        uint64_t sample_tag = CRC(paddr >> 12) % 256;
         // std::cout << "paddr: " << paddr << endl;
        uint32_t sample_set = (paddr >> 6) % SAMPLER_SETS;
        // std::cout <<"set: " << set << " PC: "  << PC <<" sample_set: " << sample_set << " sample_tag:" << sample_tag << " sampled_set: " << SAMPLED_SET(set)<< " currentVal: " << currentVal <<endl;
        // std::cout << "sample_set: " << sample_set <<endl;
        // std::cout << "sample_tag: " << sample_tag <<endl;
        // std::cout << "set: " << set <<endl;
        // std::cout << "currentVal: " << currentVal <<endl;
         // std::cout << "SAMPLER_SETS: " << SAMPLER_SETS <<endl;
        // std::cout << "Type of (paddr >> 6): " << typeid(paddr >> 6).name() << std::endl;
        
        // std::cout << "(paddr >> 6): " << (paddr >> 6) << ", SAMPLER_SETS: " << SAMPLER_SETS << std::endl;
        // std::cout << "sample_set: " << sample_set << std::endl;
       

        //If line has been used before, ignoring prefetching (demand access operation)
        if((type != PREFETCH) && (cache_history_sampler[sample_set].find(sample_tag) != cache_history_sampler[sample_set].end())){
            unsigned int current_time = set_timer[set];
            if(current_time < cache_history_sampler[sample_set][sample_tag].previousVal){
                current_time += TIMER_SIZE;
            }
            uint64_t previousVal = cache_history_sampler[sample_set][sample_tag].previousVal % OPTGEN_SIZE;
            bool isWrap = (current_time - cache_history_sampler[sample_set][sample_tag].previousVal) > OPTGEN_SIZE;
            // std::cout << "set_timer[set]:" << set_timer[set] << ", current time " << currentVal << " previous time " << previousVal << "isWrap: " << isWrap << std::endl;
            //Train predictor positvely for last PC value that was prefetched
            // std::cout << "OPTGenvector:" << (optgen_occup_vector[set].is_cache(currentVal, previousVal)  ? " true" : " is a miss") <<endl;

            //Train predictor positvely for last PC value that was prefetched
           
            // std::cout << "isWrap: " << isWrap << " set:" << set << " ,set_timer[set]:" << set_timer[set] << ",current time: " << current_time <<  " previous time: " << cache_history_sampler[sample_set][sample_tag].previousVal<< ",currentVal: "  << currentVal << " previousVal " << previousVal <<  std::endl;
            if(!isWrap && optgen_occup_vector[set].is_cache(currentVal, previousVal)){
                if(cache_history_sampler[sample_set][sample_tag].prefetching){
                    predictor_prefetch->increase(cache_history_sampler[sample_set][sample_tag].PCval);
                }
                else{
                    predictor_demand->increase(cache_history_sampler[sample_set][sample_tag].PCval);
                    // std::cout << "Increase - cache_history_sampler[sample_set][sample_tag].PCval: " << cache_history_sampler[sample_set][sample_tag].PCval<< endl;
                }
            }
            //Train predictor negatively since OPT did not cache this line
            else{
                if(cache_history_sampler[sample_set][sample_tag].prefetching){
                    predictor_prefetch->decrease(cache_history_sampler[sample_set][sample_tag].PCval);
                }
                else{
                    predictor_demand->decrease(cache_history_sampler[sample_set][sample_tag].PCval);
                    // std::cout << "decrease - cache_history_sampler[sample_set][sample_tag].PCval: " << cache_history_sampler[sample_set][sample_tag].PCval<< endl;
                }
            }
            
            optgen_occup_vector[set].set_access(currentVal);

            /////
       
            // std::cout <<"Hit--->set: " << set << " sample_set: " << sample_set << " sample_tag:" << sample_tag << " sampled_set: " << SAMPLED_SET(set) << ", Hashed PC: " << (CRC(PC) % 2048) ;
            // std::cout <<" currentVal: " << currentVal << " previousVal: " << cache_history_sampler[sample_set][sample_tag].previousVal % OPTGEN_SIZE << " Cached PCVal:" << cache_history_sampler[sample_set][sample_tag].PCval << " lru: " << cache_history_sampler[sample_set][sample_tag].lru << endl;
          

            //Update cache history
            update_cache_history(sample_set, cache_history_sampler[sample_set][sample_tag].lru);
            // std::cout << "call update_cache_history in the first loop :" << "lru: " << cache_history_sampler[sample_set][sample_tag].lru<< endl;
            //Mark prefetching as false since demand access

            //Mark prefetching as false since demand access
            cache_history_sampler[sample_set][sample_tag].prefetching = false;
        }
        //If line has not been used before, mark as prefetch or demand
        else if(cache_history_sampler[sample_set].find(sample_tag) == cache_history_sampler[sample_set].end()){
            //If sampling, find victim from cache
            if(cache_history_sampler[sample_set].size() == SAMPLER_HIST){
                //Replace the element in the cache history
                uint64_t addr_val = 0;
                for(map<uint64_t, HISTORY>::iterator it = cache_history_sampler[sample_set].begin(); it != cache_history_sampler[sample_set].end(); it++){
                    if((it->second).lru == (SAMPLER_HIST-1)){
                        addr_val = it->first;
                        break;
                    }
                }
                cache_history_sampler[sample_set].erase(addr_val);
            }

            //Create new entry
            cache_history_sampler[sample_set][sample_tag].init();
            // std::cout << " Create new entry for tag:" <<sample_tag<< endl;
            //If preftech, mark it as a prefetching or if not, just set the demand access

            //   ///
     
            // std::cout <<"Miss---->set: " << set <<" sample_set: " << sample_set << " sample_tag:" << sample_tag << " sampled_set: " << SAMPLED_SET(set);
            // std::cout <<" currentVal: " << currentVal << " previousVal: " << cache_history_sampler[sample_set][sample_tag].previousVal << " Cached PCVal:" << cache_history_sampler[sample_set][sample_tag].PCval << " lru: " << cache_history_sampler[sample_set][sample_tag].lru << endl;
           
            if(type == PREFETCH){
                cache_history_sampler[sample_set][sample_tag].set_prefetch();
                optgen_occup_vector[set].set_prefetch(currentVal);
            }
            else{
                optgen_occup_vector[set].set_access(currentVal);
            }

            //Update cache history
            update_cache_history(sample_set, SAMPLER_HIST-1);
            // std::cout << "call update_cache_history in the second loop :" << endl;
        }
        //If line is neither of the two above options, then it is a prefetch line
        else{
            uint64_t previousVal = cache_history_sampler[sample_set][sample_tag].previousVal % OPTGEN_SIZE;
             // std::cout << "set_timer[set]: "<<set_timer[set]<< ", cache_history_sampler[sample_set][sample_tag].previousVal " << cache_history_sampler[sample_set][sample_tag].previousVal << ", NUM_CORE: " << NUM_CORE << std::endl;
            // std::cout << "condition: " << ((set_timer[set] - cache_history_sampler[sample_set][sample_tag].previousVal) )<<endl;
            if((set_timer[set]) - cache_history_sampler[sample_set][sample_tag].previousVal < 5*NUM_CORE){
                // std::cout << "set_timer[set]: "<<set_timer[set]<< ", current time " << currentVal << ", previous time " << previousVal << std::endl;
                if(optgen_occup_vector[set].is_cache(currentVal, previousVal)){
                    if(cache_history_sampler[sample_set][sample_tag].prefetching){
                        predictor_prefetch->increase(cache_history_sampler[sample_set][sample_tag].PCval);
                    }
                    else{
                        predictor_demand->increase(cache_history_sampler[sample_set][sample_tag].PCval);
                        // std::cout << "Increase - cache_history_sampler[sample_set][sample_tag].PCval: " << cache_history_sampler[sample_set][sample_tag].PCval<< endl;
                    }
                }
            }
           
            // std::cout <<"--->set: " << set << " sample_set: " << sample_set << " sample_tag:" << sample_tag << " sampled_set: " << SAMPLED_SET(set) << ", Hashed PC: " << (CRC(PC) % 2048) ;
            // std::cout <<" currentVal: " << currentVal << " previousVal: " << cache_history_sampler[sample_set][sample_tag].previousVal << " Cached PCVal:" << cache_history_sampler[sample_set][sample_tag].PCval << " lru: " << cache_history_sampler[sample_set][sample_tag].lru << endl;
          
           cache_history_sampler[sample_set][sample_tag].set_prefetch();
            optgen_occup_vector[set].set_prefetch(currentVal);
            //Update cache history
            update_cache_history(sample_set, cache_history_sampler[sample_set][sample_tag].lru);
            // std::cout << "call update_cache_history in the third loop :" << endl;

        }   
        //Update the sample with time and PC
        cache_history_sampler[sample_set][sample_tag].update(set_timer[set], (CRC(PC) % 2048));
        cache_history_sampler[sample_set][sample_tag].lru = 0;
        set_timer[set] = (set_timer[set] + 1) % TIMER_SIZE;
        // std::cout << "New set_timer[set]:" << set_timer[set] <<endl;
        // std::cout <<"set: " << set << " sample_set: " << sample_set << " sample_tag:" << sample_tag << " sampled_set: " << SAMPLED_SET(set) << ", Hashed PC: " << (CRC(PC) % 2048) ;;
        // std::cout <<" currentVal: " << currentVal << " previousVal: " << cache_history_sampler[sample_set][sample_tag].previousVal << " Cached PCVal:" << cache_history_sampler[sample_set][sample_tag].PCval << " lru: " << cache_history_sampler[sample_set][sample_tag].lru << endl;
    }

    //Retrieve Hawkeye's prediction for line
    bool prediction = predictor_demand->get_prediction((CRC(PC) % 2048));
//   cout <<"prediction: " << prediction << " for Hashed PC: " << (CRC(PC) % 2048) << ", set: " << set << endl;
  
    if(type == PREFETCH){
        prediction = predictor_prefetch->get_prediction((CRC(PC) % 2048));
    }
    
    sample_signature[set][way] = ((CRC(PC) % 2048));
    // cout <<"write in sample_signature:" << (CRC(PC) % 2048)<< "Way: " << way << endl;
    //Fix RRIP counters with correct RRPVs and age accordingly
    if(!prediction){
        rrip[set][way] = MAXRRIP;
    }
    else{
        rrip[set][way] = 0;
        if(!hit){
            //Verifying RRPV of lines has not saturated
            bool isMaxVal = false;
            for(uint32_t i = 0; i < LLC_WAYS; i++){
                if(rrip[set][i] == MAXRRIP-1){
                    isMaxVal = true;
                }
            }
            // cout <<"isMaxVal:" << isMaxVal ;
            //Aging cache-friendly lines that have not saturated
            for(uint32_t i = 0; i < LLC_WAYS; i++){
                if(!isMaxVal && rrip[set][i] < MAXRRIP-1){
                    rrip[set][i]++;
                    //   cout <<"Hi:" << endl;
                }
            }
        }
        rrip[set][way] = 0;
    }

    /// PRINT RRIP
//     cout  <<" RRIP after change:";
//  for(uint32_t i = 0; i < LLC_WAYS; i++){
//     cout << rrip[1519][i] << " ";
//  }
//     cout  << endl;
    
   }
   }
}

// Use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{
    int hits = 0;
    int access = 0;
    for(int i = 0; i < LLC_SETS; i++){
        hits += optgen_occup_vector[i].get_optgen_hits();
        access += optgen_occup_vector[i].access;
    }

    cout<< "OPTGen Hits: " << hits << endl;
    cout<< "OPTGen Access: " << access << endl;
    cout<< "OPTGEN Hit Rate: " << 100 * ( (double)hits/(double)access )<< endl;
}

// Use this function to print out your own stats at the end of simulation
void PrintStats()
{
    int hits = 0;
    int access = 0;
    for(int i = 0; i < LLC_SETS; i++){
        hits += optgen_occup_vector[i].get_optgen_hits();
        access += optgen_occup_vector[i].access;
    }

    cout<< "Final OPTGen Hits: " << hits << endl;
    cout<< "Final OPTGen Access: " << access << endl;
    cout<< "Final OPTGEN Hit Rate: " << 100 * ( (double)hits/(double)access )<< endl;

}


void printCacheHistorySampler() {
    for(int i = 0; i < SAMPLER_ENTRIES; i++){
        for (const auto& entry : cache_history_sampler[i]) {
            const HISTORY& history = entry.second;
            std::cout << "set: " << i
                    << ", Address: " << entry.first 
                    << ", PCval: " << history.PCval 
                    << ", PreviousVal: " << history.previousVal 
                    << ", LRU: " << history.lru 
                    << ", Prefetching: " << (history.prefetching ? "Yes" : "No") 
                    << std::endl;
        }
    }
}