#include "bitmap.hpp"
#include <iostream>
using std::vector;
using std::cout;
// constructor
Bitmap::Bitmap(Disk& disk,int bitmap_block,int total_blocks)
    :disk(disk),bitmap_block(bitmap_block),total_blocks(total_blocks){
        bits.resize(total_blocks,false);
    }

//load bitmap from disk

void Bitmap::load(){
    vector<char> buffer;
    disk.read_block(bitmap_block,buffer);
    for(int i=0;i<total_blocks;i++){
        bits[i]=buffer[i];
    }
}

//save bitmap to diskk
void Bitmap::save(){
    vector<char> buffer(Disk::BLOCK_SIZE,0);
    for(int i=0;i<total_blocks;i++){
        buffer[i]=bits[i];
    }
    disk.write_block(bitmap_block,buffer);
}

//allocate free block

int Bitmap::allocate_block(){
    for(int i=0;i<total_blocks;i++){
        if(!bits[i]){
            bits[i]=true;
            save();
            return i;
        }
    }
    return -1;
}

//free block

void Bitmap::free_block(int index){
    bits[index]=false;
    save();
}

//debuging and print

void Bitmap::debug(){
    cout<<"Bitmap: ";
    for(int i=0;i<20;i++){
        cout<<bits[i]<<" ";
    }
    cout<<"\n";
}