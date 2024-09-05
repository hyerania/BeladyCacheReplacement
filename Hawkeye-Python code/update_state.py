import math
import numpy as np
from optgen import OPTgen , OPTGEN_SIZE
from helper_function import CRC, History
from predictor import Hawkeye_Predictor
from hawkeye_algorithm import SAMPLER_SETS, TIMER_SIZE, update_cache_history, SAMPLER_HIST, NUM_CORE, SAMPLED_SET,MAXRRIP,LLC_WAYS
def UpdateReplacementState(cpu, set, way, paddr, PC, victim_addr, type, hit):
    global rrip, prefetching, sample_signature, set_timer, optgen_occup_vector, cache_history_sampler, predictor_demand, predictor_prefetch

    # Address alignment
    paddr = (paddr >> 6) << 6

    # Ignore writebacks
    if type == "WRITEBACK":
        return

    # Update prefetching status
    if type == "PREFETCH":
        prefetching[set][way] = not hit
    else:
        prefetching[set][way] = False

    
    #Only if we are using sampling sets for OPTgen  /// # Processing for sampling sets for OPTgen
    if SAMPLED_SET(set):
        currentVal = int(set_timer[set] % OPTGEN_SIZE)
        sample_tag = CRC(paddr >> 12) % 256
        sample_set = (paddr >> 6) % SAMPLER_SETS
##########################################################################################      
        ##If line has been used before, ignoring prefetching (demand access operation)
##########################################################################################    
        if type != "PREFETCH" and sample_tag in cache_history_sampler[sample_set]:
            current_time = set_timer[set]
            if(current_time < cache_history_sampler[sample_set][sample_tag].previousVal):
                current_time += TIMER_SIZE
            previousVal = int(cache_history_sampler[sample_set][sample_tag].previousVal % OPTGEN_SIZE)
            isWrap = (current_time - cache_history_sampler[sample_set][sample_tag].previousVal) > OPTGEN_SIZE
            
            # Train predictor positively for the last PC value that was prefetched
            if not isWrap and optgen_occup_vector[set].is_cache(currentVal, previousVal):
                
                if cache_history_sampler[sample_set][sample_tag].prefetching:
                    predictor_prefetch.increase(cache_history_sampler[sample_set][sample_tag].PCval)
                else:
                    predictor_demand.increase(cache_history_sampler[sample_set][sample_tag].PCval)

                
                # print(f"Wrap-around condition: {isWrap}, met for Set: {set}, currentVal: {currentVal}, previousVal: {previousVal}, Timer value: {set_timer[set]}")
                # print(f"!Wrap Cache HIT at Set: {set}, Address: {paddr}")

            # Train predictor negatively since OPT did not cache this line
            else:
                if cache_history_sampler[sample_set][sample_tag].prefetching:
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
        elif sample_tag not in cache_history_sampler[sample_set]:
            # If sampling, find victim from cache
            if len(cache_history_sampler[sample_set]) == SAMPLER_HIST:
                # Replace the element in the cache history
                addr_val = None
                for addr, history in cache_history_sampler[sample_set].items():
                    if history.lru == (SAMPLER_HIST-1):
                        addr_val = addr
                        break   
           
                del cache_history_sampler[sample_set][addr_val]
            
            
            # Create new entry
            cache_history_sampler[sample_set][sample_tag] = History()
            # If prefetch, mark it as a prefetching or if not, just set the demand access
            if type == "PREFETCH":
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
            previousVal = cache_history_sampler[sample_set][sample_tag].previousVal % OPTGEN_SIZE
            if (set_timer[set] - cache_history_sampler[sample_set][sample_tag].previousVal < 5 * NUM_CORE):
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


    #Retrieve Hawkeye's prediction for line
    
    prediction = predictor_demand.get_prediction(PC) if type != "PREFETCH" else predictor_prefetch.get_prediction(PC)
    
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
    hits = 0
    access = 0

    for i in range(LLC_SETS):
        hits += optgen_occup_vector[i].get_optgen_hits()
        access += optgen_occup_vector[i].access

    hit_rate = 100 * (hits / access) if access != 0 else 0
    print(f"Final OPTGen Hits: {hits}")
    print(f"Final OPTGen Access: {access}")
    print(f"Final OPTGEN Hit Rate: {hit_rate}%")







# Initialize global variables
rrip = np.full((2048, 16), MAXRRIP, dtype=np.uint32)
prefetching = np.zeros((2048, 16), dtype=bool)
sample_signature = np.zeros((2048, 16), dtype=np.uint64)
set_timer = np.zeros(2048, dtype=np.uint64)
optgen_occup_vector = [OPTgen() for _ in range(2048)]
cache_history_sampler = [{} for _ in range(SAMPLER_SETS)]
predictor_demand = Hawkeye_Predictor()
predictor_prefetch = Hawkeye_Predictor()

def print_state(set):
    print("RRIP State for set", set, ":", rrip[set])
    print("Prefetching State for set", set, ":", prefetching[set])
    print("Sample Signature for set", set, ":", sample_signature[set])
    print("Set Timer for set", set, ":", set_timer[set])
    print("Cache History Sampler for set", set, ":", cache_history_sampler[set // (2048 // SAMPLER_SETS)])
    print()

# Simulate cache accesses
print("Initial State:")
print_state(0)

# Simulate a prefetch miss
UpdateReplacementState(0, 0, 0, 0xABCDEF, 0x12345, 0, "PREFETCH", True)
print("After Prefetch Miss:")
# Check prediction for prefetch
prefetch_prediction = predictor_prefetch.get_prediction(0x12345)
print("Prefetch Prediction:", prefetch_prediction)
print_state(0)

UpdateReplacementState(0, 0, 0, 0xABCDEF, 0x12345, 0, "PREFETCH", False)
print("After Prefetch Miss:")
# Check prediction for prefetch
prefetch_prediction = predictor_prefetch.get_prediction(0x12345)
print("Prefetch Prediction:", prefetch_prediction)
print_state(0)

UpdateReplacementState(0, 0, 0, 0xABCDEF, 0x12345, 0, "PREFETCH", False)
print("After Prefetch Miss:")
# Check prediction for prefetch
prefetch_prediction = predictor_prefetch.get_prediction(0x12345)
print("Prefetch Prediction:", prefetch_prediction)
print_state(0)

# Simulate a demand access hit
UpdateReplacementState(0, 0, 0, 0xABCDEF, 0x12345, 0, "DEMAND", True)
print("After Demand Hit:")
# Check prediction for demand
demand_prediction = predictor_demand.get_prediction(0x12345)
print("Demand Prediction:", demand_prediction)
print_state(0)

# Simulate a demand access miss
UpdateReplacementState(0, 0, 1, 0xFEDCBA, 0x54321, 0, "DEMAND", False)
print("After Demand Miss:")
# Check prediction for demand
demand_prediction = predictor_demand.get_prediction(0x54321)
print("Demand Prediction:", demand_prediction)
print_state(0)