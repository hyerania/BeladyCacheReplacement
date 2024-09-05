from helper_function import CRC, History


class Hawkeye_Predictor:
    MAX_PCMAP = 31
    PCMAP_SIZE = 2048

    def __init__(self):
        self.PC_Map = {}

    def crc_hash(self, PC):
        return CRC(PC) % self.PCMAP_SIZE

    def get_prediction(self, PC):
        result = self.crc_hash(PC)
        return self.PC_Map.get(result, (self.MAX_PCMAP + 1) // 2) >= (self.MAX_PCMAP + 1) // 2

    # def increase(self, PC):
    #     result = self.crc_hash(PC)
    #     self.PC_Map[result] = min(self.PC_Map.get(result, (self.MAX_PCMAP + 1) // 2) + 1, self.MAX_PCMAP)
    #     print(f"Increase: PC {PC} (hashed to {result}), Counter: {self.PC_Map[result]}")

    def increase(self, PC):
        result = self.crc_hash(PC)
        if result not in self.PC_Map:
            self.PC_Map[result] = (self.MAX_PCMAP + 1) // 2
        
        if self.PC_Map[result] < self.MAX_PCMAP:
            self.PC_Map[result] += 1
        else:
            # If it's already at MAX_PCMAP, it remains unchanged
            self.PC_Map[result] = self.MAX_PCMAP

        # print(f"Increase: PC {PC} (hashed to {result}), Counter: {self.PC_Map[result]}")


    def decrease(self, PC):
        result = self.crc_hash(PC)
        if result not in self.PC_Map:
            self.PC_Map[result] = (self.MAX_PCMAP + 1) // 2
        if self.PC_Map[result] != 0:
            self.PC_Map[result] = self.PC_Map[result] - 1   
        
        # print(f"Decrease: PC {PC} (hashed to {result}), Counter: {self.PC_Map[result]}")

    def print_PC_Map(self):
        # print("Current State of PC_Map:")
        for pc, counter in self.PC_Map.items():
            print(f"  PC {pc}: Counter {counter}")

# # Usage
# predictor = Hawkeye_Predictor()
# # PC_example = 12345
# pcs = [00000, 67890, 12345, 11111, 67890]
# num_pcs = len(pcs)
# for i in range(num_pcs):
#     predictor.increase(pcs[i])
#     prediction = predictor.get_prediction(pcs[i])
#     # print("After increase, prediction for PC", pcs[i], ":", prediction)

# for i in range(num_pcs): 
#     predictor.decrease(pcs[i])
# # predictor.print_PC_Map()
# # predictor.increase(PC_example)
#     prediction = predictor.get_prediction(pcs[i])
#     # print("After decrease, prediction for PC", pcs[i], ":", prediction)

# # for i in range(num_pcs):
#     # print("CRC of block address", pcs[i], ":", CRC(pcs[i]))
# predictor.print_PC_Map()