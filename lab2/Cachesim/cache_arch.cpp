#include "cache_arch.h"

CacheClass::CacheClass(int t, int c, int w, int v, string fn) {
    total_cache_size = t;
    cache_block_size = c;
    ways_num = w;
    victim_block_num = v;
    filename = fn;

    cache_offset = log(cache_block_size)/log(2);
    cache_index = log(total_cache_size / cache_block_size)/log(2) + 10 - log(ways_num)/log(2);
    cache_tag = ADDR_BITS - cache_index - cache_offset;
    cache_entry = log(total_cache_size / cache_block_size)/log(2) + 10;

    miss_num = 0;

    victimline.push_back(CacheLine());
    index.push_back(Index());
}

vector<struct FileLine> CacheClass::readFile(string filename) {
    vector<struct FileLine> filelines;
    filelines.push_back(FileLine());

    struct FileLine temp_line;

    ifstream infile;
    infile.open(filename.c_str(), std::ifstream::in);

    string sline = "";
    char inst_opcode;
    int inst_offset;
    string inst_addr;
    while (!infile.eof()) {
        getline(infile, sline);
        istringstream iss(sline);
        if (!(iss >> inst_opcode >> inst_offset >> inst_addr)) break;
#ifdef DEBUG
        cout << "opcode: " << inst_opcode << endl;
        cout << "offset: " << inst_offset << endl;
        cout << "addr: " << inst_addr << endl;
#endif
        temp_line.addr = inst_addr;
        temp_line.opcode = inst_opcode;
        temp_line.offset = inst_offset;
        filelines.push_back(temp_line);
        l++;
    }
    infile.close();
    return filelines;
}

string CacheClass::convertAddr(string str) {
    string ret_addr;
    for (int i = 0; i < str.length(); i++) {
        switch(toupper(str[i])) {
            case '0': ret_addr.append("0000");
            case '1': ret_addr.append("0001");
            case '2': ret_addr.append("0010");
            case '3': ret_addr.append("0011");
            case '4': ret_addr.append("0100");
            case '5': ret_addr.append("0101");
            case '6': ret_addr.append("0110");
            case '7': ret_addr.append("0111");
            case '8': ret_addr.append("1000");
            case '9': ret_addr.append("1001");
            case 'A': ret_addr.append("1010");
            case 'B': ret_addr.append("1011");
            case 'C': ret_addr.append("1100");
            case 'D': ret_addr.append("1101");
            case 'E': ret_addr.append("1110");
            case 'F': ret_addr.append("1111");
        }
    }
    return ret_addr;
}

int CacheClass::computeIndex(string addr) {
    string str;
    str.assign(addr, cache_tag, cache_index);
    int ret = 0;
    char c;
    for (int i = 0; i < cache_index; i++) {
        ret += 2 ^ atoi(&str[i]);
    }
    return ret;
}

int CacheClass::computeTag(string addr) {
    string str;
    str.assign(addr, 0, cache_tag);
    int ret = 0;
    char c;
    for (int i = 0; i < cache_index; i++) {
        ret += 2 ^ atoi(&str[i]);
    }
    return ret;
}

int CacheClass::computeOffset(string addr) {
    string str;
    str.assign(addr, cache_tag + cache_index, cache_offset);
    int ret = 0;
    char c;
    for (int i = 0; i < cache_index; i++) {
        ret += 2 ^ atoi(&str[i]);
    }
    return ret;
}

// return val: 
// 0: miss in both cache and victim cache
// 1: hit in cache
// 2: miss in cache, hit in victim cache
int CacheClass::isHit(struct FileLine fileline) {
    int idx = computeIndex(fileline.addr);
    long tag = computeTag(fileline.addr);
    long dest_tag = (tag << idx) | idx;     // tag that we need to find in victim cache

    // hit in cache
    for (int i = 0; i < ways_num; i++) {
        if (!index[idx].cacheline[i].valid && index[idx].cacheline[i].tag == tag) {
            index[idx].cacheline[i].cnt += 1;
            return 1;
        }
    }
    // hit in victim cache
    for (int i = 0; i < victim_block_num && i < victimline.size(); i++) {
        if (victimline[i].tag == dest_tag) {
            // hit_num += 1;
            victimline[i].cnt += 1;
            return 2;
        }
    }
    // miss in both cache and victim cache
    miss_num += 1;
    return 0;
}

void CacheClass::insertLine(struct FileLine fileline) {
    int idx = computeIndex(fileline.addr);
    long tag = computeTag(fileline.addr);
    long dest_tag = (tag << idx) | idx;     // tag that we need to find in victim cache
    long min_cnt = LONG_MAX;
    long min_victim_cnt = LONG_MAX;
    int min_victim_line = 0;
    int min_line = 0;

    cout << "44444444444444" << endl;

    if (isHit(fileline) == 1) { // hit in cache. do nothing
        return;
    }
    if (isHit(fileline) == 2) { // miss in cache, hit in victim cache, do replacement, LRU
        for (int i = 0; i < ways_num; i++) {
            min_cnt = min(min_cnt, index[idx].cacheline[i].cnt);
        }
        cout << "55555555555555" << endl;
        for (int i = 0; i < ways_num; i++) {
            if (index[idx].cacheline[i].cnt == min_cnt) {
                // first assign to evicted cache line
                evicted_cacheline = index[idx].cacheline[i];
                // do replacement
                index[idx].cacheline[i].tag = tag;
                index[idx].cacheline[i].cnt = 1;
                // insert evicted line into victim cache
                // if empty lines in victim cache, push directly
                // if not, find the min cnt in victim cache and replace
                cout << "66666666666666" << endl; 
                if (victimline.size() < victim_block_num) {
                    victimline.push_back(evicted_cacheline);
                }else {
                    for (int j = 0; j < victim_block_num; j++) {
                        if (min_victim_cnt > victimline[j].cnt) {   // if update, fresh min victim line index
                            min_victim_cnt = victimline[j].cnt;
                            min_victim_line = j;
                        }
                    }
                    victimline[min_victim_line] = evicted_cacheline;
                }
            }
        }   
    }
    cout << "777777777777777777" << endl;
    if (isHit(fileline) == 0) {                         // miss in both cache and victim cache, insert a new line
        for (int i = 0; i < ways_num; i++) {
            if (index[idx].cacheline[i].valid == 1) {
                index[idx].cacheline[i].tag = tag;
                index[idx].cacheline[i].valid = 0;
                index[idx].cacheline[i].cnt += 1;
                return;
            }
        }
        // no valid place for this addr, replace
        for (int i = 1; i < ways_num; i++) {
            if (min_cnt > index[idx].cacheline[i].cnt) {
                min_cnt = index[idx].cacheline[i].cnt;
                min_line = i;
            }
        }
        // TODO: evict
    }
}

float CacheClass::computeMissRate(long l, long miss_num) {
    return miss_num / l;
}

void CacheClass::Applications() {
    cout << "Reading trace file..." << filename << endl;
    vector<struct FileLine> filelines = readFile(filename);
    cout << "Total lines in the file: " << l << endl;
    int v = 0;

    while (v < filelines.size()) {
        filelines[v].addr = convertAddr(filelines[v].addr);
        cout << "222222222222222" << endl;
        insertLine(filelines[v]);
        cout << "333333333333333" << endl;
        v++;
    }
    float miss_rate = computeMissRate(l, miss_num);
    cout << "Miss rate of file [" << filename << "] is: " << miss_rate << endl;
}
