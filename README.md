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
make
./cachesim -i [filename] -c [cache size (KB)] -b [cache block szie (B)] -w [set assoc] -v [victim cache lines]
```
### Performance
ASSOCIATIVITY
with victim cache disabled
| Trace  | Direct  |  2-way  |  4 way  |Fully Assoc|
| ------ | ------- | ------- | ------- | --------- |
|gcc-10K |  5.47%  |  5.46%  |  5.46%  |   5.46%   |
|gcc-1M  |0.837404%|0.734005%|0.700558%| 0.69776%  |


VICTIM CACHE
with gcc-1M trace
|    VC     | No VC   |    4    |    8    |   16    |
|-----------|---------|---------|---------|---------|
|   Direct  |0.837404%|0.82042% |0.817622%|0.810789%|
|   4-way   |0.700558%|0.698021%|0.698021%|0.69776% |
|Fully Assoc|0.69776% |0.69776% |0.69776% |0.69776% |

| First Header  | Second Header |
| ------------- | ------------- |
| Content Cell  | Content Cell  |
| Content Cell  | Content Cell  |
