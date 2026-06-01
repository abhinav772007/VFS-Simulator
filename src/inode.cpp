#include "inode.hpp"
#include <cstring>
using std::vector;


//constructor
InodeTable::InodeTable(Disk &disk,int inode_start):disk(disk),inode_start(inode_start){}    
// write inode to disk
void InodeTable::write_inode(int index,Inode &inode){
    int block=inode_start+(index*sizeof(Inode))/Disk::BLOCK_SIZE;
    vector<char> buffer;
    disk.read_block(block,buffer);
    int offset=(index*sizeof(Inode))%Disk::BLOCK_SIZE;
    memcpy(buffer.data()+offset,&inode,sizeof(Inode));
    disk.write_block(block,buffer);
}
// read inode from disk
Inode InodeTable::read_inode(int index){
    int block=inode_start+(index*sizeof(Inode))/Disk::BLOCK_SIZE;
    vector<char> buffer;
    disk.read_block(block,buffer);
    int offset=(index*sizeof(Inode))%Disk::BLOCK_SIZE;
    Inode inode;
    memcpy(&inode,buffer.data()+offset,sizeof(Inode));
    return inode;
}
// find free inode
int InodeTable::allocate_inode(){
    for(int i=0;i<128;i++){ //total_inodes is the total number of inodes in the filesystem
        Inode inode=read_inode(i);
        if(!inode.used){
            inode.used=1;
            write_inode(i,inode);
        
            return i;
        }
    }
    return -1; //no free inode found
}
// free inode
void InodeTable::free_inode(int index){
    Inode inode=read_inode(index);
    inode.used=0;
    write_inode(index,inode);
}