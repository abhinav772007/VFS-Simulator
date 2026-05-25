#pragma once
#include "disk.hpp"

struct superblock{
    int total_blocks;
    int block_size;
    int inode_start;
    int inode_blocks;
    int data_start;
    int total_inodes;
};

class FileSystem{
    private:
        Disk &disk;
        superblock sb;
        void write_superblock();
        void read_superblock();
        void refresh_root_block();
        int root_dir_block;
    
    public:
        FileSystem(Disk &disk);
        
        void format();
        void load();
        void debug();
        const superblock& get_superblock() const{ return sb;}
        bool create_file(const char* name);
        void list_files();
        bool write_file(const char* name, const std::string& data);
        bool read_file(const char* name,std::string &out);

};
