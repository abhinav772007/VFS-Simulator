#pragma once
# include "disk.hpp"
#include <vector>

class Bitmap{
    private:
        Disk& disk;
        int bitmap_block;
        int total_blocks;
        std::vector<bool> bits;
    public:
        Bitmap(Disk& disk,int bitmap_block,int total_blocks);
        void load();
        void save();
        int allocate_block(); //finding for free block
        void free_block(int index);
        void debug();
};
