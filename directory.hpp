#pragma once
#include "disk.hpp"

struct DirEntry{
    char name[32];
    int inode_id;
};

class Directory{
    private:
        Disk &disk;
        int block_num;
        DirEntry entries[64]; //size 
    public:
        Directory(Disk& disk,int block_num);
        void save();
        void load();
        void add_entry(char* name,int inode_index);
        int find_entry(char *name);
        void list();
};