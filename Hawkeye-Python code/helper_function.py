
def CRC(block_address):
    crcPolynomial = 3988292384
    return_val = block_address
    for i in range(32):
        if return_val & 1:
            return_val = (return_val >> 1) ^ crcPolynomial
        else:
            return_val >>= 1
    return return_val



class History:
    def __init__(self):
        self.address = 0 
        self.PCval = 0
        self.previousVal = 0
        self.lru = 0
        self.prefetching = False

    def init(self):
        self.PCval = 0
        self.previousVal = 0
        self.lru = 0
        self.prefetching = False

    def update(self, currentVal, PC):
        self.previousVal = currentVal
        self.PCval = PC

    def set_prefetch(self):
        self.prefetching = True
        
        
        
# def print_cache_history_sampler(cache_history_sampler):
#     for set_index, set_entries in enumerate(cache_history_sampler):
#         print(f"Set {set_index}:")
#         for address, history in set_entries.items():
#             print(f"  Address: {address}, PCval: {history.PCval}, PreviousVal: {history.previousVal}, "
#                   f"LRU: {history.lru}, Prefetching: {'Yes' if history.prefetching else 'No'}")


# Example usage of CRC
# print("CRC of block address:", CRC(12345))

# Example usage of History class
history = History()
history.init()
history.update(currentVal=123, PC=456)
history.set_prefetch()
# print(f"PCval: {history.PCval}, previousVal: {history.previousVal}, LRU: {history.lru}, Prefetching: {history.prefetching}")

# Example usage of print_cache_history_sampler
# cache_history_sampler = [{123: history, 456: history}]  # Example data
# print_cache_history_sampler(cache_history_sampler)

