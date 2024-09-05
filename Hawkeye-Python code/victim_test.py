import math
import numpy as np
from optgen import OPTgen
from helper_function import CRC, History
from hawkeye_algorithm import SAMPLER_SETS, InitReplacementState, GetVictimInSet, rrip

cpu = 0
set = 0  # Example set number
current_set = None  # You'll need to define this based on your implementation
PC = 0x12345  # Example PC
paddr = 0xABCDEF  # Example physical address
access_type = 0  # Define based on your access type constants

InitReplacementState()
rrip[0][0] = 3
rrip[0][1] = 6
rrip[0][2] = 0
rrip[0][3] = 5
victim = GetVictimInSet(cpu, set, current_set, PC, paddr, access_type)
print(f"Victim way in set {set}: {victim}")
