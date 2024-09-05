#ifndef HAWKEYE_PREDICTOR_H
#define HAWKEYE_PREDICTOR_H

using namespace std;
#include <vector>
#include <map>
#include "helper_function.h"

#define MAX_PCMAP 31
#define PCMAP_SIZE 2048

class Hawkeye_Predictor{
private:
	map<uint64_t, int> PC_Map;

public:
	//Return prediction for PC Address
	bool get_prediction(uint64_t PC){
		// uint64_t result = CRC(PC) % PCMAP_SIZE;
		uint64_t result = PC;
		if(PC_Map.find(result) != PC_Map.end() && PC_Map[result] < ((MAX_PCMAP+1)/2)){
			return false;
			// cout << "prediction for PC:" << PC << ": " << false << endl;
		}

		// cout << "prediction for PC:" << PC << ": " << true << endl;
		return true;
	}

	void increase(uint64_t PC){
		// uint64_t result = CRC(PC) % PCMAP_SIZE;
		uint64_t result = PC;
		if(PC_Map.find(result) == PC_Map.end()){
			PC_Map[result] = (MAX_PCMAP + 1)/2;
		}

		if(PC_Map[result] < MAX_PCMAP){
			PC_Map[result] = PC_Map[result]+1;
		}
		else{
			PC_Map[result] = MAX_PCMAP;
		}
		 // Print the updated counter
        std::cout << "Increase - PC: " << PC << ", Hashed to: " << result << ", Counter: " << PC_Map[result] << std::endl;
	}

	void decrease(uint64_t PC){
		// uint64_t result = CRC(PC) % PCMAP_SIZE;
		uint64_t result = PC;
		if(PC_Map.find(result) == PC_Map.end()){
			PC_Map[result] = (MAX_PCMAP + 1)/2;
		}
		if(PC_Map[result] != 0){
			PC_Map[result] = PC_Map[result] - 1;
		}

		// Print the updated counter
        std::cout << "Decrease - PC: " << PC << ", Hashed to: " << result << ", Counter: " << PC_Map[result] << std::endl;
	}

};

#endif