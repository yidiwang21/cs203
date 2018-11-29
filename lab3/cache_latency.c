#define _GNU_SOURCE

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <stdlib.h>
#include "unistd.h"
#include "rdtsc.h"

typedef unsigned char byte;

// #define PREFETCH_ENABLED

// #define REPEAT_TIME     1000000
#define SECOND_TO_NS    1000000000

int main(int argc, char const *argv[])
{
    cpu_set_t set;
    CPU_ZERO(&set);        // clear cpu mask
    CPU_SET(0, &set);      // set cpu 0
    sched_setaffinity(0, sizeof(cpu_set_t), &set);  // 0 is the calling process

    if (argc != 4) {
        fprintf(stderr, "# Usage: ./cache_latency [L1 cache] [L2 cache] [L3 cache]");
    }
    const long L1_CACHE_SIZE = 1024 * atoi(argv[1]);
    const long L2_CACHE_SIZE = 1024 * atoi(argv[2]);
    const long L3_CACHE_SIZE = 1024 * atoi(argv[3]);

    const long TRASH_ARRAY_SIZE = L1_CACHE_SIZE + L2_CACHE_SIZE + L3_CACHE_SIZE;

    int *newArray = (int *)malloc(L1_CACHE_SIZE * sizeof(int));    
    int *evictArrayL1 = (int *)malloc(L1_CACHE_SIZE * sizeof(int));
    int *evictArrayL2 = (int *)malloc(L2_CACHE_SIZE * sizeof(int));
    int *evictArrayL3 = (int *)malloc(L3_CACHE_SIZE * sizeof(int));
    int *trashArray = (int *)malloc(TRASH_ARRAY_SIZE * sizeof(int));

    long index = 0;
    long count = 0;
    long readValue = 0;
    long tick1 = 0;
    long tick2 = 0;
    double latencyMainMem = 0;
    double latencyL1 = 0;
    double latencyL2 = 0;
    double latencyL3 = 0;

    struct timespec start_time, end_time;


    memset(newArray, 0xff, L1_CACHE_SIZE * sizeof(int));
    memset(evictArrayL1, 0xff, L1_CACHE_SIZE * sizeof(int));
    memset(evictArrayL2, 0xff, L2_CACHE_SIZE * sizeof(int));
    memset(evictArrayL3, 0xff, L3_CACHE_SIZE * sizeof(int));
    memset(trashArray, 0xff, TRASH_ARRAY_SIZE * sizeof(int)); // let it "clear" L1

    // TODO: 
    // 1. access main memory with newArray, then L1 should be full
    // 2. access newArray, measuring L1 latency
    // 3. evict content in L1, then values in newArray should now be in L2
    // 4. access newArray, measuring L2 latency
    // 5. evict content in L2, then values in newArray should now be in L3
    // 6. access newArray, measuring L3 latency

    for (int j = 0; j < 5; j++) {
        for (long i = 0; i < TRASH_ARRAY_SIZE; i++) {
            int clr = trashArray[i];
            clr++;
        }
    }

    // access main memory
    clock_gettime(CLOCK_REALTIME, &start_time);
    // for (long i = 0; i < L1_CACHE_SIZE; i++) {
    //     int temp = newArray[index];
    //     // just to avoid prefetching the adjacent lines
    //     #ifdef PREFETCH_ENABLED
    //     index++;
    //     #else
    //     index = (index + i) & 1023;
    //     #endif
    // }
    index = 0;
    count = 0;
    while (index < L1_CACHE_SIZE) {
        int tmp = newArray[index];               //Access Value from L2
        index = (index + tmp + ((index & 4) ? 28 : 36));   // on average this should give 32 element skips, with changing strides
        count++;                                           //divide overall time by this 
    }
    clock_gettime(CLOCK_REALTIME, &end_time);
    // latencyMainMem = ((end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec)) / L1_CACHE_SIZE;
    latencyMainMem = (end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec);
    latencyMainMem /= count;
    printf("Main memory latency: %f ns\n", latencyMainMem);

    // access L1 cache
    clock_gettime(CLOCK_REALTIME, &start_time);
    index = 0;
    count = 0;
    while (index < L1_CACHE_SIZE) {
        int tmp = newArray[index];               //Access Value from L2
        index = (index + tmp + ((index & 4) ? 28 : 36));   // on average this should give 32 element skips, with changing strides
        count++;                                           //divide overall time by this 
    }
    clock_gettime(CLOCK_REALTIME, &end_time);
    latencyL1 = (end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec);
    latencyL1 /= count;
    printf("L1 cache latency: %f ns\n", latencyL1);

    // evict values in L1, then the evicted values will go to L2
    // for (long i = 0; i < L1_CACHE_SIZE; i++) {
    //     int evicted_val = evictArrayL1[i];
    // }
    for (int j = 0; j < 100; j++) {
        for(count=0; count < L1_CACHE_SIZE; count++){
            int read = evictArrayL1[count]; 
            read++;
            readValue+=read;               
        }
    }

    // access L2 cache
    clock_gettime(CLOCK_REALTIME, &start_time);
    index = 0;
    count = 0;
    while (index < L1_CACHE_SIZE) {
        int tmp = newArray[index];               //Access Value from L2
        index = (index + tmp + ((index & 4) ? 28 : 36));   // on average this should give 32 element skips, with changing strides
        count++;                                           //divide overall time by this 
    }
    clock_gettime(CLOCK_REALTIME, &end_time);
    latencyL2 = (end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec);
    latencyL2 /= count;
    printf("L2 cache latency: %f ns\n", latencyL2);

    // evict values in L2, then the evicted values will go to L3
    for (int j = 0; j < 100; j++) {
        for(count=0; count < L2_CACHE_SIZE; count++){
            int read = evictArrayL2[count]; 
            read++;
            readValue+=read;               
        }
    }

    // access L3 cache
    clock_gettime(CLOCK_REALTIME, &start_time);
    index = 0;
    count = 0;
    while (index < L1_CACHE_SIZE) {
        int tmp = newArray[index];               //Access Value from L2
        index = (index + tmp + ((index & 4) ? 28 : 36));   // on average this should give 32 element skips, with changing strides
        count++;                                           //divide overall time by this 
    }
    clock_gettime(CLOCK_REALTIME, &end_time);
    latencyL3 = (end_time.tv_sec - start_time.tv_sec) * SECOND_TO_NS + (end_time.tv_nsec - start_time.tv_nsec);
    latencyL3 /= count;
    printf("L3 cache latency: %f ns\n", latencyL3);

    return 0;
}
