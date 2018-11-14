#include "cache.h"

using namespace std;

// #define DEBUG

#define ADDR_BITS   32
#define INT_MAX     65535
#define LONG_MAX    4294967295   

CacheClass::CacheClass(unsigned int t, unsigned int c, unsigned int w, unsigned int v, string fn, bool ve) {
    total_cache_size = t;
    cache_block_size = c;
    ways_num = w;
    victim_block_num = v;
    filename = fn;
    victim_cache_enabled = ve;

    cache_offset = log(cache_block_size)/log(2);
    cache_index = (ways_num == 0) ? 0 : (log(total_cache_size / cache_block_size)/log(2) + 10 - log(ways_num)/log(2));  // consider fully assoc cache
    cache_tag = ADDR_BITS - cache_index - cache_offset;
    cache_entry = log(total_cache_size / cache_block_size)/log(2) + 10;

    actual_index = pow(2, cache_index);
    entry_per_index = (ways_num == 0) ? pow(2, cache_entry) : ways_num;

    hit_victim_col = -1;
    evicted_tag = 0;
    evicted_cnt = 0;

    miss_num = 0;
    line_num = 0;

    cout << "=======================================================" << endl;
    cout << "# Cache Design: " << endl;
    cout << "# Cache index: " << cache_index << endl;
    cout << "# Cache offset: " << cache_offset << endl;
    cout << "# Cache tag: " << cache_tag << endl;
    cout << "# Cache entry per index: " << entry_per_index << endl;
    cout << "# Victim cache enabled: " << victim_cache_enabled << endl;
    cout << "# Victim cache blocks: " << victim_block_num << endl;
    if (ways_num == 0) cout << "# Fully associative cache..." << endl;
    if (ways_num == 1) cout << "# Direct mapped cache..." << endl;
    cout << "=======================================================" << endl;
}

void CacheClass::initArch() {
    // allocate spaces cache (including fully assoc cache), using row ptr 
    cout << "# Allocating spaces for cache..." << endl;
    tag_array = (unsigned long **)malloc(actual_index * sizeof(unsigned long *));
    cnt_array = (unsigned long **)malloc(actual_index * sizeof(unsigned long *));
    valid_array = (bool **)malloc(actual_index * sizeof(bool *));
    min_cnt_col_array = (unsigned long *)malloc(actual_index * sizeof(unsigned long));
    for (unsigned long i = 0; i < actual_index; i++) {
        tag_array[i] = (unsigned long *)malloc(entry_per_index * sizeof(unsigned long));
        cnt_array[i] = (unsigned long *)malloc(entry_per_index * sizeof(unsigned long));
        valid_array[i] = (bool *)malloc(entry_per_index * sizeof(bool));
        // initialize values
        min_cnt_col_array[i] = 0;
        for (unsigned long j = 0; j < entry_per_index; j++) {
            tag_array[i][j] = 0;
            cnt_array[i][j] = 0;
            valid_array[i][j] = true;
        }
    }
    // whether victim cache enabled?
    if (victim_cache_enabled == false) return;
    // allocate spaces for victim cache
    cout << "# Allocating spaces for vicitm cache..." << endl;
    victim_tag = (unsigned long *)malloc(victim_block_num * sizeof(unsigned long));
    victim_cnt = (unsigned long *)malloc(victim_block_num * sizeof(unsigned long));
    victim_valid = (bool *)malloc(victim_block_num * sizeof(bool));
    min_cnt_col_victim = 0;
    for (int k = 0; k < victim_block_num; k++) {
        victim_tag[k] = 0;
        victim_cnt[k] = 0;
        victim_valid[k] = true;
    }
    cout << "# Allocating spaces finished." << endl;
}

vector<struct FileLine> CacheClass::readFile(string filename) {
    vector<struct FileLine> filelines;
    filelines.push_back(FileLine());
    struct FileLine temp;

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
        temp.addr = inst_addr;
        temp.opcode = inst_opcode;
        temp.offset = inst_offset;
        filelines.push_back(temp);
        line_num++;
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

unsigned long CacheClass::computeIndex(string addr) {
    string str;
    str.assign(addr, cache_tag, cache_index);
    unsigned long ret = 0;
    for (unsigned long i = 0; i < cache_index; i++) {
        if (str[str.length() - i - 1] == '1') {
            ret += pow(2, i);
        }
    }
    return ret;
}

unsigned long CacheClass::computeTag(string addr) {
    string str;
    str.assign(addr, 0, cache_tag);
    unsigned long ret = 0;
    for (unsigned long i = 0; i < cache_tag; i++) {
        if (str[str.length() - i - 1] == '1') {
            ret += pow(2, i);
        }
    }
    return ret;
}

unsigned long CacheClass::computeOffset(string addr) {
    string str;
    str.assign(addr, cache_tag + cache_index, cache_offset);
    unsigned long ret = 0;
    for (unsigned long i = 0; i < cache_offset; i++) {
        if (str[str.length() - i - 1] == '1') {
            ret += pow(2, i);
        }
    }
    return ret;
}

unsigned long CacheClass::computeAddr(string addr) {
    string str;
    str.assign(addr, 0, cache_tag + cache_index);
    unsigned long ret = 0;
    for (unsigned long i = 0; i < cache_index + cache_tag; i++) {
        if (str[str.length() - i - 1] == '1') {
            ret += pow(2, i);
        }
    }
    return ret;
}

int CacheClass::isHit(struct FileLine fileline) {
    unsigned long dest_idx = computeIndex(fileline.addr);
    unsigned long dest_tag = computeTag(fileline.addr);
    unsigned long dest_tag_full = (dest_tag << cache_index) | dest_idx;

    // hit in cache
    for (unsigned long j = 0; j < entry_per_index; j++) {
        if (valid_array[dest_idx][j] == false && tag_array[dest_idx][j] == dest_tag) {
            cnt_array[dest_idx][j] += 1;
            return 1;
        }
    }
    // miss in cache, hit in victim cache
    for (int j = 0; j < victim_block_num; j++) {
        if (victim_valid[j] == false && victim_tag[j] == dest_tag_full) {
        #ifdef DEBUG
            cout << "Victim cache useful!" << endl;
        #endif
            victim_cnt[j] += 1;
            hit_victim_col = j;
            return 2;
        }
    }
    // miss in both cache and victim cache
    return 0;
}

void CacheClass::clearVictimLine(int idx) {
    victim_cnt[idx] = 0;
    victim_tag[idx] = 0;
    victim_valid[idx] = true;
}

unsigned long CacheClass::updateMinCacheline(unsigned long idx) {
    unsigned long ret = 0;  // the col index of the min element cnt @ idx in cache
    unsigned long tmp = LONG_MAX;
    for (unsigned long j = 0; j < entry_per_index; j++) {
        if (tmp > cnt_array[idx][j]) { // find new min, update
            tmp = cnt_array[idx][j];
            ret = j;
        }
    }
    return ret;
}

unsigned int CacheClass::updateMinVictimline() {
    unsigned int ret = 0;  // the col index of the min element cnt @ idx in victim cache
    unsigned long tmp = LONG_MAX;
    for (unsigned long j = 0; j < victim_block_num; j++) {
        if (tmp > victim_cnt[j]) { // find new min, update
            tmp = victim_cnt[j];
            ret = j;
        }
    }
    return ret;
}



void CacheClass::insertLine(struct FileLine fileline) {
    unsigned long src_idx = computeIndex(fileline.addr);
    unsigned long src_tag = computeTag(fileline.addr);
    unsigned long dest_tag_full = (src_tag << cache_index) | src_idx;
    // unsigned long test_tag = computeAddr(fileline.addr);

    // hit in cache, do nothing
    if (isHit(fileline) == 1) return;
    
    min_cnt_col_array[src_idx] = updateMinCacheline(src_idx);
    unsigned long tmp_col = min_cnt_col_array[src_idx];
    // miss in cache, hit in victim cache, only when victim cache enabled
    if (isHit(fileline) == 2 && victim_cache_enabled == true) {
        // cacheline available, insert line to cache, delete in victim cache
        for (unsigned long j = 0; j < entry_per_index; j++) {
            if (valid_array[src_idx][j] == true) {
                tag_array[src_idx][j] = src_tag;
                cnt_array[src_idx][j] = victim_cnt[hit_victim_col];
                valid_array[src_idx][j] = false;
                clearVictimLine(j);
                return;
            }
        }
        // conflict miss in cache, evicted line -> victim cache, replace evicted line in cache
        evicted_tag = tag_array[src_idx][(min_cnt_col_array[src_idx])];
        evicted_tag = (evicted_tag << cache_index) | src_idx;   // to fit in victim tag
        evicted_cnt = cnt_array[src_idx][(min_cnt_col_array[src_idx])];
        // put hit line into cache
        tag_array[src_idx][(min_cnt_col_array[src_idx])] = src_tag;
        cnt_array[src_idx][(min_cnt_col_array[src_idx])] = victim_cnt[hit_victim_col];
        valid_array[src_idx][(min_cnt_col_array[src_idx])] = false;
        // put evicted line into victim cache
        victim_tag[hit_victim_col] = evicted_tag;
        victim_cnt[hit_victim_col] = evicted_cnt;
        victim_valid[hit_victim_col] = false;
        return;
    }
    // miss in both cache and victim cache
    if (isHit(fileline) == 0) {
        miss_num += 1;
        // cacheline available, insert line to cache
        for (unsigned long j = 0; j < entry_per_index; j++) {
            if (valid_array[src_idx][j] == true) {
                tag_array[src_idx][j] = src_tag;
                cnt_array[src_idx][j] = victim_cnt[hit_victim_col];
                valid_array[src_idx][j] = false;
                return;
            }
        }
        // conflict miss in cache, evicted line -> victim cache, replace evicted line in cache
        if (victim_cache_enabled == false) {    // put new line into cache
            tag_array[src_idx][tmp_col] = src_tag;
            cnt_array[src_idx][tmp_col] = 1;
            valid_array[src_idx][tmp_col] = false;
            return;
        }else {
            // get evicted line
            evicted_tag = tag_array[src_idx][tmp_col];
            evicted_tag = (evicted_tag << cache_index) | src_idx;   // to fit in victim tag
            evicted_cnt = cnt_array[src_idx][tmp_col];
            // put new line into cache
            tag_array[src_idx][tmp_col] = src_tag;
            cnt_array[src_idx][tmp_col] = victim_cnt[hit_victim_col];
            valid_array[src_idx][tmp_col] = false;
            // if available space in victim cache
            for (int j = 0; j < victim_block_num; j++) {
                if (victim_valid[j] == true) {
                    victim_tag[j] = evicted_tag;
                    victim_cnt[j] = evicted_cnt;
                    victim_valid[j] = false;
                    return;
                }
            }
            // no available space in victim line, LRU
            min_cnt_col_victim = updateMinVictimline();
            victim_tag[min_cnt_col_victim] = evicted_tag;
            victim_cnt[min_cnt_col_victim] = evicted_cnt;
            victim_valid[min_cnt_col_victim] = false;
            return;
        }
    }
}

double CacheClass::computeMissRate(long l, long miss_num) {
    return 100 * double(miss_num) / double(l);
}

double CacheClass::computeHitRate(long l, long miss_num) {
    return 100 * (double(l) - double(miss_num)) / double(l);
}

void CacheClass::Applications() {
    cout << "# Reading trace file..." << filename << endl;
    vector<struct FileLine> filelines = readFile(filename);
    cout << "# Total lines in the file: " << line_num << endl;
    initArch();
    int v = 1;

    while (v < filelines.size()) {
        filelines[v].addr = convertAddr(filelines[v].addr);
        insertLine(filelines[v]);
        v++;
    }
    float miss_rate = computeMissRate(line_num, miss_num);
    float hit_rate = computeHitRate(line_num, miss_num);
    cout << "# Miss num: " << miss_num << endl;
    cout << "# Lines in the file: " << line_num << endl;
    cout << "# Miss rate of file " << filename << " is: " << miss_rate << "%" << endl;
    cout << "# Hit rate of file " << filename << " is: " << hit_rate << "%" << endl;
#ifdef DEBUG
    cout << "victim cache: " << endl;
    for (int i = 0; i <victim_block_num; i++) {
        cout << i << ": " << victim_tag[i] << endl;
    }
#endif
    exit (EXIT_SUCCESS);
}