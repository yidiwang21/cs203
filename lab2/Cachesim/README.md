## Author
Yidi Wang, Ruzhuo Wang
## Usage
```
./run.sh -i [filename] -cs [Total cache size] -bs [Cache block size] -w [Number of ways] -v [victim block number]
```
## Performance (Hit Rate)
ASSOCIATIVITY (with victim cache disabled)

| Trace  | Direct  |  2-way  |  4 way  |Fully Assoc|
| ------ | ------- | ------- | ------- | --------- |
|gcc-10K |  94.53% |  94.54% |  94.54% |   94.54%  |
|gcc-1M  | 99.1626%| 99.2971%| 99.3022%|  99.3022% |


VICTIM CACHE (with gcc-1M trace)

|    VC     |    4    |    8    |    16   |
|-----------|---------|---------|---------|
|   Direct  | 99.1962%| 99.2193%| 99.2438%|
|   4-way   | 99.3022%| 99.3022%| 99.3022%|
|Fully Assoc| 99.3022%| 99.3022%| 99.3022%|
