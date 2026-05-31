#pragma once
#include "disk.hpp"
#include<string>
#include<vector>

struct superblock{
    int total_blocks;
    int block_size;
    int inode_start;
    int inode_blocks;
    int data_start;
    int total_inodes;
};

class FileSystem{
    private:
        Disk &disk;
        superblock sb;
       
        int root_dir_block;
        int current_dir_inode; //0 for root
        void write_superblock();
        void read_superblock();
        void refresh_root_block();
        int get_dir_block(int inode_id); //func to get dir
        int bitmap_block() const {return sb.inode_start+sb.inode_blocks;}
    
    public:
        FileSystem(Disk &disk);
        
        void format();
        void load();
        void debug();
        const superblock& get_superblock() const{ return sb;}
        bool create_file(const char* name);
        bool delete_file(const char* name);
        void list_files();
        bool write_file(const char* name, const std::string& data);
        bool read_file(const char* name,std::string &out);
        bool make_dir(const char* name); //sort of mkdir
        bool change_dir(const char* name); // this is sort of cd
        bool change_dir_up(); // sort of cd..
        void pwd(); //cur dir

        //snapshots...
        bool snapshot_save(std::string &name);
        bool snapshot_load(std::string &name);
        void snapshot_list();

        //vizualize
        void viz_map();
        void viz_bitmap();
        void viz_inode(int inode_id);
        void viz_tree();
        void print_tree_rec(Disk &disk, int inode_start, int inode_id, const std::string &rep);
        void bench_read( std::string &name,int iter);
    };
