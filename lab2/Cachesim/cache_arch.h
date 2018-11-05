#ifndef _CACHE_ARCH_
#define _CACHE_ARCH_

#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>

using namespace std;

class CacheArch
{
public:
    CacheArch();
    int total_cache_size;
    int cache_block_size;
    int ways_num;
    int victim_block_num;

    int hit_num = 0;
    int miss_num = 0;

    int cache_offset = log(cache_block_size)/log(2);
    int cache_index = log(total_cache_size / cache_block_size)/log(2) + 10 - log(ways_num)/log(2);
    int cache_tag = 32 - cache_index - cache_offset;
    
private:
    
    int cache_entry = log(total_cache_size / cache_block_size)/log(2) + 10;

    struct CacheLine {
                bool valid = 1; // 1: valid
                int tag;
                int data;
                long cnt;
            };
    struct CacheLine evicted_cacheline;

    struct Index {
        int cacheline_num;  
        int entry;
        struct CacheLine* cacheline;
    };

    struct Index* index = (struct Index *)malloc(cache_index * sizeof(struct Index));

    // FIXME: 
    void init_struct() {
        this->index.cacheline = (struct CacheLine *)malloc(ways_num * sizeof(struct CacheLine));
    }
    
    // struct Evicted_CacheLine{
    //     bool valid = 1; // 1: valid
    //     int tag;
    //     int data;
    //     long cnt;
    // };
    

    void insertLine(struct FileOpt::FileLine fileline);
    bool isHit(struct FileOpt::FileLine fileline);
};


class FileOpt
{
public:
    FileOpt();
    string convertAddr(string str);
    struct FileLine readLine(string filename);  
    int computeIndex(string addr);
    int computeTag(string addr);
    int computeOffset(string addr);
    
    struct FileLine {
        char inst;
        int offset;
        string addr;
    };
private:
    
};



extern CacheArch cachearch;

extern FileOpt fileopt;

#endif