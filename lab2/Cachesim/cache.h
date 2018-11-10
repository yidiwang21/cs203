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

struct FileLine {
    char opcode;
    int offset;
    string addr;
};

class CacheClass
{
public:
    CacheClass(int t, int c, int w, int v, string fn, bool ve);
    int total_cache_size;
    int cache_block_size;
    int ways_num;
    int victim_block_num;
    string filename;
    bool victim_cache_enabled;

private:
    int cache_offset;
    int cache_index;
    int cache_tag;
    int cache_entry;

    long actual_index;
    long entry_per_index;

    long miss_num;
    long line_num;

    // methods of operation
    void initArch(void);
    void insertLine(struct FileLine fileline);
    int isHit(struct FileLine fileline);
    string convertAddr(string str);
    long computeIndex(string addr);
    long computeTag(string addr);
    long computeOffset(string addr);
    long computeAddr(string addr);
    double computeMissRate(long line_num, long miss_num);
    vector<struct FileLine> readFile(string filename);  

    // basic data struct of cache
    long **tag_array;    // store tags, dimension: index * ways
    long **cnt_array;    // store cnt of each tag, dimension: index * ways
    bool **valid_array;    // store valid bit of each tag, dimension: index * ways
    long *min_cnt_col_array;    // store the col idx of the minimum cnt in a index, dimension: index * 1
    // victim cache
    long *victim_tag;   // store tag in the victim cache, dimension: 1 * victim_block_num
    long *victim_cnt; // store cnt in the victim cache, dimension: 1 * victim_block_num
    bool *victim_valid; // store valid bit in the victim cache, dimension: 1 * victim_block_num
    int min_cnt_col_victim;    // store the col idx of the minimum cnt in the vicitm cache
};

#endif