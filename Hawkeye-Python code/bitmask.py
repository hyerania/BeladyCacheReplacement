import math

def bitmask(l):
    return (1 << l) - 1 if l < 64 else 0xFFFFFFFFFFFFFFFF

def bits(x, i, l):
    return (x >> i) & bitmask(l)

def SAMPLED_SET(set_num):
    return bits(set_num, 0, 6) == bits(set_num, int(math.log2(4096)) - 6, 6)

# Test the bitmask and bits function
for l in range(1, 65):
    print(f"bitmask({l}): {bitmask(l)}")

test_val = 0xF0F0F0F0F0F0F0F0  # Sample value for testing
print(f"bits({test_val}, 4, 8): {bits(test_val, 4, 8)}")

# Test the SAMPLED_SET function
for set_num in range(100):
    print(f"Set {set_num} is {'sampled' if SAMPLED_SET(set_num) else 'not sampled'}")
