import math
import random
import numpy as np
from helper_function import CRC, History
from optgen import OPTgen, OPTGEN_SIZE
from predictor import Hawkeye_Predictor
from hawkeye_algorithm import UpdateReplacementState,InitReplacementState,GetVictimInSet,LLC_SETS, optgen_occup_vector,LLC_SETS, optgen_occup_vector, print_stats_heartbeat,print_stats

with open('VictimWays.txt', "w") as file:
    pass

####################################################################################################
## _____      ___       __   _____
##   |       |___      (__     |
##   |       |___       __)    |
###################################################################################################


#Test the algorithm with num_accesses that will be generate in the file

# np.random.seed(42)
# # random.seed(seed_value)
# num_accesses = 500
# cache_accesses = []

# for _ in range(num_accesses):
#     set_num = random.randint(0, LLC_SETS -1 )  # LLC_SETS = 2048
#     way_num = random.randint(0, LLC_WAYS -1)  # LLC_WAYS = 16
#     access_type = random.choice(["PREFETCH", "DEMAND"])
#     is_hit = random.choice([True, False])
#     paddr = random.randint(0x1000, 0xFFFF)
#     PC = random.randint(0x1000, 0xFFFF)
#     cache_accesses.append(f"{set_num}, {way_num}, {access_type}, {is_hit}, 0x{paddr:X}, 0x{PC:X}")

# # # Join the cache accesses into a single string
# cache_access_sequence = "\n".join(cache_accesses)


# Read cache accesses from the file
with open('/mnt/e/F3CAS/Hawkeye/BeladyCacheReplacement/cache_accesses.txt', "r") as file:
    cache_access_lines = file.readlines()

# write cache accesses in the file
# with open('cache_accesses.txt', "w") as file:
#     file.write(cache_access_sequence)



# def simulate_cache_access(set_number, way_number, access_type, is_hit, paddr, PC):
#     """
#     Simulates a cache access and updates the replacement state accordingly.

#     :param set_number: The cache set number.
#     :param way_number: The cache way number.
#     :param access_type: Type of access, either "PREFETCH" or "DEMAND".
#     :param is_hit: Boolean indicating if the access is a hit (True) or miss (False).
#     :param paddr: The physical address being accessed.
#     :param PC: The program counter value for the access.
#     """
#     victim_addr = 0  # Assuming 0, "as default victim address"
#     UpdateReplacementState(0, set_number, way_number, paddr, PC, victim_addr, access_type, int(is_hit))

def print_cache_state(set_number):
    """
    Prints the state of the cache for a specific set.

    :param set_number: The cache set number to print the state for.
    """
    # print(f"Set {set_number} State:")
    # print("RRIP:", rrip[set_number])
    # print("Prefetching:", prefetching[set_number])
    # print("Sample Signature:", sample_signature[set_number])
    # print("Set Timer:", set_timer[set_number])
    # print()


# cache_access_sequence = [
#     (0, 0, "PREFETCH", False, 0x1000, 0x2000),
#     (0, 0, "PREFETCH", True, 0x1000, 0x2000),
#     (0, 0, "DEMAND", False, 0x1000, 0x2000),
#     (0, 0, "PREFETCH", False, 0x1000, 0x2000),
#     (0, 0, "PREFETCH", False, 0x1000, 0x4000),
#     (0, 0, "DEMAND", False, 0x1000, 0x4000),
#     (0, 0, "PREFETCH", False, 0x1000, 0x2000),
#     (0, 0, "PREFETCH", True, 0x1000, 0x2000),
#     (0, 0, "DEMAND", False, 0x1000, 0x2000),
#     (0, 0, "PREFETCH", False, 0x1000, 0x2000),
#     (0, 0, "PREFETCH", False, 0x1000, 0x4000),
#     (0, 0, "PREFETCH", False, 0x1000, 0x4000),
#     (0, 0, "DEMAND", True, 0x1000, 0x4000),
#     (0, 0, "DEMAND", False, 0x1000, 0x4000),
#     (0, 0, "DEMAND", True, 0x1000, 0x4000),
#     (0, 0, "DEMAND", False, 0x1000, 0x4000),
#     # ... add more accesses as needed
# ]

# Initialize the replacement state
InitReplacementState()

# # Simulate the defined sequence of cache accesses - without file
# for access in cache_access_sequence:
#     set_number, way_number, access_type, is_hit, paddr, PC = access
#     print(f"Simulating Cache Access: Set={set_number}, Way={way_number}, Type={access_type}, Hit={is_hit}, Addr={hex(paddr)}, PC={hex(PC)}")
#     simulate_cache_access(set_number, way_number, access_type, is_hit, paddr, PC)
#     victim_way = GetVictimInSet(0, set_number, 0, PC, paddr, access_type)
#     print("victim_way:", victim_way)
#     print_cache_state(set_number)


# Simulate the defined sequence of cache accesses
for access_line in cache_access_lines:
    access_line = access_line.strip()  # Remove newline character
    if not access_line:
        continue  # Skip empty lines

    # Split the line into components
    access_components = access_line.split(", ")
    # if len(access_components) != 6:
    #     print(f"Skipping invalid line: {access_line}")
    #     continue

    # Unpack the components
    set_number, way_number, access_type, is_hit, paddr_str, PC_str = access_components
    set_number = np.uint32(set_number)
    way_number = np.uint32(way_number)
    access_type = np.uint32(access_type)
    is_hit = is_hit == "True"
    paddr = np.uint32(paddr_str)  # Convert hexadecimal string to integer
    PC = np.uint32(PC_str)          # Convert hexadecimal string to integer

    # print(f"Simulating Cache Access: Set={set_number}, Way={way_number}, Type={access_type}, Hit={is_hit}, Addr={hex(paddr)}, PC={hex(PC)}")
   
    victim_way = GetVictimInSet(0, set_number, 0, PC, paddr, access_type)
    UpdateReplacementState(0, set_number, victim_way, paddr, PC, 0, access_type, is_hit)
    # simulate_cache_access(set_number, victim_way, access_type, is_hit, paddr, PC)
    
    # print("Victim way:", victim_way)
    sequential_mapping = []
    mapping_str = f"0x{paddr} -> VictimWay: {victim_way}, set: {set_number}"
    sequential_mapping.append(mapping_str)

    with open('VictimWays.txt', "a") as file:
        for mapping in sequential_mapping:
            file.write(mapping + "\n")
    print_cache_state(set_number)


# # Simulate the defined sequence of cache accesses
# cache_access_lines = cache_access_sequence.split("\n")
# for access_line in cache_access_lines:
#     # Split the line into components
#     access_components = access_line.split(", ")

#     # Unpack the components
#     set_number, way_number, access_type, is_hit, paddr_str, PC_str = access_components
#     set_number = int(set_number)
#     way_number = int(way_number)
#     is_hit = is_hit == "True"
#     paddr = int(paddr_str, 16)  # Convert hexadecimal string to integer
#     PC = int(PC_str, 16)        # Convert hexadecimal string to integer

#     print(f"Simulating Cache Access: Set={set_number}, Way={way_number}, Type={access_type}, Hit={is_hit}, Addr={hex(paddr)}, PC={hex(PC)}")
#     simulate_cache_access(set_number, way_number, access_type, is_hit, paddr, PC)
#     victim_way = GetVictimInSet(0, set_number, 0, PC, paddr, access_type)
#     print("Victim way:", victim_way)
#     print_cache_state(set_number)

# Print final statistics
print_stats_heartbeat(LLC_SETS, optgen_occup_vector)
print_stats(LLC_SETS, optgen_occup_vector)
