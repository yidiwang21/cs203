#define _GNU_SOURCE

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <stdlib.h>
#include "unistd.h"

typedef unsigned char byte;

#define PREFETCH_ENABLED

// #define REPEAT_TIME     1000000
#define SECOND_TO_NS    1000000000

int main(int argc, char const *argv[])
{
    cpu_set_t set;
    CPU_ZERO(&set);        // clear cpu mask
    CPU_SET(16, &set);      // set cpu 0
    sched_setaffinity(16, sizeof(cpu_set_t), &set);  // 0 is the calling process

    if (argc != 4) {
        fprintf(stderr, "# Usage: ./cache_latency [L1 cache] [L2 cache] [L3 cache]");
    }
    const long L1_CACHE_SIZE = 1024 * atoi(argv[1]);
    const long L2_CACHE_SIZE = 1024 * atoi(argv[2]);
    const long L3_CACHE_SIZE = 1024 * atoi(argv[3]);

    const long TRASH_ARRAY_SIZE = L1_CACHE_SIZE + L2_CACHE_SIZE + L3_CACHE_SIZE;

    byte *newArray = (byte *)malloc(L1_CACHE_SIZE * sizeof(byte));    
    byte *evictArrayL1 = (byte *)malloc(L1_CACHE_SIZE * sizeof(byte));
    byte *evictArrayL2 = (byte *)malloc(L2_CACHE_SIZE * sizeof(byte));
    byte *evictArrayL3 = (byte *)malloc(L3_CACHE_SIZE * sizeof(byte));
    byte *trashArray = (byte *)malloc(TRASH_ARRAY_SIZE * sizeof(byte));

    long idx = 0;
    double latencyMainMem = 0;
    double latencyL1 = 0;
    double latencyL2 = 0;
    double latencyL3 = 0;

    struct timespec start_time, end_time;


    memset(newArray, 0xff, L1_CACHE_SIZE * sizeof(byte));
    memset(evictArrayL1, 0xff, L1_CACHE_SIZE * sizeof(byte));
    memset(evictArrayL2, 0xff, L2_CACHE_SIZE * sizeof(byte));
    memset(evictArrayL3, 0xff, L3_CACHE_SIZE * sizeof(byte));
    memset(trashArray, 0xff, TRASH_ARRAY_SIZE * sizeof(byte)); // let it "clear" L1

    // TODO: 
    // 1. access main memory with newArray, then L1 should be full
    // 2. access newArray, measuring L1 latency
    // 3. evict content in L1, then values in newArray should now be in L2
    // 4. access newArray, measuring L2 latency
    // 5. evict content in L2, then values in newArray should now be in L3
    // 6. access newArray, measuring L3 latency

    for (long i = 0; i < TRASH_ARRAY_SIZE; i++) {
        byte clr = trashArray[i];
        clr++;
    }

    // access main memory
    clock_gettime(CLOCK_REALTIME, &start_time);
    for (long i = 0; i < L1_CACHE_SIZE; i++) {
        byte temp = newArray[idx];
        // just to avoid prefetching the adjacent lines
        #ifdef PREFETCH_ENABLED
        idx++;
        #else
        idx = (idx + i) & 1023;
        #endif
    }
    clock_gettime(CLOCK_REALTIME, &end_time);
    // latencyMainMem = ((end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec)) / L1_CACHE_SIZE;
    latencyMainMem = (end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec);
    latencyMainMem /= L1_CACHE_SIZE;
    printf("Main memory latency: %f\n", latencyMainMem);

    // access L1 cache
    idx = 0;
    clock_gettime(CLOCK_REALTIME, &start_time);
    for (long i = 0; i < L1_CACHE_SIZE; i++) {
        byte temp = newArray[idx];
        // just to avoid prefetching the adjacent lines
        #ifdef PREFETCH_ENABLED
        idx++;
        #else
        idx = (idx + i) & 1023;
        #endif
    }
    clock_gettime(CLOCK_REALTIME, &end_time);
    latencyL1 = (end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec);
    latencyL1 /= L1_CACHE_SIZE;
    printf("L1 cache latency: %f\n", latencyL1);

    // evict values in L1, then the evicted values will go to L2
    for (long i = 0; i < L1_CACHE_SIZE; i++) {
        byte evicted_val = evictArrayL1[i];
    }

    // access L2 cache
    idx = 0;
    clock_gettime(CLOCK_REALTIME, &start_time);
    for (long i = 0; i < L1_CACHE_SIZE; i++) {
        byte temp = newArray[idx];
        // just to avoid prefetching the adjacent lines
        #ifdef PREFETCH_ENABLED
        idx++;
        #else
        idx = (idx + i) & 1023;
        #endif
    }
    clock_gettime(CLOCK_REALTIME, &end_time);
    latencyL2 = (end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec);
    latencyL2 /= L1_CACHE_SIZE;
    printf("L2 cache latency: %f\n", latencyL2);

    // evict values in L2, then the evicted values will go to L3
    for (long i = 0; i < L2_CACHE_SIZE; i++) {
        byte evicted_val = evictArrayL2[i];
    }

    // access L3 cache
    idx = 0;
    clock_gettime(CLOCK_REALTIME, &start_time);
    for (long i = 0; i < L1_CACHE_SIZE; i++) {
        byte temp = newArray[idx];
        // just to avoid prefetching the adjacent lines
        #ifdef PREFETCH_ENABLED
        idx++;
        #else
        idx = (idx + i) & 1023;
        #endif
    }
    clock_gettime(CLOCK_REALTIME, &end_time);
    latencyL3 = (end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec);
    latencyL3 /= L1_CACHE_SIZE;
    printf("L3 cache latency: %f\n", latencyL3);

    return 0;
}
