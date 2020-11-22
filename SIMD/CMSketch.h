#ifndef _CM_SKETCH_CLOCK_H_
#define _CM_SKETCH_CLOCK_H_

#include "BOBHash32.h"

class CMSketch
{
    static const int 
        MAX_CELL_NUM = 10 * 1e5 + 5,
        MAX_HASH_NUM = 50;
    int
        d,
        width,
        w,
        window,
        memory,
        updateLen,
        max_clock_val,
        lastUpdateIdx;
    BOBHash32 hash[MAX_HASH_NUM];
        uint16_t buckets[MAX_CELL_NUM];
        uint8_t clocks[MAX_CELL_NUM];
public:
    void init(int _win, int _d, int _memory);
    void insert(const char *key);
    int query(const char *key);
    void update(int insertTimesPerUpdate);
private:
    void update_range(int beg, int end, int val);
};



#endif //_CM_SKETCH_CLOCK_H_