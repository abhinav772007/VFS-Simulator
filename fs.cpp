#include "fs.hpp"
#include "inode.hpp"
#include <iostream>
#include <cstring>
using std::vector;
using std::cout;
//constructor
FileSystem::FileSystem(Disk& disk): disk(disk){}

void FileSystem::write_superblock(){
vector<char> block(Disk::BLOCK_SIZE,0);
memcpy(block.data(),&sb,sizeof(superblock));
disk.write_block(0,block);
}
void FileSystem::read_superblock(){
    vector<char> block;
    disk.read_block(0,block);
    memcpy(&sb,block.data(),sizeof(superblock));
}
void FileSystem::format(){
cout<<"Formatting filesystem...\n";
sb.total_blocks=Disk::TOTAL_BLOCKS;
sb.block_size=Disk::BLOCK_SIZE;
sb.inode_start=1;
sb.inode_blocks=10;
sb.data_start=sb.inode_start+sb.inode_blocks;
sb.total_inodes=128;
write_superblock();
InodeTable it(disk,sb.inode_start);
Inode empty;
 memset(&empty,0,sizeof(Inode));
 for(int i=0;i<sb.total_inodes;i++){
    it.write_inode(i,empty);
 }
cout<<"Filesystem formatted successfully\n";
}
void FileSystem::load(){
read_superblock();
cout<<"Filesystem loaded successfully\n";

}
void FileSystem::debug(){
cout<<"filesystem info:\n";
cout<<"Total blocks: "<<sb.total_blocks<<"\n";
cout<<"Block size: "<<sb.block_size<<"\n";
cout<<"Inode start: "<<sb.inode_start<<"\n";
cout<<"Inode blocks: "<<sb.inode_blocks<<"\n";
cout<<"Data start: "<<sb.data_start<<"\n";
cout<<"Total inodes: "<<sb.total_inodes<<"\n";

}