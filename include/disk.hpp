#pragma once
#include <fstream>
#include <vector>
#include <string>
#include<list>
#include<unordered_map>

using ll=long;
class Disk{
    private:
        std::fstream file;
        std::string filename;
        bool cache_enabled;
        int cache_capacity;
        ll cache_hits;
        ll cache_misses;
        std::unordered_map<ll,std::vector<char>> cache_map;
        std::list<ll> cache_lru;
        
        void initialize_disk();
        void read_block_uncached(int block_num,std::vector<char> & buffer);
        void write_block_uncached(int block_num, const std::vector<char>& buffer);
        void clear_touch(int block_num);
        void cache_put(int block_num,const std::vector<char>& data);
        
        public:
        static const int BLOCK_SIZE=4096;
        static const int TOTAL_BLOCKS=1024;
        Disk(const std::string& filename);
        void read_block(int block_num,std::vector<char> & buffer);
        void write_block(int block_num,const std::vector<char>& buffer); 
        std::string &path();
        void close_disk();
        void reopen_disk();

        void clear_cache();
        void set_cache_enabled(bool enabled);
        bool is_cache_enabled();
        void cache_stats();
        void set_cache_capacity(int capacity);
};