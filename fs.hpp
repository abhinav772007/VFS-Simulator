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
    
    public:
        FileSystem(Disk &disk);
        void format();
        void load();
        void debug();


};