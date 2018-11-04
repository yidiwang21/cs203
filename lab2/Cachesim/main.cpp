#include <iostrema>
#include <stdlib.h>
#include <stdio.h>
#include "unistd.h"

using namespace std;

int main(int argc, char const *argv[])
{
    
    int opt;
    int total_cache_size;
    int cache_block_size;
    int num_of_ways;

    string filename = "gcc-1M.memtrace";

    while((opt = getopt(argc, argv, "")) != EOF){    
        switch (opt)
        {
            case 'i': filename.assign(optarg); break;
            case 'cs': total_cache_size = atoi(optarg);
            case 'bs': cache_block_size = atoi(optarg);
            case 'w': num_of_ways = atoi(optarg);
            case '?': fprintf(stderr, "Usage: \n -i [filename] \n
                                                    -cs [Total cache size (KB: 1 - 4096)] \n
                                                    -bs [Cache block size (B: 2, 4, 8, 16, 32, 64)] \n
                                                    -w [Number of ways (0, 1, 2, 4, 8, 16)] ");          
            default: cout<<endl; abort();
        }
    }
    
    if (total_cache_size > 4096 || total_cache_size < 1) {
        fprintf(stderr, "Total cache size must be in range of 1 - 4096 (KB)!");
        return -1;
    }
    if (cache_block_size < 2 || cache_block_size > 64 || (cache_block_size & (cache_block_size - 1)) != 0) {
        fprintf(stderr, "Cache block size must be in 2, 4, 8, 16, 32, 64 (B)!");
        return -1;
    }
    if (num_of_ways < 0 || num_of_ways > 16 || (num_of_ways & (num_of_ways - 1)) != 0) {
        fprintf(stderr, "Number of ways must be in 0, 1, 2, 3, 4, 8, 16!");
        return -1;   
    }

    cout<<"passed"<<endl;
    return 0;
}
