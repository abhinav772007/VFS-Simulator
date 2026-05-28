#pragma once
#include <fstream>
#include <vector>
#include <string>

class Disk{
    private:
        std::fstream file;
        std::string filename;
        void initialize_disk();
    public:
        static const int BLOCK_SIZE=4096;
        static const int TOTAL_BLOCKS=1024;
        Disk(const std::string& filename);
        void read_block(int block_num,std::vector<char> & buffer);
        void write_block(int block_num,const std::vector<char>& buffer); 
        std::string &path();
        void close_disk();
        void reopen_disk();
};