#include "directory.hpp"
#include <cstring>
#include <iostream>
using std::vector;
using std::cout;
//constructor
Directory::Directory(Disk &disk,int block_num):disk(disk),block_num(block_num){}

// load directory from disk
void Directory::load(){
    vector<char> buffer;
    disk.read_block(block_num,buffer);
    memcpy(entries,buffer.data(),sizeof(entries));
}

//save directory to disk
void Directory::save(){
    vector<char> buffer(Disk::BLOCK_SIZE,0);
    memcpy(buffer.data(),entries,sizeof(entries));
    disk.write_block(block_num,buffer);
}

//adding file
void Directory::add_entry(char* name,int inode_id){
    for(int i=0;i<64;i++){
        if(entries[i].inode_id=='\0'){
            strcpy(entries[i].name,name);
            entries[i].inode_id=inode_id;
            return;
        }
    }
}

//finding file
int Directory::find_entry(char *name){
    for(int i=0;i<64;i++){
        if(strcmp(entries[i].name,name)=='\0'){
            return entries[i].inode_id;
        }
    }
    return -1;
}

//listing files

void Directory::list(){
    cout<<"files:\n";
    for(int i=0;i<64;i++){
        if(entries[i].inode_id!='\0'){
            cout<<entries[i].name<<"\n";
        }
    }
}