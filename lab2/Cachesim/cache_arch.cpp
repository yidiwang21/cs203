#include "cache_arch.h"

CacheClass::CacheClass(int t, int c, int w, int v, string fn) {
    total_cache_size = t;
    cache_block_size = c;
    ways_num = w;
    victim_block_num = v;
    filename = fn;

    cache_offset = log(cache_block_size)/log(2);
    cache_index = (ways_num == 0) ? 0 : (log(total_cache_size / cache_block_size)/log(2) + 10 - log(ways_num)/log(2));  // consider fully assoc cache
    cache_tag = ADDR_BITS - cache_index - cache_offset;
    cache_entry = log(total_cache_size / cache_block_size)/log(2) + 10;

    fully_assoc_entry = (ways_num == 0) ? pow(2, cache_entry) : 0;

    cout << "=======================================================" << endl;
    cout << "# Cache Design: " << endl;
    cout << "# Cache index: " << cache_index << endl;
    cout << "# Cache offset: " << cache_offset << endl;
    cout << "# Cache tag: " << cache_tag << endl;
    cout << "# Cache entry: " << cache_entry << endl;
    cout << "=======================================================" << endl;

    miss_num = 0;

    victimline.push_back(CacheLine());
}

void CacheClass::initArch() {
    if (ways_num != 0) {
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
    }else { // fully assoc cache, ways_num = 0
        struct Index tmp;
        tmp.cacheline = (struct CacheLine *)malloc(sizeof(struct CacheLine));
        tmp.cacheline->cnt = 0;
        tmp.cacheline->valid = true;
        tmp.cacheline->tag = 0;
        // fully_assoc_lines.push_back(Index());
        for (int i = 0; i < fully_assoc_entry; i++) {
            fully_assoc_lines.push_back(tmp);
#ifdef DEBUG
            cout << "fully_assoc_lines [" << i << "].cacheline->valid: " << fully_assoc_lines[i].cacheline->valid << endl;
#endif // DEBUG
        }
        cout << "size of fully_assoc_lines: " << fully_assoc_lines.size() << endl;
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
    for (int i = 0; i < cache_tag; i++) {
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
    for (int i = 0; i < cache_offset; i++) {
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
    long tag = computeTag(fileline.addr);
    // FIXME: 
    long dest_tag = computeAddr(fileline.addr);
    // long dest_tag = (tag << idx) | idx;     // tag that we need to find in victim cache
    
    if (ways_num != 0) {
        // hit in cache
        for (int i = 0; i < ways_num; i++) {
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
    }else {
        // hit in cache
        for (int i = 0; i < fully_assoc_entry; i++) {
            if (fully_assoc_lines[i].cacheline->valid == false && fully_assoc_lines[i].cacheline->tag == tag) {
                cout << "fully_assoc_lines[i].cacheline->valid: " << fully_assoc_lines[i].cacheline->valid << endl;
                cout << "Hit!!!" << endl;
                fully_assoc_lines[i].cacheline->cnt += 1;
                return 1;
            }
        }
        // hit in victim cache
        for (int i = 0; i < victim_block_num && i < victimline.size(); i++) {
            if (victimline[i].tag == dest_tag) {
                cout << "Victim cache useful!" << endl;
                victimline[i].cnt += 1;
                return 2;
            }
        }
        return 0;
    }
}

void CacheClass::insertLine(struct FileLine fileline) {
    int idx = computeIndex(fileline.addr);
    long tag = computeTag(fileline.addr);
    cout << "computed tag = " << tag << endl;
    long dest_tag = (tag << idx) | idx;     // tag that we need to find in victim cache
    long min_cnt = LONG_MAX;
    long min_victim_cnt = LONG_MAX;
    int min_victim_line = 0;
    int min_line = 0;

    if (ways_num != 0) {
        if (isHit(fileline) == 1) { // hit in cache. do nothing
            return;
        }
        // FIXME: this never happens!
        // TODO: add break
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
                    // if victim cache available, push directly
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
        if (isHit(fileline) == 0) { // miss in both cache and victim cache, insert a new line
            miss_num += 1;
            for (int i = 0; i < ways_num; i++) {
                if (index[idx].cacheline[i].valid == 1) {
                    index[idx].cacheline[i].tag = tag;
                    index[idx].cacheline[i].valid = false;
                    index[idx].cacheline[i].cnt = 1;
                    return;
                }
            }
            // FIXME: this never happens!
            // no valid place for this addr, replace
            // FIXME: cnt check
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
    }else { // fully assoc cache, ways_num = 0
        if (isHit(fileline) == 1) { // hit in cache. do nothing
            cout << "11111111111111111" << endl;
            return;
        }
        if (isHit(fileline) == 2) { // miss in cache, hit in victim cache
            for (int i = 0; i < fully_assoc_entry; i++) { // find an valid line, and insert
                if (fully_assoc_lines[i].cacheline->valid == true) {
                    fully_assoc_lines[i].cacheline->valid = false;
                    fully_assoc_lines[i].cacheline->cnt = 1;
                    fully_assoc_lines[i].cacheline->tag = tag;
                    return;
                }
            }
            // cache full, do replacement
            // find the min cnt line in cache, LRU
            for (int i = 0; i < fully_assoc_entry; i++) {
                if (min_cnt > fully_assoc_lines[i].cacheline->cnt) {
                    min_cnt = fully_assoc_lines[i].cacheline->cnt;
                    min_line = i;
                }
            }
            evicted_cacheline = *(fully_assoc_lines[min_line].cacheline);
            // replace
            fully_assoc_lines[min_line].cacheline->cnt = 1;
            fully_assoc_lines[min_line].cacheline->tag = tag;
            fully_assoc_lines[min_line].cacheline->valid = false;
            // insert the evicted line into victim cache
            if (victimline.size() < victim_block_num) {
                victimline.push_back(evicted_cacheline);
                return;
            }else { // replace in victim cache at line min_victim_line
                for (int j = 0; j < victim_block_num; j++) {
                    if (min_victim_cnt > victimline[j].cnt) {
                        min_victim_cnt = victimline[j].cnt;
                        min_victim_line = j;
                    }
                }
                victimline[min_victim_line] = evicted_cacheline;
                return;
            }
        }
        if (isHit(fileline) == 0) { // miss in both cache and victim cache
            miss_num += 1;
            // find the first available line and insert
            for (int i = 0; i < fully_assoc_entry; i++) {
                if (fully_assoc_lines[i].cacheline->valid == true) {
                    fully_assoc_lines[i].cacheline->valid = false;
                    fully_assoc_lines[i].cacheline->cnt = 1;
                    fully_assoc_lines[i].cacheline->tag = tag;
                    cout << "insert tag = " << fully_assoc_lines[i].cacheline->tag << endl;
                    cout << "insert valid = " << fully_assoc_lines[i].cacheline->valid << endl;
                    return;
                }
            }
            // if no available place in cache, LRU
            for (int i = 1; i < fully_assoc_entry; i++) {
                if (min_cnt > fully_assoc_lines[i].cacheline->cnt) {
                    min_cnt = fully_assoc_lines[i].cacheline->cnt;
                    min_line = i;
                }
            }
            evicted_cacheline = *(fully_assoc_lines[min_line].cacheline);
            fully_assoc_lines[min_line].cacheline->cnt = 1;
            fully_assoc_lines[min_line].cacheline->tag = tag;
            fully_assoc_lines[min_line].cacheline->valid = false;
            if (victimline.size() < victim_block_num) {
                victimline.push_back(evicted_cacheline);
                return;
            }else {
                for (int j = 0; j < victim_block_num; j++) {
                    if (min_victim_cnt > victimline[j].cnt) {
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
