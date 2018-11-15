#!/bin/bash

filename=
total_cache_size=0
cache_block_size=0
ways_num=0
victim_block_num=0

while [ -n "$1" ]; do
    case $1 in 
        -i  | --file)               shift
                                    filename=$1
                                    ;;
        -cs | --cache_size)         shift
                                    total_cache_size=$1
                                    ;;
        -bs | --block_size)         shift
                                    cache_block_size=$1
                                    ;;
        -w  | --#ways)              shift
                                    ways_num=$1
                                    ;;
        -v  | --#victim_cache)      shift
                                    victim_block_num=$1
                                    ;;
        -h  | --help)               echo "# Usage: -i [filename] -cs [Total cache size] -bs [Cache block size] -w [Number of ways] -v [victim block number]"
                                    exit
                                    ;;
        * )                         echo "Option $1 not recognized!"
    esac
    shift
done

./cachesim -i ${filename} -c ${total_cache_size} -b ${cache_block_size} -w ${ways_num} -v ${victim_block_num}