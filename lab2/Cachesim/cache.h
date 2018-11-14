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
    CacheClass(unsigned int t, unsigned int c, unsigned int w, unsigned int v, string fn, bool ve);
    unsigned int total_cache_size;
    unsigned int cache_block_size;
    unsigned int ways_num;
    unsigned int victim_block_num;
    string filename;
    bool victim_cache_enabled;

    void Applications();

private:
    unsigned int cache_offset;
    unsigned int cache_index;
    unsigned int cache_tag;
    unsigned int cache_entry;

    unsigned long actual_index;
    unsigned long entry_per_index;

    unsigned long miss_num;
    unsigned long line_num;

    // methods of operation
    void initArch(void);
    void clearVictimLine(int idx);
    void insertLine(struct FileLine fileline);
    unsigned long updateMinCacheline(unsigned long idx);
    unsigned int updateMinVictimline();
    int isHit(struct FileLine fileline);
    string convertAddr(string str);
    unsigned long computeIndex(string addr);
    unsigned long computeTag(string addr);
    unsigned long computeOffset(string addr);
    unsigned long computeAddr(string addr);
    double computeMissRate(long line_num, long miss_num);
    double computeHitRate(long line_num, long miss_num);
    vector<struct FileLine> readFile(string filename);  

    // basic data struct of cache
    unsigned long **tag_array;    // store tags, dimension: index * ways
    unsigned long **cnt_array;    // store cnt of each tag, dimension: index * ways
    bool **valid_array;    // store valid bit of each tag, dimension: index * ways
    unsigned long *min_cnt_col_array;    // store the col idx of the minimum cnt in a index, dimension: index * 1
    // victim cache
    unsigned long *victim_tag;   // store tag in the victim cache, dimension: 1 * victim_block_num
    unsigned long *victim_cnt; // store cnt in the victim cache, dimension: 1 * victim_block_num
    bool *victim_valid; // store valid bit in the victim cache, dimension: 1 * victim_block_num
    int min_cnt_col_victim;    // store the col idx of the minimum cnt in the vicitm cache
    int hit_victim_col;       // store the entry # of victim cache if hit in victim cache
    // evicted buf
    unsigned long evicted_tag;  // need to convert to victim tag
    unsigned long evicted_cnt;
};

#endif