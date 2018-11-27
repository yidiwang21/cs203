#include <time.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char byte;

#define L1_CACHE_SIZE   32 * 1024 / sizeof(byte)
#define L2_CACHE_SIZE   256 * 1024 / sizeof(byte)
#define L3_CACHE_SIZE   9216 * 1024 / sizeof(byte)
#define REPEAT_TIME     1000000

int main(int argc, char const *argv[])
{
    

    byte newArray[L1_CACHE_SIZE];
    byte evictArrayL1[L1_CACHE_SIZE];
    byte evictArrayL2[L2_CACHE_SIZE];
    byte evictArrayL3[L3_CACHE_SIZE];

    long idx = 0;

    struct timespec start_time, end_time;

    memset(newArray, 0, L1_CACHE_SIZE * sizeof(byte));
    memset(evictArrayL1, 0, L1_CACHE_SIZE * sizeof(byte));
    memset(evictArrayL2, 0, L2_CACHE_SIZE * sizeof(byte));
    memset(evictArrayL3, 0, L3_CACHE_SIZE * sizeof(byte));

    // TODO: 
    // 1. access main memory with newArray, then L1 should be full
    // 2. access newArray, measuring L1 latency
    // 3. evict content in L1, then values in newArray should now be in L2
    // 4. access newArray, measuring L2 latency
    // 5. evict content in L2, then values in newArray should now be in L3
    // 6. access newArray, measuring L3 latency

    clock_gettime(CLOCK_REALTIME, &start_time);
    // access main memory
    for (long i = 0; i < L1_CACHE_SIZE; i++) {
        byte temp = newArray[idx];
        // to avoid prefetching 

    }

    clock_gettime(CLOCK_REALTIME, &end_time);
    



    return 0;
}
