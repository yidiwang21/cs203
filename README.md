# cs203

Based on <https://bitbucket.org/danwong/cs203-labs-f16>

### Usage:
#### lab1:
```
make
./pipesim -i [filename] -f [forwarding window width (0~2)]
```
#### lab2:
```
./run.sh -i [filename] -cs [Total cache size] -bs [Cache block size] -w [Number of ways] -v [victim block number]
```
#### lab3:
```
make
./cache_latency [L1(D) cache size (KB)] [L2 cache size (KB)] [L3 cache size (KB)]
```
### Performance
#### Hit Rate
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

#### Cache Latencies

| Main memory latency | 19.631031 ns |
|  L1 cache latency   | 7.632923 ns  |
|  L2 cache latency   | 11.526963 ns |
|  L3 cache latency   | 15.453169 ns |
