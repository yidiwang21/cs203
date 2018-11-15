#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "unistd.h"

#include "cache.h"

using namespace std;

int main(int argc, char *argv[]) {
    
    int opt;
    int total_cache_size;
    int cache_block_size;
    int ways_num;
    int victim_block_num = 0;

    bool victim_cache_enabled = false;

    string filename = "gcc-1M.memtrace";

    while((opt = getopt(argc, argv, "i:c:b:w:v: ")) != EOF){    
        switch (opt) {
            case 'i': filename.assign(optarg); break;
            case 'c': total_cache_size = atoi(optarg); break;
            case 'b': cache_block_size = atoi(optarg); break;
            case 'w': ways_num = atoi(optarg); break;
            case 'v': victim_block_num = atoi(optarg); victim_cache_enabled = true; break;
            case '?': fprintf(stderr, "# Usage: \n -i [filename] -cs [Total cache size] -bs [Cache block size] -w [Number of ways] -v [victim block number]");
            default: cout<<endl; abort();
        }
    }

    if (total_cache_size > 4096 || total_cache_size < 1) {
        fprintf(stderr, "# Total cache size must be in range of 1 - 4096 (KB)! \n");
        return -1;
    }
    if (cache_block_size < 2 || cache_block_size > 64 || (cache_block_size & (cache_block_size - 1)) != 0) {
        fprintf(stderr, "# Cache block size must be in 2, 4, 8, 16, 32, 64 (B)! \n");
        return -1;
    }
    if (ways_num < 0 || ways_num > 16 || (ways_num & (ways_num - 1)) != 0) {
        fprintf(stderr, "# Number of ways must be in 0, 1, 2, 4, 8, 16! \n");
        return -1;   
    }
    if (victim_block_num != 0 && victim_block_num != 4 && victim_block_num != 8 && victim_block_num != 16) {
        fprintf(stderr, "# Size of victim cache must be in 4, 8, 16! \n");
        return -1;   
    }

    if (ways_num == 0) {
        cout << "# Fully associative cache..." << endl;
    }
    if (ways_num == 1) {
        cout << "# Direct mapped cache..." << endl;
    }

    CacheClass CacheArch(total_cache_size, cache_block_size, ways_num, victim_block_num, filename, victim_cache_enabled);
    CacheArch.Applications();
    
    return 0;
}
