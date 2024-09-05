#include <iostream>
#include "hawkeye_predictor.h"
#include "helper_function.h"

int main() {
    Hawkeye_Predictor predictor;
    // CRC crc;

    // Example PC values for testing
    uint64_t pcs[] = {00000, 67890, 12345, 11111, 67890};
    size_t num_pcs = sizeof(pcs) / sizeof(uint64_t);

    // Increase and decrease operations
    for (size_t i = 0; i < num_pcs; ++i) {
        predictor.increase(pcs[i]);

        // Test the prediction for each PC after increasing
        bool prediction = predictor.get_prediction(pcs[i]);
        std::cout << "After increase, prediction for PC " << pcs[i] << ": " << (prediction ? "true" : "false") << std::endl;
    }

    for (size_t i = 0; i < num_pcs; ++i) {
        predictor.decrease(pcs[i]);

        // Test the prediction for each PC after decreasing
        bool prediction = predictor.get_prediction(pcs[i]);
        std::cout << "After decrease, prediction for PC " << pcs[i] << ": " << (prediction ? "true" : "false") << std::endl;
    }

    for (size_t i=0; i<num_pcs; ++i){

        uint64_t crc = CRC(pcs[i]);
        std::cout << "CRC of block address" << pcs[i] << ":" << crc << std::endl;

    }

    return 0;
}
