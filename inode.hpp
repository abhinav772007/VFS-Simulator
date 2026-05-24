#pragma once
#include "disk.hpp"

struct Inode{
    int size;
    int is_dir; //0=file,1=directory
    int direct_blocks[5]; // pointers to data blocks
    int used; //1=used,0=not used
};
class InodeTable{
    private:
        Disk &disk;
        int inode_start;
    public:
        InodeTable(Disk &disk,int inode_start);//constructor
        void write_inode(int index,Inode &inode);
        Inode read_inode(int index);
        int allocate_inode(); //find free inode
        void free_inode(int index);
};