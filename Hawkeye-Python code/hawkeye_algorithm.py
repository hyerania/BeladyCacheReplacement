import math
import random
import numpy as np
from helper_function import CRC, History
from optgen import OPTgen, OPTGEN_SIZE
from predictor import Hawkeye_Predictor

with open('VictimWaysfinal.txt', "w") as file:
    pass

WRITEBACK = 3
PREFETCH = 2
DEMAND = 0



NUM_CORE = 1
LLC_SETS = NUM_CORE * 2048
LLC_WAYS = 16

### 3-bits RRIP counter
MAXRRIP = 7
rrip = np.zeros((LLC_SETS, LLC_WAYS), dtype=np.uint32)


##Hawkeye predictors for demand and prefetch requests
predictor_demand = Hawkeye_Predictor()   ##2K entries, 5-bit counter per each entry
predictor_prefetch = Hawkeye_Predictor()   ##2K entries, 5-bit counter per each entry


optgen_occup_vector = [OPTgen() for _ in range(LLC_SETS)]  ##//64 vecotrs, 128 entries each


###Prefectching
prefetching = np.zeros((LLC_SETS, LLC_WAYS), dtype=bool)




### Sampler components tracking cache history
SAMPLER_ENTRIES = 2800
SAMPLER_HIST = 8
SAMPLER_SETS = (SAMPLER_ENTRIES // SAMPLER_HIST)
cache_history_sampler = [{} for _ in range(SAMPLER_SETS)]
sample_signature = np.zeros((LLC_SETS, LLC_WAYS), dtype=np.int64)

##History time
TIMER_SIZE = 1024
set_timer = np.zeros(LLC_SETS, dtype=np.int64) ##64 sets, where 1 timer is used for every set



##Mathmatical functions needed for sampling set
def bitmask(l):
    return (1 << l) - 1 if l < 64 else 0xFFFFFFFFFFFFFFFF
def bits(x, i, l):
    return (x >> i) & bitmask(l)
def SAMPLED_SET(set):    #Helper function to sample 64 sets for each core
    return bits(set, 0, 6) == bits(set, int(math.log2(LLC_SETS)) - 6, 6)


##Initialize replacement state
def InitReplacementState():
    global NUM_CORE, LLC_SETS, LLC_WAYS, MAXRRIP, rrip
    global predictor_demand, predictor_prefetch, optgen_occup_vector
    global prefetching, SAMPLER_ENTRIES, SAMPLER_HIST, SAMPLER_SETS
    global cache_history_sampler, sample_signature, TIMER_SIZE, set_timer
    print("Initialize Hawkeye replacement policy state")
    # global rrip, prefetching, sample_signature, set_timer, optgen_occup_vector, cache_history_sampler

    # Initialize rrip, sample_signature, and prefetching
    # print("rrip array:")
    for i in range(LLC_SETS):
        for j in range(LLC_WAYS):
            rrip[i][j] = MAXRRIP
            sample_signature[i][j] = 0
            prefetching[i][j] = False
            # print(rrip[i][j], end=" ")
        # print()

        set_timer[i] = 0
        optgen_occup_vector[i].init(LLC_WAYS - 2)
    
    cache_history_sampler = cache_history_sampler[:SAMPLER_SETS]
   
    # Clear cache_history_sampler
    for i in range(SAMPLER_SETS):
        cache_history_sampler[i].clear()

    print("Finished initializing Hawkeye replacement policy state")




## Find replacement victim
# # Return value should be 0 ~ 15 or 16 (bypass)
def GetVictimInSet(cpu, set, current_set, PC, paddr, type):
    global NUM_CORE, LLC_SETS, LLC_WAYS, MAXRRIP, rrip
    global predictor_demand, predictor_prefetch, optgen_occup_vector
    global prefetching, SAMPLER_ENTRIES, SAMPLER_HIST, SAMPLER_SETS
    global cache_history_sampler, sample_signature, TIMER_SIZE, set_timer
    # print(f"Function GetVictimInSet called with arguments: cpu: {cpu}, set: {set}, current_set: {current_set}, PC: {PC}, paddr: {paddr}, access_type: {type}" )
    # print("rrip before finding victim:")
    # print(rrip[set])
    # Find the line with RRPV of MAXRRIP (7) in the set
    for i in range(LLC_WAYS):
        if rrip[set][i] == MAXRRIP:
            with open("VictimWaysfinal.txt", "a") as file:
                file.write(f"0x{paddr} -> VictimWay: {i}, set: {set}\n")
            return i  # Found victim

    # If no RRPV of MAXRRIP, find the next highest RRPV value (oldest line)
    max_rrpv = 0
    victim_way = -1
    for i in range(LLC_WAYS):
        if rrip[set][i] >= max_rrpv:
            max_rrpv = rrip[set][i]
            victim_way = i


    
    # Train the predictor negatively on evictions
    if SAMPLED_SET(set):
        if prefetching[set][victim_way]:
            predictor_prefetch.decrease(sample_signature[set][victim_way])
        else:
            predictor_demand.decrease(sample_signature[set][victim_way])


    # print("rrip after finding victim:")
    # print(rrip[set])
    # for i in range(LLC_WAYS):
    #     print(rrip[set][i], end=" ")
    # print() 
    # print("RRIP of victim way before modification:", rrip[set][victim_way])
    
    with open("VictimWaysfinal.txt", "a") as file:
        file.write(f"0x{paddr} -> VictimWay: {victim_way}, set: {set}\n")
    return victim_way

# Function to update the cache history based on the current value
def update_cache_history(sample_set, currentVal):
    global cache_history_sampler
    sample_set = np.uint(sample_set)
    currentVal = np.uint(currentVal)
    for address, history in cache_history_sampler[sample_set].items():
        if history.lru < currentVal:
            history.lru += 1





def UpdateReplacementState(cpu, set, way, paddr, PC, victim_addr, type, hit):
    global NUM_CORE, LLC_SETS, LLC_WAYS, MAXRRIP, rrip
    global predictor_demand, predictor_prefetch, optgen_occup_vector
    global prefetching, SAMPLER_ENTRIES, SAMPLER_HIST, SAMPLER_SETS
    global cache_history_sampler, sample_signature, TIMER_SIZE, set_timer
    

    # print(f"Function UpdateReplacementState called with arguments: cpu: {cpu}, set: {set}, way: {way}, paddr: {paddr}, PC: {PC}, victim_addr: {victim_addr}, type: {type}, hit: {hit}")
    # Address alignment
    paddr = (paddr >> 6) << 6

    # Ignore writebacks
    if type == WRITEBACK:
        return

    # Update prefetching status
    if type == PREFETCH:
        prefetching[set][way] = not hit
    else:
        prefetching[set][way] = False

    
    #Only if we are using sampling sets for OPTgen  /// # Processing for sampling sets for OPTgen
    if SAMPLED_SET(set):
        currentVal = np.uint64(set_timer[set] % OPTGEN_SIZE)
        sample_tag = np.uint64(CRC(paddr >> 12) % 256)
        sample_set = np.uint32((paddr >> 6) % SAMPLER_SETS)
        # print(f"sample_set: {sample_set}")
        # print(f"set: {sample_set}")
        # print(f"SAMPLER_SETS: {SAMPLER_SETS}")
        print(f"(paddr >> 6): {paddr >> 6}")
        print(f"sample_set: {(paddr >> 6) % SAMPLER_SETS}")

##########################################################################################      
        ##If line has been used before, ignoring prefetching (demand access operation)
##########################################################################################    
        if ((type != PREFETCH) and (sample_tag in cache_history_sampler[sample_set])):
            current_time = np.uint64(set_timer[set])
            if(current_time < cache_history_sampler[sample_set][sample_tag].previousVal):
                current_time += TIMER_SIZE
            previousVal = np.uint64(cache_history_sampler[sample_set][sample_tag].previousVal % OPTGEN_SIZE)
            isWrap = (current_time - cache_history_sampler[sample_set][sample_tag].previousVal) > OPTGEN_SIZE
            
            # Train predictor positively for the last PC value that was prefetched
            # isHit = optgen_occup_vector[set].is_cache(currentVal, previousVal)
            # print(f"1set_timer[set]: {set_timer[set]}, current time: {currentVal},previous time: {previousVal}")
            if (not isWrap and optgen_occup_vector[set].is_cache(currentVal, previousVal)):
               
                if (cache_history_sampler[sample_set][sample_tag].prefetching):
                    predictor_prefetch.increase(cache_history_sampler[sample_set][sample_tag].PCval)
                else:
                    predictor_demand.increase(cache_history_sampler[sample_set][sample_tag].PCval)

                
                # print(f"Wrap-around condition: {isWrap}, met for Set: {set}, currentVal: {currentVal}, previousVal: {previousVal}, Timer value: {set_timer[set]}")
                # print(f"!Wrap Cache HIT at Set: {set}, Address: {paddr}")

            # Train predictor negatively since OPT did not cache this line
            else:
                if (cache_history_sampler[sample_set][sample_tag].prefetching):
                    predictor_prefetch.decrease(cache_history_sampler[sample_set][sample_tag].PCval)
                else:
                    predictor_demand.decrease(cache_history_sampler[sample_set][sample_tag].PCval)
                
                # print(f"Predictor update for PC: {PC}, Prefetch: {'true' if prefetching[set][way] else 'false'}")
                # print(f"!Wrap Cache MISS at Set: {set}, Address: {paddr}")
            
            
            optgen_occup_vector[set].set_access(currentVal)
            update_cache_history(sample_set, cache_history_sampler[sample_set][sample_tag].lru)

            # Mark prefetching as false since demand access
            cache_history_sampler[sample_set][sample_tag].prefetching = False
##########################################################################################                      
        ##If line has not been used before, mark as prefetch or demand     
##########################################################################################         
        elif (sample_tag not in cache_history_sampler[sample_set]):
            # If sampling, find victim from cache
            if len(cache_history_sampler[sample_set]) == SAMPLER_HIST:
                # Replace the element in the cache history
                addr_val = 0
                for addr, history in cache_history_sampler[sample_set].items():
                    if history.lru == (SAMPLER_HIST-1):
                        addr_val = addr
                        break   
           
                del cache_history_sampler[sample_set][addr_val]
            
            
            # Create new entry
            cache_history_sampler[sample_set][sample_tag] = History()
            cache_history_sampler[sample_set][sample_tag].init()
            # If prefetch, mark it as a prefetching or if not, just set the demand access
            if type == PREFETCH:
                cache_history_sampler[sample_set][sample_tag].set_prefetch()
                optgen_occup_vector[set].set_prefetch(currentVal)
            else:
                optgen_occup_vector[set].set_access(currentVal)

            # Update cache history
            update_cache_history(sample_set, SAMPLER_HIST - 1)
            
##########################################################################################  
        #If line is neither of the two above options, then it is a prefetch line
##########################################################################################           
        else:
            previousVal = np.uint64(cache_history_sampler[sample_set][sample_tag].previousVal % OPTGEN_SIZE)
            # print(f"set_timer[set]: {set_timer[set]}, cache_history_sampler[sample_set][sample_tag].previousVal : {cache_history_sampler[sample_set][sample_tag].previousVal },NUM_CORE: {NUM_CORE}")
            if (set_timer[set] - cache_history_sampler[sample_set][sample_tag].previousVal < 5 * NUM_CORE):
                # isHit = optgen_occup_vector[set].is_cache(currentVal, previousVal)
                # print(f"set_timer[set]: {set_timer[set]}, current time: {currentVal},previous time: {previousVal}")
                if (optgen_occup_vector[set].is_cache(currentVal, previousVal)):
                    
                    if (cache_history_sampler[sample_set][sample_tag].prefetching):
                        predictor_prefetch.increase(cache_history_sampler[sample_set][sample_tag].PCval)
                    else:
                        predictor_demand.increase(cache_history_sampler[sample_set][sample_tag].PCval)

            cache_history_sampler[sample_set][sample_tag].set_prefetch()
            optgen_occup_vector[set].set_prefetch(currentVal)
            #Update cache history
            update_cache_history(sample_set, cache_history_sampler[sample_set][sample_tag].lru)

        #Update the sample with time and PC
        cache_history_sampler[sample_set][sample_tag].update(set_timer[set], PC)
        cache_history_sampler[sample_set][sample_tag].lru = 0
        set_timer[set] = (set_timer[set] + 1) % TIMER_SIZE
        # print(f"New set_timer[set]: ",  set_timer[set])


    #Retrieve Hawkeye's prediction for line
    
    prediction = predictor_demand.get_prediction(PC) if type != PREFETCH else predictor_prefetch.get_prediction(PC)
    
    # Assign the program counter to the sample signature for this set and way
    sample_signature[set][way] = PC

    # Fix RRIP counters based on the prediction
    if not prediction:
        # If not predicted (cache averse), set the RRIP to the maximum value
        rrip[set][way] = MAXRRIP
    else:
        # If predicted (cache friendly), set the RRIP to 0
        rrip[set][way] = 0
        if not hit:
            # Check if any RRIP value has reached MAXRRIP - 1
            is_max_val = any(rrip[set][i] == MAXRRIP - 1 for i in range(LLC_WAYS))

            # Increment the RRIP of other cache lines if they haven't reached MAXRRIP - 1
            for i in range(LLC_WAYS):
                if not is_max_val and rrip[set][i] < MAXRRIP - 1:
                    rrip[set][i] += 1
        rrip[set][way] = 0
          
            
def print_stats_heartbeat(LLC_SETS, optgen_occup_vector):
    global NUM_CORE,LLC_WAYS, MAXRRIP, rrip
    global predictor_demand, predictor_prefetch 
    global prefetching, SAMPLER_ENTRIES, SAMPLER_HIST, SAMPLER_SETS
    global cache_history_sampler, sample_signature, TIMER_SIZE, set_timer
    hits = 0
    access = 0

    for i in range(LLC_SETS):
        hits += optgen_occup_vector[i].get_optgen_hits()
        access += optgen_occup_vector[i].access

    hit_rate = 100 * (hits / access) if access != 0 else 0
    print(f"OPTGen Hits: {hits}")
    print(f"OPTGen Access: {access}")
    print(f"OPTGEN Hit Rate: {hit_rate}%")

def print_stats(LLC_SETS, optgen_occup_vector):
    global NUM_CORE, LLC_WAYS, MAXRRIP, rrip
    global predictor_demand, predictor_prefetch
    global prefetching, SAMPLER_ENTRIES, SAMPLER_HIST, SAMPLER_SETS
    global cache_history_sampler, sample_signature, TIMER_SIZE, set_timer
    hits = 0
    access = 0

    for i in range(LLC_SETS):
        hits += optgen_occup_vector[i].get_optgen_hits()
        access += optgen_occup_vector[i].access

    hit_rate = 100 * (hits / access) if access != 0 else 0
    print(f"Final OPTGen Hits: {hits}")
    print(f"Final OPTGen Access: {access}")
    print(f"Final OPTGEN Hit Rate: {hit_rate}%")
