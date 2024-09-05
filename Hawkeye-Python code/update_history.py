from helper_function import CRC, History
from hawkeye_algorithm import SAMPLER_SETS, cache_history_sampler,InitReplacementState

# Function to update the cache history based on the current value
def update_cache_history(sample_set, currentVal):
    for address, history in cache_history_sampler[sample_set].items():
        if history.lru < currentVal:
            history.lru += 1


def print_cache_history_sampler(cache_history_sampler):
    for set_index, set_entries in enumerate(cache_history_sampler):
        # print(f"Set {set_index}:")
        for address, history in set_entries.items():
            print(f"  Address: {address}, PCval: {history.PCval}, PreviousVal: {history.previousVal}, "
                  f"LRU: {history.lru}, Prefetching: {'Yes' if history.prefetching else 'No'}")

# Example test code
def test_update_cache_history():
    
    InitReplacementState()
    
    sample_set = 2  # Example sample set number

    # Populate with some example data
    for i in range(5):
        cache_history_sampler[sample_set][i] = History()
        cache_history_sampler[sample_set][i].lru = i

    print("Before updating cache history:")
    print_cache_history_sampler(cache_history_sampler)
 

    # Update the cache history with a current value
    currentVal = 3
    update_cache_history(sample_set, currentVal)

    print("\nAfter updating cache history:")
    print_cache_history_sampler(cache_history_sampler)

# Call the test function
test_update_cache_history()
