#include "cache.h"

#define ADDR_BITS   32

CacheClass::CacheClass(int t, int c, int w, int v, string fn, bool ve) {
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

    miss_num = 0;
    line_num = 0;

    cout << "=======================================================" << endl;
    cout << "# Cache Design: " << endl;
    cout << "# Cache index: " << cache_index << endl;
    cout << "# Cache offset: " << cache_offset << endl;
    cout << "# Cache tag: " << cache_tag << endl;
    cout << "# Cache entry: " << cache_entry << endl;
    cout << "# Victim cache enabled: " << victim_cache_enabled << endl;
    cout << "# Victim cache blocks: " << victim_block_num << endl;
    cout << "=======================================================" << endl;
}

void CacheClass::initArch() {
    // allocate spaces cache (including fully assoc cache), using row ptr 
    cout << "# Allocating spaces for cache..." << endl;
    tag_array = (long **)malloc(actual_index * sizeof(long *));
    cnt_array = (long **)malloc(actual_index * sizeof(long *));
    valid_array = (bool **)malloc(actual_index * sizeof(bool *));
    min_cnt_col_array = (long *)malloc(actual_index * sizeof(long));
    for (int i = 0; i < actual_index; i++) {
        tag_array[i] = (long *)malloc(entry_per_index * sizeof(long));
        cnt_array[i] = (long *)malloc(entry_per_index * sizeof(long));
        valid_array[i] = (bool *)malloc(entry_per_index * sizeof(bool));
        // initialize values
        min_cnt_col_array[i] = 0;
        for (int j = 0; j < entry_per_index; j++) {
            tag_array[i][j] = 0;
            cnt_array[i][j] = 0;
            valid_array[i][j] = true;
        }
    }
    // whether victim cache enabled?
    if (victim_cache_enabled == false) return;
    // allocate spaces for victim cache
    cout << "# Allocating spaces for vicitm cache..." << endl;
    victim_tag = (long *)malloc(victim_block_num * sizeof(long));
    victim_cnt = (long *)malloc(victim_block_num * sizeof(long));
    victim_valid = (bool *)malloc(victim_block_num * sizeof(bool));
    min_cnt_col_victim = 0;
    for (int k = 0; k < victim_block_num; k++) {
        victim_tag[k] = 0;
        victim_cnt[k] = 0;
        victim_valid[k] = 0;
    }
    // TODO: tag cannot be 0 in isHit()
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