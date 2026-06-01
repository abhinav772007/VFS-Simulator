#pragma once
#include "disk.hpp"
#include <vector>
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
        bool add_entry(const char* name,int inode_index);
        int remove_entry(const char* name);
        int find_entry(const char *name);
        void list();
        std::vector<DirEntry> get_entries();
};