#include "fs.hpp"
#include "inode.hpp"
#include "bitmap.hpp"
#include "directory.hpp"
#include <iostream>
#include <cstring>
using std::vector;
using std::cout;
using std::cerr;
using std::string;

static const int MAX_DIRECT_BLOCKS=5;

static int blocks_for_size(int size){
    if(size<=0) return 0;
    int n=(size+Disk::BLOCK_SIZE-1)/Disk::BLOCK_SIZE;
    return n>MAX_DIRECT_BLOCKS?MAX_DIRECT_BLOCKS:n;
}

static int max_file_bytes(){
    return MAX_DIRECT_BLOCKS*Disk::BLOCK_SIZE;
}
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

void FileSystem::refresh_root_block(){
    InodeTable it(disk,sb.inode_start);
    Inode root=it.read_inode(0);
    root_dir_block=root.direct_blocks[0];
}
void FileSystem::format(){
cout<<"Formatting filesystem...\n";
sb.total_blocks=Disk::TOTAL_BLOCKS;
sb.block_size=Disk::BLOCK_SIZE;
sb.inode_start=1;
sb.inode_blocks=10;
int bitmap_block=sb.inode_start+sb.inode_blocks;
sb.data_start=sb.inode_start+sb.inode_blocks+1;
sb.total_inodes=128;
write_superblock();

InodeTable it(disk,sb.inode_start);
Inode empty;
 memset(&empty,0,sizeof(Inode));
 for(int i=0;i<sb.total_inodes;i++){
    it.write_inode(i,empty);
 }
 
 Bitmap bm(disk,bitmap_block,sb.total_blocks); //init bitmap
for(int i=0;i<sb.data_start;i++){
    bm.allocate_block();
}
bm.load();
int root_block=bm.allocate_block();
//InodeTable it(disk,sb.inode_start);
Inode root;
memset(&root,0,sizeof(Inode));
root.used=1;
root.is_dir=1;
root.direct_blocks[0]=root_block;
it.write_inode(0,root);
//initialize dir block
DirEntry empty_entries[64];
memset(empty_entries,0,sizeof(empty_entries));
vector<char> buffer(Disk::BLOCK_SIZE,0);
memcpy(buffer.data(),empty_entries,sizeof(empty_entries));
disk.write_block(root_block,buffer);
root_dir_block=root_block;
cout<<"Filesystem formatted successfully\n";
}
void FileSystem::load(){
read_superblock();
refresh_root_block();
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
bool FileSystem::create_file(const char* name){
    InodeTable it(disk,sb.inode_start);
    Directory dir(disk,root_dir_block);
    dir.load();

    int inode_id=it.allocate_inode();
    if(inode_id<0){
        cerr<<"sorry,no free inode\n";
        return false;
    }
    Inode file_inode;
    memset(&file_inode,0,sizeof(file_inode));
    file_inode.used=1;
    file_inode.is_dir=0;
    file_inode.size=0;
    it.write_inode(inode_id,file_inode);

    if(!dir.add_entry(name,inode_id)){
        it.free_inode(inode_id);
        cerr<<"could not add entry(duplicate entry or dir full)..\n";
        return false;
    }
    dir.save();
    cout<<"file created -> "<<name <<" (inode "<<inode_id<<")\n";
    return true;
}

void FileSystem::list_files(){
    Directory dir(disk,root_dir_block);
    dir.load();
    dir.list();
}

bool FileSystem::write_file(const char* name,const string &data){
    InodeTable it(disk,sb.inode_start);
    int bitmap_block=sb.inode_start+sb.inode_blocks;
    Bitmap bm(disk,bitmap_block,sb.total_blocks);
    bm.load();
    Directory dir(disk,root_dir_block);
    dir.load();

    int inode_id=dir.find_entry(name);
    if(inode_id<0){
        cerr<<"file not found\n";
        return false;
    }

    Inode inode=it.read_inode(inode_id);
    size_t n=data.size();
    int maxby=max_file_bytes();
    if((int)n>maxby){
        cerr<<"file too large.. (max bytes are "<<maxby<<"),cutting extra size.\n";
        n=maxby;
    }
    int new_blocks=blocks_for_size((int)n);
    int old=blocks_for_size(inode.size);

    //write 
    for(int i=0;i<new_blocks;i++){
        if(inode.direct_blocks[i]==0){
         int block=bm.allocate_block();
        if(block<0){
            cerr<<"no free blocks\n";
            return false;
        }
        inode.direct_blocks[i]=block;
        }
        vector<char> buffer(Disk::BLOCK_SIZE,0);
        size_t offset=(size_t)i*Disk::BLOCK_SIZE;
        size_t chunk=n-offset;
        if(chunk>Disk::BLOCK_SIZE)chunk=Disk::BLOCK_SIZE;
        if(chunk>0)memcpy(buffer.data(),data.data()+offset,chunk);
        disk.write_block(inode.direct_blocks[i],buffer);
    }
    
   //shrink (free blocks are not used)
   for(int i=new_blocks;i<old;i++){
    if(inode.direct_blocks[i]!=0){
        bm.free_block(inode.direct_blocks[i]);
        inode.direct_blocks[i]=0;
    }
   }
    
    
    inode.size=(int)n;
    inode.used=1;
    inode.is_dir=0;
    it.write_inode(inode_id,inode);
    cout<<"written "<<n<<" bytes ( "<<new_blocks<<" blocks\n";
    return true;

}

bool FileSystem::read_file(const char* name,string& out){
    InodeTable it(disk,sb.inode_start);
    Directory dir(disk,root_dir_block);
    dir.load();
    int inode_id=dir.find_entry(name);
    if(inode_id==-1){
        cerr<<"file not found\n";
        return false;
    }
    Inode inode=it.read_inode(inode_id);
    if(inode.size==0){
        out.clear();
        cout<<"file is empty...\n";
        return true;
    }
    int need=blocks_for_size(inode.size);
    out.clear();
    out.reserve(inode.size);
    for(int i=0;i<need;i++){
        if(inode.direct_blocks[i]==0){
            cerr<<"Invalid inode: data block "<<i<<" not found\n";
            return false;
        }
        vector<char> buffer;
        disk.read_block(inode.direct_blocks[i],buffer);
        int offset=i*Disk::BLOCK_SIZE;
        int rem=inode.size-offset;
        int chunk=rem>Disk::BLOCK_SIZE?Disk::BLOCK_SIZE:rem;
        out.append(buffer.data(),buffer.data()+chunk);
    }
    return true;
    
}
bool FileSystem::delete_file(const char* name){
    InodeTable it(disk,sb.inode_start);
    int bitmap_block=sb.inode_blocks+sb.inode_start;
    Bitmap bm(disk,bitmap_block,sb.total_blocks);
    bm.load();
    Directory dir(disk,root_dir_block);
    dir.load();

    int inode_id=dir.find_entry(name);
    if(inode_id<0){
        cerr<<"file not found\n";
        return false;
    }
    if(inode_id==0){
        cerr<<"cannot delete root !! \n";
        return false;
    }
    Inode inode=it.read_inode(inode_id);
    if(inode.is_dir){
        cerr<<"it is directory,we cant delete it!! \n";
        return false;
    }

    for(int i=0;i<5;i++){
        if(inode.direct_blocks[i]!=0){
            bm.free_block(inode.direct_blocks[i]);
        }
    }
    it.free_inode(inode_id);
    if(dir.remove_entry(name)<0){
        cerr<<"directory entry missing...\n";
        return false;
    }

    dir.save();
    cout<<"deleted "<<name<<" \n";
    return true;

}