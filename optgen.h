#ifndef OPTGEN_H
#define OPTGEN_H

using namespace std;

#include <vector>
#define OPTGEN_SIZE 128

struct OPTgen{
    vector<unsigned int> liveness_intervals;
    uint64_t num_cache;
    uint64_t access;
    uint64_t cache_size;

    //Initialize values
    void init(uint64_t size){
        num_cache = 0;
        access = 0;
        cache_size = size;
        liveness_intervals.resize(OPTGEN_SIZE, 0);
    }

    //Return number of hits
    uint64_t get_optgen_hits(){
        return num_cache;
    }

    void set_access(uint64_t val){
        access++;
        liveness_intervals[val] = 0;
    }

    void set_prefetch(uint64_t val){
        liveness_intervals[val] = 0;
    }

    //Return if hit or miss
    bool is_cache(uint64_t val, uint64_t endVal){
        bool cache = true;
        unsigned int count = endVal;
        while (count != val){
            if(liveness_intervals[count] >= cache_size){
                cache = false;
                cout <<"not cached: " << ", liveness_intervals [" << count << "]: " << liveness_intervals[count] <<endl;
                break;
                  
            }
            cout <<"count: " << count  << ", liveness_intervals [" << count << "]: " << liveness_intervals[count] <<endl;
            count = (count+1) % liveness_intervals.size();
           
        }

        if(cache){
            count = endVal;
            while(count != val){
                liveness_intervals[count]++;
                // cout << "updade count: " << count << ", liveness_intervals [" << count << "]: " << liveness_intervals[count] <<endl;
                count = (count+1) % liveness_intervals.size();
            }
            num_cache++;
        }
        cout<< "is Cache: " << cache << "endVal: " << endVal << " val: " << val <<endl;
        return cache;
    }
};

#endif