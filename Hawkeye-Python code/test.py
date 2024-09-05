import re
import math
import random
import numpy as np
from helper_function import CRC, History
from optgen import OPTgen, OPTGEN_SIZE
from predictor import Hawkeye_Predictor
from hawkeye_algorithm import UpdateReplacementState,InitReplacementState,GetVictimInSet,LLC_SETS, optgen_occup_vector,LLC_SETS, optgen_occup_vector, print_stats_heartbeat,print_stats


def parse_and_call_function(line, output_file):
    if 'Initialize Hawkeye replacement policy state' in line:
        InitReplacementState()
    elif 'Function GetVictimInSet called with arguments:' in line:
        
        args = re.findall(r'cpu: (\d+), set: (\d+), current_set: (0x[\da-fA-F]+), PC: (\d+), paddr: (\d+), type: (\d+)', line)
        # print(f"GetVictimInSet args found: {args}")  
        if args:
            # Convert arguments to appropriate types
            cpu, set_num, current_set, PC, paddr, type = args[0]
            cpu, set_num, PC, paddr, type = map(int, [cpu, set_num, PC, paddr, type])
            victim_way = GetVictimInSet(cpu, set_num, current_set, PC, paddr, type)
            output_file.write(f"0x{paddr} -> VictimWay: {victim_way}, set: {set_num}\n")
            
    elif 'Function UpdateReplacementState called with arguments:' in line:
        # Assuming UpdateReplacementState does not need current_set as hex
        args = re.findall(r'cpu: (\d+), set: (\d+), way: (\d+), paddr: (\d+), PC: (\d+), victim_addr: (\d+), type: (\d+), hit: (\d+)', line)
        if args:
            int_args = [int(arg) for arg in args[0]]
            UpdateReplacementState(*int_args)




victim_file = "VictimWays.txt"      
with open('/mnt/e/F3CAS/Hawkeye/BeladyCacheReplacement/functioncall.txt', 'r') as file, open (victim_file, 'w') as output_file:
    for line in file:
        parse_and_call_function(line, output_file)
        
        


        
print_stats_heartbeat(LLC_SETS, optgen_occup_vector)
print_stats(LLC_SETS, optgen_occup_vector)
