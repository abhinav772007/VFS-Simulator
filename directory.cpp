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
bool Directory::add_entry(const char* name,int inode_id){
    if(inode_id<0)return false;
    if(find_entry(name)>=0)return false;
    for(int i=0;i<64;i++){
        if(entries[i].inode_id==0){
            strncpy(entries[i].name,name,sizeof(entries[i].name)-1);
            entries[i].name[sizeof(entries[i].name)-1]='\0';
            entries[i].inode_id=inode_id;
            return true;
        }
    }
    return false;
}

//finding file
int Directory::find_entry(const char *name){
    for(int i=0;i<64;i++){
        if(entries[i].inode_id!=0 && strcmp(entries[i].name,name)==0){
            return entries[i].inode_id;
        }
    }
    return -1;
}

//remove entry
int Directory::remove_entry(const char* name){
    for(int i=0;i<64;i++){
        if(entries[i].inode_id!=0 && strcmp(entries[i].name,name)==0){
            int inode_id=entries[i].inode_id;
            entries[i].inode_id=0;
            memset(entries[i].name,0,sizeof(entries[i].name));
            return inode_id;
        }
    }
    return -1;
}

//listing files

void Directory::list(){
    cout<<"files:\n";
    bool check=false;
    for(int i=0;i<64;i++){
        if(entries[i].inode_id!=0){
            cout<<entries[i].name<<"\n";
            check=true;
        }
    }
    if(!check)cout<<"empty :( \n";
}

vector<DirEntry> Directory::get_entries(){
vector<DirEntry> out;
for(int i=0;i<64;i++){
    if(entries[i].inode_id!=0)out.push_back(entries[i]);
}
return out;
}