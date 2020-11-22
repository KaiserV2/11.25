#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "BOBHash32.h"

class Bitmap
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
    void init(int _window, int _memory, int _hashnum=1);
    void insert(int x);
    void update(int insertTimesPerUpdate);
    double query();
private:
    void update_range(int beg, int end, int val);
};

#endif //_BITMAP_H_