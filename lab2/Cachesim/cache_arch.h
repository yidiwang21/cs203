#ifndef _CACHE_ARCH_
#define _CACHE_ARCH_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

// #define DEBUG

#define INT_MAX     65535
#define LONG_MAX    4294967295
#define ADDR_BITS   32

class CacheClass {
public:
    CacheClass(int t, int c, int w, int v, string fn);
    int total_cache_size;
    int cache_block_size;
    int ways_num;
    int victim_block_num;
    bool victim_cache_enabled;
    string filename;

    // long hit_num = 0;
    long miss_num;

    int cache_offset;
    int cache_index;
    int cache_tag;

    void Applications();
    
private:
    int cache_entry;
    long l = 0;          // line cnt in a file

    struct CacheLine {
        bool valid;
        long tag;
        int data;
        long cnt;
    };
    struct CacheLine evicted_cacheline;
    vector<struct CacheLine> victimline;

    struct Index {
        int cacheline_num;  
        int entry;
        // vector<struct CacheLine> cacheline;
        struct CacheLine *cacheline;
    };
    // vector<struct Index> index;
    // struct Index* index = (struct Index *)malloc(cache_index * sizeof(struct Index));
    struct Index* index;

    void initArch(void);
    void insertLine(struct FileLine fileline);
    int isHit(struct FileLine fileline);

    string convertAddr(string str);
    vector<struct FileLine> readFile(string filename);  
    int computeIndex(string addr);
    int computeTag(string addr);
    int computeOffset(string addr);
    double computeMissRate(long l, long miss_num);
};

struct FileLine {
    char opcode;
    int offset;
    string addr;
};

#endif