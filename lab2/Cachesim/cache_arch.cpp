#include "cache_arch.h"
#include <fstream>

CacheClass::CacheClass() {}

vector<struct FileLine> CacheClass::readFile(string filename) {
    vector<struct FileLine> filelines;
    filelines.push_back(FileLine());

    ifstream infile(filename);
    string line;
    char inst_opcode;
    int inst_offset;
    string inst_addr;
    int l = 0;
    while (getline(infile, line)) {
        istringstream iss(line);
        if (!(iss >> inst_opcode >> inst_offset >> inst_addr)) break;
        filelines[l].addr = inst_addr;
        filelines[l].opcode = inst_opcode;
        filelines[l].offset = inst_offset;
        l++;
    }
    return 
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
    for (int i = 0; i < victim_block_num; i++) {
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

    if (isHit(fileline) == 1) { // do nothing
        return;
    }
    if (isHit(fileline) == 2) { // do replacement, LRU
        for (int i = 0; i < ways_num; i++) {
            min_cnt = min(min_cnt, index[idx].cacheline[i].cnt);
        }
        for (int i = 0; i < ways_num; i++) {
            if (index[idx].cacheline[i].cnt == min_cnt) {
                // first assign to evicted cache line
                evicted_cacheline = index[idx].cacheline[i];
                // do replacement
                index[idx].cacheline[i].tag = tag;
                index[idx].cacheline[i].cnt = 1;
            }
        }   
    }
    if (isHit(fileline) == 0) {                         // insert a new line
        for (int i = 0; i < ways_num; i++) {
            if (index[idx].cacheline[i].valid == 1) {
                index[idx].cacheline[i].tag = tag;
                index[idx].cacheline[i].valid = 0;
                index[idx].cacheline[i].cnt += 1;
                return;
            }   
        }
    }
}
