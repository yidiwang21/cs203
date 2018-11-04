#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "unistd.h"

using namespace std;

int main(int argc, char *argv[])
{
    
    int opt;
    int total_cache_size;
    int cache_block_size;
    int num_of_ways;

    string filename = "gcc-1M.memtrace";

    while((opt = getopt(argc, argv, "i:c:b:w: ")) != EOF){    
        switch (opt)
        {
            case 'i': filename.assign(optarg); break;
            case 'c': total_cache_size = atoi(optarg); break;
            case 'b': cache_block_size = atoi(optarg); break;
            case 'w': num_of_ways = atoi(optarg); break;
            case '?': fprintf(stderr, "# Usage: \n -i [filename] -cs [Total cache size] -bs [Cache block size] -w [Number of ways] ");
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
    if (num_of_ways < 0 || num_of_ways > 16 || (num_of_ways & (num_of_ways - 1)) != 0) {
        fprintf(stderr, "# Number of ways must be in 0, 1, 2, 3, 4, 8, 16! \n");
        return -1;   
    }

    cout<<"passed"<<endl;
    return 0;
}
