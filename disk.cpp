#include "disk.hpp"
#include <iostream>

Disk::Disk(const std::string &filename):filename(filename){
    file.open(filename,std::ios::in | std::ios::out | std::ios::binary);
    if(!file){
        std::cout<<"Disk not found :(,\n creating disk\n";
        initialize_disk();
        file.open(filename,std::ios::in | std::ios::out | std::ios::binary);
    }
}
void Disk::initialize_disk(){
std::fstream newfile(filename,std::ios::out | std::ios::binary);
std::vector<char> zero_block(BLOCK_SIZE,0);
for(int i=0;i<TOTAL_BLOCKS;i++){
    newfile.write(zero_block.data(),BLOCK_SIZE);
}
newfile.close();
std::cout<<"Disk initialized successfully\n";
}
void Disk::read_block(int block_num,std::vector<char> & buffer){
    if(block_num<0 || block_num>=TOTAL_BLOCKS){
        throw std::invalid_argument("Invalid block number");
    }
    buffer.resize(BLOCK_SIZE);
    file.seekg(block_num*BLOCK_SIZE,std::ios::beg);
    file.read(buffer.data(),BLOCK_SIZE);
    if(!file){
        throw std::runtime_error("Failed to read block");
    }
}
void Disk::write_block(int block_num,const std::vector<char>& buffer){
    if(block_num<0 || block_num>=TOTAL_BLOCKS){
        throw std::invalid_argument("Invalid block number");
    }
    file.seekp(block_num*BLOCK_SIZE,std::ios::beg);
    file.write(buffer.data(),BLOCK_SIZE);
    file.flush();
    if(!file){
        throw std::runtime_error("Failed to write block");
    }
}