#include "fs.hpp"
#include "inode.hpp"
#include "bitmap.hpp"
#include "directory.hpp"
#include <iostream>
#include <cstring>
//#include<experimental/filesystem>
#include<fstream>
#include<sstream>
#include<sys/stat.h>
#include<iomanip>
#include <direct.h>
#include<chrono>

using std::vector;
using std::cout;
using std::cerr;
using std::string;
//namespace fsys=std::filesystem;

static const int MAX_DIRECT_BLOCKS=5;
static const char* SNAP_DIR = "data/snapshots";
static const char* SNAP_INDEX = "data/snapshots/index.txt";

static bool copy_file_binary(const std::string& src, const std::string& dst) {
    std::ifstream in(src, std::ios::binary);
    if (!in) return false;
    std::ofstream out(dst, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << in.rdbuf();
    return out.good();
}

static int blocks_for_size(int size){
    if(size<=0) return 0;
    int n=(size+Disk::BLOCK_SIZE-1)/Disk::BLOCK_SIZE;
    return n>MAX_DIRECT_BLOCKS?MAX_DIRECT_BLOCKS:n;
}

static int max_file_bytes(){
    return MAX_DIRECT_BLOCKS*Disk::BLOCK_SIZE;
}
//constructor
FileSystem::FileSystem(Disk& disk): disk(disk),root_dir_block(0),current_dir_inode(0){}

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
    int block=get_dir_block(current_dir_inode);
    if(block<0){
        cerr<<"invalid current directory\n";
        return false;
    }
    Directory dir(disk,block);
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
    int block=get_dir_block(current_dir_inode);
    if(block<0)return;

    InodeTable it(disk,sb.inode_start);
    Directory dir(disk,block);
    dir.load();
    dir.list();
}

bool FileSystem::write_file(const char* name,const string &data){
    InodeTable it(disk,sb.inode_start);
    int bitmap_block=sb.inode_start+sb.inode_blocks;
    Bitmap bm(disk,bitmap_block,sb.total_blocks);
    bm.load();
    int block=get_dir_block(current_dir_inode);
    if(block<0){
        cerr<<"invalid current directory\n";
        return false;
    }
    Directory dir(disk,block);
    dir.load();

    int inode_id=dir.find_entry(name);
    if(inode_id<0){
        cerr<<"file not found\n";
        return false;
    }

    Inode inode=it.read_inode(inode_id);
    if(inode.is_dir){
        cerr<<"it is a directory not a file\n";
        return false;
    }
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
    cout<<"written "<<n<<" bytes ( "<<new_blocks<<" blocks)\n";
    return true;

}

bool FileSystem::read_file(const char* name,string& out){
    InodeTable it(disk,sb.inode_start);
    int block=get_dir_block(current_dir_inode);
    if(block<0){
        cerr<<"invalid current directory\n";
        return false;
    }
    Directory dir(disk,block);
    dir.load();
    int inode_id=dir.find_entry(name);
    if(inode_id==-1){
        cerr<<"file not found\n";
        return false;
    }
    Inode inode=it.read_inode(inode_id);
    if(inode.is_dir){
        cerr<<"it is a directory not a file\n";
        return false;
    }
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
    int block=get_dir_block(current_dir_inode);
    if(block<0){
        cerr<<"invalid current directory\n";
        return false;
    }
    Directory dir(disk,block);
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

int FileSystem::get_dir_block(int inode_id){
    InodeTable it(disk,sb.inode_start);
    Inode in=it.read_inode(inode_id);
    if(!in.used || !in.is_dir || in.direct_blocks[0]==0)return -1;
    return in.direct_blocks[0];
}

bool FileSystem::make_dir(const char* name){
    int parent_block=get_dir_block(current_dir_inode);
    if(parent_block<0) return false;

    InodeTable it(disk,sb.inode_start);
    Bitmap bm(disk,bitmap_block(),sb.total_blocks);
    bm.load();
    Directory dir(disk,parent_block);
    dir.load();
if(dir.find_entry(name)>=0){
    cerr<<"name exists already..\n";
    return false;
}
int inode_id=it.allocate_inode();
if(inode_id<0){
    cerr<<"no free inode\n";
    return false;
}
int block=bm.allocate_block();
if(block<0){
    it.free_inode(inode_id);
    cerr<<"no free blocks\n";
    return false;
}
Inode new_dir;
memset(&new_dir,0,sizeof(new_dir));
new_dir.used=1;
new_dir.is_dir=1;
new_dir.direct_blocks[0]=block;
it.write_inode(inode_id,new_dir);
DirEntry empty[64];
memset(empty,0,sizeof(empty));
vector<char> buf(Disk::BLOCK_SIZE,0);
memcpy(buf.data(),empty,sizeof(empty));
disk.write_block(block,buf);

if(!dir.add_entry(name,inode_id)){
    bm.free_block(block);
    it.free_inode(inode_id);
    cerr<<"directory full\n";
    return false;
}
dir.save();

cout<<"directory created: "<<name<<" (inode "<<inode_id<<" )\n";
return true;
}

bool FileSystem::change_dir(const char* name){
    if(strcmp(name,"..")==0)return change_dir_up();
    int block=get_dir_block(current_dir_inode);
    if(block<0)return false;
    InodeTable it(disk,sb.inode_start);
    Directory dir(disk,block);
    dir.load();

    int target_id=dir.find_entry(name);
    if(target_id<0){
        cerr<<"directory not found\n";
        return false;
    }

    Inode target=it.read_inode(target_id);
    if(!target.is_dir){
        cerr<<"not a dir\n";
        return false;
    }

    current_dir_inode=target_id;
    cout<<"changed dir\n";
    return true;
}


bool FileSystem::change_dir_up(){
    if(current_dir_inode==0){
        cout<<"current dir is root\n";
        return true;
    }
    current_dir_inode=0;
    cout<<"changed to root\n";
    return true;
} 

void FileSystem::pwd(){
if(current_dir_inode==0)cout<<"/ (root,inode 0)\n";
else cout<<"current dir inode: "<<current_dir_inode<<"\n";
}

bool FileSystem::snapshot_save(string &name){
    _mkdir("data");
    _mkdir(SNAP_DIR);
    if(name.empty()){
        cerr << "empty input..\n";
        return false;
    }

    string src = disk.path();
    string dst = string(SNAP_DIR) + "/" + name + ".img";

    disk.close_disk();
    bool ok = copy_file_binary(src, dst);
    disk.reopen_disk();

    if(!ok){
        cerr << "snapshot save failed\n";
        return false;
    }

    std::ofstream idx(SNAP_INDEX, std::ios::app);
    idx << name << "\n";
    cout << "snapshot saved: " << name << "\n";
    return true;
}

bool FileSystem::snapshot_load(string &name){
    if(name.empty()){
        cerr << "empty input..\n";
        return false;
    }

    string src = string(SNAP_DIR) + "/" + name + ".img";
    if(!std::ifstream(src).good()){
        cerr << "snapshot not found\n";
        return false;
    }

    string dst = disk.path();
    disk.close_disk();
    bool ok = copy_file_binary(src, dst);
    disk.reopen_disk();

    if(!ok){
        cerr << "snapshot load failed\n";
        return false;
    }

    load();
    current_dir_inode = 0;
    cout << "snapshot loaded: " << name << "\n";
    return true;
}

void FileSystem::snapshot_list(){
    std::ifstream idx(SNAP_INDEX);
    if(!idx.good()){
        cout << "no snapshots\n";
        return;
    }

    cout << "snapshots:\n";
    string name;
    while(idx >> name){
        cout << " " << name << "\n";
    }
}

void FileSystem::viz_map(){
    int bmap=sb.inode_start+sb.inode_blocks;
    cout<<"list...\n";
    cout<<" superblock: 0\n";
    cout<<" inode blocks: "<<sb.inode_start<<" .. "<<(sb.inode_start+sb.inode_blocks-1)<<"\n";
    cout<<" bitmap block: "<<bmap<<"\n";
    cout<<" data blocks: "<<sb.data_start<<" .. "<<(sb.total_blocks-1)<<"\n";
}

void FileSystem::viz_bitmap(){
    Bitmap bm(disk,bitmap_block(),sb.total_blocks);
    bm.load();
    bm.debug();
}

void FileSystem::viz_inode(int inode_id){
    if(inode_id<0 || inode_id>=sb.total_inodes){
        cerr<<"invalid inode id\n";
        return ;
    }
    InodeTable it(disk,sb.inode_start);
    Inode in=it.read_inode(inode_id);
    cout<<"inode "<<inode_id<<":\n";
    cout<<"used: "<<in.used<<"\n";
    cout<<"is_dir: "<<in.is_dir<<"\n";
    cout<<"size: "<<in.size<<"\n";
    cout<<"direct blocks: ";
    for(int i=0;i<5;i++)cout<<in.direct_blocks[i]<<" ";
    cout<<"\n";
    return ;
}

void FileSystem::print_tree_rec(Disk& disk, int inode_start, int inode_id, const string &rep){    InodeTable it(disk,inode_start);
    Inode dir_inode=it.read_inode(inode_id);
    if(!dir_inode.is_dir|| dir_inode.direct_blocks[0]==0)return;

    Directory dir(disk,dir_inode.direct_blocks[0]);
    dir.load();
    auto ents=dir.get_entries();
    for(auto&e:ents){
        Inode n=it.read_inode(e.inode_id);
        cout<<rep<<"|-- "<<e.name;
        if(n.is_dir)cout<<"/(inode "<<e.inode_id<<", size "<<n.size<<")\n";
        else cout<<"(inode "<<e.inode_id<<", size "<<n.size<<" )\n";
        if(n.is_dir)print_tree_rec(disk,inode_start,e.inode_id,rep+"| ");

    }
}

void FileSystem::viz_tree(){
    cout<<"/ (inode 0)\n";
    print_tree_rec(disk,sb.inode_start,0,"");
}

void FileSystem::bench_read(std::string &name,int iter){
if(iter<1){
    cerr<<"iterations should be +ve number\n";
    return;
}
auto st=std::chrono::high_resolution_clock::now();
for(int i=0;i<iter;i++){
    string out;
    if(!read_file(name.c_str(),out)){
        cerr<<"failed to read\n";
        return;}
}
auto end=std::chrono::high_resolution_clock::now();
auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(end-st).count();
cout<<"read "<<name<<" "<<iter<<" times in "<<ms<<"ms\n";
cout<<"average time per read: "<<(double)ms/iter<<"ms\n";
disk.cache_stats();
}