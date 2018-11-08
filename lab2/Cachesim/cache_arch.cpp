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

    cout << "=======================================================" << endl;
    cout << "# Cache Design: " << endl;
    cout << "# Cache index: " << cache_index << endl;
    cout << "# Cache offset: " << cache_offset << endl;
    cout << "# Cache tag: " << cache_tag << endl;
    cout << "=======================================================" << endl;

    miss_num = 0;

    victimline.push_back(CacheLine());
}

void CacheClass::initArch() {
    long temp_index = pow(2, cache_index);
    index = (struct Index *)malloc(temp_index * sizeof(struct Index));
    cout << "# Allocating spaces for cache..." << endl;
    for (int i = 0; i < temp_index; i++) {
        index[i].cacheline = (struct CacheLine *)malloc(ways_num * sizeof(struct CacheLine));
        for (int j = 0; j < ways_num; j++) {
            index[i].cacheline[j].cnt = 0;
            index[i].cacheline[j].valid = true;
            index[i].cacheline[j].tag = 0;
        }
    }
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
            case '0': ret_addr.append("0000"); break;
            case '1': ret_addr.append("0001"); break;
            case '2': ret_addr.append("0010"); break;
            case '3': ret_addr.append("0011"); break;
            case '4': ret_addr.append("0100"); break;
            case '5': ret_addr.append("0101"); break;
            case '6': ret_addr.append("0110"); break;
            case '7': ret_addr.append("0111"); break;
            case '8': ret_addr.append("1000"); break;
            case '9': ret_addr.append("1001"); break;
            case 'A': ret_addr.append("1010"); break;
            case 'B': ret_addr.append("1011"); break;
            case 'C': ret_addr.append("1100"); break;
            case 'D': ret_addr.append("1101"); break;
            case 'E': ret_addr.append("1110"); break;
            case 'F': ret_addr.append("1111"); break;
        }
    }
    // extract last 32 bits if longer; add 0s to the front if shorter
    if (ret_addr.length() > ADDR_BITS) {
        ret_addr = ret_addr.substr(ret_addr.length() - ADDR_BITS);
    }
    if (ret_addr.length() < ADDR_BITS) {
        ret_addr = string(ADDR_BITS - ret_addr.length(), '0').append(ret_addr);
    }
    return ret_addr;
}

long CacheClass::computeIndex(string addr) {
    string str;
    str.assign(addr, cache_tag, cache_index);
    long ret = 0;
    for (int i = 0; i < cache_index; i++) {
        if (str[str.length() - i] == '1') {
            ret += pow(2, i);
        }
    }
    return ret;
}

long CacheClass::computeTag(string addr) {
    string str;
    str.assign(addr, 0, cache_tag);
    long ret = 0;
    for (int i = 0; i < cache_index; i++) {
        if (str[str.length() - i] == '1') {
            ret += pow(2, i);
        }
    }
    return ret;
}

long CacheClass::computeOffset(string addr) {
    string str;
    str.assign(addr, cache_tag + cache_index, cache_offset);
    long ret = 0;
    for (int i = 0; i < cache_index; i++) {
        if (str[str.length() - i] == '1') {
            ret += pow(2, i);
        }
    }
    return ret;
}

long CacheClass::computeAddr(string addr) {
    long ret = 0;
    for (int i = 0; i < cache_index + cache_tag; i++) {
        if (addr[addr.length() - i] == '1') {
            ret += pow(2, i);
        }
    }
    return ret;
}

// return val: 
// 0: miss in both cache and victim cache
// 1: hit in cache
// 2: miss in cache, hit in victim cache
int CacheClass::isHit(struct FileLine fileline) {
    int idx = computeIndex(fileline.addr);
#ifdef DEBUG
    cout << "computed idx: " << idx << endl;
#endif // DEBUG
    long tag = computeTag(fileline.addr);
    // FIXME: 
    long dest_tag = computeAddr(fileline.addr);
    // long dest_tag = (tag << idx) | idx;     // tag that we need to find in victim cache
    

    // hit in cache
    for (int i = 0; i < ways_num; i++) {
#ifdef DEBUG
        cout << "valid: " << index[idx].cacheline[i].valid << endl;
        cout << "tag: " << index[idx].cacheline[i].tag << endl;
#endif // DEBUG
        if (index[idx].cacheline[i].valid == false && index[idx].cacheline[i].tag == tag) {
            index[idx].cacheline[i].cnt += 1;
            return 1;
        }
    }
    // hit in victim cache
    for (int i = 0; i < victim_block_num && i < victimline.size(); i++) {
        if (victimline[i].tag == dest_tag) {
            cout << "Victim cache useful!" << endl;
            // hit_num += 1;
            victimline[i].cnt += 1;
            return 2;
        }
    }
    // miss in both cache and victim cache
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

    if (isHit(fileline) == 1) { // hit in cache. do nothing
        return;
    }
    // FIXME: this never happens!
    if (isHit(fileline) == 2) { // miss in cache, hit in victim cache, do replacement, LRU
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
                index[idx].cacheline[i].valid = false;
                // insert evicted line into victim cache
                // if empty lines in victim cache, push directly
                // if not, find the min cnt in victim cache and replace
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
                    return;
                }
            }
        }   
    }
    if (isHit(fileline) == 0) {                         // miss in both cache and victim cache, insert a new line
        miss_num += 1;
        for (int i = 0; i < ways_num; i++) {
            if (index[idx].cacheline[i].valid == 1) {
                index[idx].cacheline[i].tag = tag;
                index[idx].cacheline[i].valid = false;
                index[idx].cacheline[i].cnt += 1;
                return;
            }
        }
        // FIXME: this never happens!
        // no valid place for this addr, replace
        for (int i = 1; i < ways_num; i++) {
            if (min_cnt > index[idx].cacheline[i].cnt) {
                min_cnt = index[idx].cacheline[i].cnt;
                min_line = i;
            }
        }
        for (int i = 1; i < ways_num; i++) {
            // first assign to evicted cache line
            evicted_cacheline = index[idx].cacheline[i];
            // do replacement
            index[idx].cacheline[i].tag = tag;
            index[idx].cacheline[i].cnt = 1;
            index[idx].cacheline[i].valid = false;
        }
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
            return;
        }
    }
}

double CacheClass::computeMissRate(long l, long miss_num) {
    return 100 * double(miss_num) / double(l);
}

void CacheClass::Applications() {
    cout << "# Reading trace file..." << filename << endl;
    vector<struct FileLine> filelines = readFile(filename);
    cout << "# Total lines in the file: " << l << endl;
    initArch();
    int v = 1;

    while (v < filelines.size()) {
#ifdef DEBUG
        cout << "filelines[" << v << "].addr: " << filelines[v].addr << endl;
#endif
        filelines[v].addr = convertAddr(filelines[v].addr);
#ifdef DEBUG
        cout << "filelines[" << v << "].addr: " << filelines[v].addr << endl;
#endif // DEBUG
        insertLine(filelines[v]);
        v++;
    }
    float miss_rate = computeMissRate(l, miss_num);
    cout << "miss_num: " << miss_num << endl;
    cout << "l: " << l << endl;
    cout << "# Miss rate of file " << filename << " is: " << miss_rate << "%" << endl;
    exit (EXIT_SUCCESS);
}
