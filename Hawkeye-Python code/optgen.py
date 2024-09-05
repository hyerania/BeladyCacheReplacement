OPTGEN_SIZE=128
class OPTgen:
    def __init__(self):
        self.liveness_intervals = [0] * OPTGEN_SIZE
        self.num_cache = 0
        self.access = 0
        self.cache_size = 0

    def init(self, size):
        self.num_cache = 0
        self.access = 0
        self.cache_size = size
        self.liveness_intervals = [0] *OPTGEN_SIZE

    def get_optgen_hits(self):
        return self.num_cache

    def set_access(self, val):
        int_val = int(val)
        self.access += 1
        self.liveness_intervals[int_val % OPTGEN_SIZE] = 0

    def set_prefetch(self, val):
        int_val = int(val)
        self.liveness_intervals[int_val % OPTGEN_SIZE] = 0

    def is_cache(self, val, end_val):
        int_val = int(val)
        int_end_val = int(end_val)
        cache_hit = True
        # int_val = int_val % OPTGEN_SIZE
        # int_end_val = int_end_val % OPTGEN_SIZE

        count = int_end_val
        while count != int_val:
            if self.liveness_intervals[count] >= self.cache_size:
                cache_hit = False
                break
            count = (count + 1) % OPTGEN_SIZE

        if cache_hit:
            count = int_end_val
            while count != int_val:
                self.liveness_intervals[count] += 1
                count = (count + 1) % OPTGEN_SIZE
            self.num_cache += 1
        # else:
        #     # Only reset the current interval to 0 on a miss
        #     self.liveness_intervals[int_val] = 0

        return cache_hit


def test_OPT_algorithm():
    cache_size = 2
    optgen = OPTgen()
    optgen.init(cache_size)

    accesses = ['A', 'B', 'C', 'D', 'A', 'E', 'B', 'C', 'D', 'A', 'E', 'B', 'F', 'D','F','A','A', 'C', 'D', 'A', 'E', 'B', 'B', 'C', 'D', 'A', 'E', 'B', 'F', 'D','F','A','C','C', 'D', 'A', 'E', 'B','A', 'B', 'C', 'D', 'A', 'E', 'B', 'C', 'D', 'A', 'E', 'B', 'F', 'D','F','A','A', 'C', 'D', 'A', 'E', 'B', 'B', 'C', 'D', 'A', 'E', 'B', 'F', 'D','F','A','C','C', 'D', 'A', 'E', 'B']
    last_access_map = {}
    hits, misses = 0, 0

    for i, address in enumerate(accesses):
        if address in last_access_map:
            previous_val = last_access_map[address]
            is_hit = optgen.is_cache(i, previous_val)
            
            print(f"Access to address {address} at time {i} {'is a hit' if is_hit else 'is a miss'}")
            if not is_hit:
                # optgen.set_access(i)
                misses += 1
            if is_hit:
                hits += 1
                
        else:
            # First access 
           
            print(f"Access to address {address} at time {i} {' --->( first access)'}")
            optgen.set_access(i)

        last_access_map[address] = i
      


    print(f"Total hits: {hits}")
    print(f"Total misses: {misses}")

if __name__ == "__main__":
    test_OPT_algorithm()
