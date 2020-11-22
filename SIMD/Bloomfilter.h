#ifndef _BLOOM_FILTER_COUNT_BASED_
#define _BLOOM_FILTER_COUNT_BASED_

#include "BOBHash32.h"

class BloomFilter
{
    static const int
        MAX_CELL_NUM = 32 * 1e5 + 5,
        MAX_HASH_NUM = 50;
    int
        window,
        memory, 
        hash_num,
        width,
        updateLen,
        max_counter_val,
        lastUpdateIdx;
    BOBHash32
        hash[MAX_HASH_NUM],
        hash_time_offset;
    alignas(64) uint8_t buckets[MAX_CELL_NUM];
public:
    void init(int _window, int _memory, int _hash_num);
    void insert(int x);
    void update(int insertTimesPerUpdate);
    bool query(int x);
private:
    void update_range(int beg, int end, int val);
};




#endif //_BLOOM_FILTER_COUNT_BASED_