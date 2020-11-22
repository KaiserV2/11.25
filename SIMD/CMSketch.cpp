#include "CMSketch.h"

#include <immintrin.h> // AVX
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* init cmsketch with parameters */
void CMSketch::init(int _window, int _d, int _memory)
{
    d = _d;
    memory = _memory;
    window = _window;
    width = memory / (sizeof(uint16_t) + sizeof(uint8_t));
    w = width / d;
    width = w * d;

    updateLen = ((1 << (sizeof(uint8_t) * 4)) - 2) * width / window;
    max_clock_val = (1 << (sizeof(uint8_t) * 4)) - 1;
    lastUpdateIdx = 0;

    for(int i = 0; i < d; ++i)
        hash[i].initialize(rand() % MAX_PRIME32);

    memset(buckets, 0, sizeof(uint16_t) * MAX_CELL_NUM);
    memset(clocks, 0, sizeof(uint8_t) * MAX_CELL_NUM);
    printf("successfully initialize cmsketch: winSize=%d, memory=%d B, counterSize=%lu B, clockSize=%lu B, width=%d, d=%d, updateLen=%d\n",
    	window, memory, sizeof(uint16_t), sizeof(uint8_t), width, d, updateLen);
}

/* insert an item */
void CMSketch::insert(const char *key)
{
    for(int i = 0; i < d; ++i){
        int pos = hash[i].run(key, 4) % w + w * i;
        buckets[pos]++;
        clocks[pos] = max_clock_val;
    }
}

/* answer frequency */
int CMSketch::query(const char *key)
{
    int ret = 0x3FFFFFFF;
    for(int i = 0; i < d; ++i){
        int pos = hash[i].run(key, 4) % w + w * i;
        ret = std::min(ret, (int)buckets[pos]);
    }
    return ret;
}

/* update buckets */
void CMSketch::update(int insertTimesPerUpdate)
{
    int _updateLen = (updateLen * insertTimesPerUpdate / 32) * 32;
    int subAll = _updateLen / width;
    int len = _updateLen % width;

    int beg = lastUpdateIdx, end = std::min(width, lastUpdateIdx + len);
    update_range(beg, end, subAll + 1);
    if(end - beg < len)
    {
        end = len - (end - beg);
        beg = 0;
        update_range(beg, end, subAll + 1);
    }

    if(end > lastUpdateIdx){
        update_range(end, width, subAll);
        update_range(0, lastUpdateIdx, subAll);
    }
    else
        update_range(end, lastUpdateIdx, subAll);
    lastUpdateIdx = end;
}


/* update range using SIMD, end>=beg */
void CMSketch::update_range(int beg, int end, int val)
{
    if(val <= 0)    return;
    //for(int i = beg; i < end; ++i){
    //    if(clocks[i] < val){
    //        clocks[i] = 0;
    //        buckets[i] = 0;
    //    }
    //    else clocks[i] -= val;
    //}
    //return;

    /* address alignment */
	if(beg % 32 != 0){
		int endIdx = std::min(32 * (beg / 32 + 1), end);
		for(int i = beg; i < endIdx; ++i)
			if(clocks[i] < val){
                clocks[i] = 0;
                buckets[i] = 0;
            }
            else clocks[i] -= val;
		beg = endIdx;
	}

	__m256i _subVal = _mm256_set1_epi8(char(val));
    __m256i _subValMinusOne = _mm256_set1_epi8(char(val - 1));
	while(beg + 32 <= end){
		__m256i clock = _mm256_loadu_si256((__m256i*)&clocks[beg]);
		__m256i subRes = _mm256_subs_epu8(clock, _subVal);
		_mm256_storeu_si256((__m256i*)&clocks[beg], subRes);

        __m256i x = _mm256_max_epu8(clock, _subValMinusOne);
        __m256i eq = _mm256_cmpeq_epi8(x, _subValMinusOne);

        __m128i low128 = _mm256_extracti128_si256(eq, 0);
        __m128i high128 = _mm256_extracti128_si256(eq, 1);
        __m256i low = _mm256_cvtepi8_epi16(low128);
        __m256i high = _mm256_cvtepi8_epi16(high128);
        
        __m256i counter_low = _mm256_loadu_si256((__m256i*)&buckets[beg]);
        __m256i counter_high = _mm256_loadu_si256((__m256i*)&buckets[beg + 16]);

        __m256i resLow = _mm256_andnot_si256(low, counter_low);
        __m256i resHigh = _mm256_andnot_si256(high, counter_high);

        _mm256_storeu_si256((__m256i*)&buckets[beg], resLow);
        _mm256_storeu_si256((__m256i*)&buckets[beg + 16], resHigh);

		beg += 32;
	}

	/* deal with the left buckets */
	while(beg < end){
		if(clocks[beg] < val){
                clocks[beg] = 0;
                buckets[beg] = 0;
            }
            else clocks[beg] -= val;
		beg++;
	}
}


