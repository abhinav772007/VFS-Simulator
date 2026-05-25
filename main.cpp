#include "disk.hpp"
#include "fs.hpp"
#include "inode.hpp"
#include "bitmap.hpp"
#include "directory.hpp"
#include <iostream>
using std::vector;
using std::cout;
using std::cin;
using std::cerr;
using std::string;
using std::fstream;

int main(){
    try{
        Disk disk("disk.img");
        FileSystem fs(disk);
        int choice;
        cout<<"1. Format filesystem\n";
        cout<<"2. Load filesystem\n";
        cin>>choice;
        if(choice==1){
            fs.format();
        }
        else{
            fs.load();
        }
        fs.debug();
        Bitmap bm(disk,11,1024);
        bm.load();
        InodeTable it(disk,1);
        Inode root=it.read_inode(0);
        Directory dir(disk,root.direct_blocks[0]);
        dir.load();

        int op;
        cout<<"enter your opt:\n";
        cin>>op;
        if(op==1){
            char name[32];
            cout<<"enter filename: \n";
            cin>>name;
            int inode_id=it.allocate_inode();
            dir.add_entry(name,inode_id);
            dir.save();
            cout<<"file created...\n";
        }
        else{
            dir.list();
        }
        int b=bm.allocate_block();
        cout<<"Allocated block: "<<b<<"\n";
        bm.debug();


        // InodeTable it(disk,1);
        // int inode_id=it.allocate_inode();
        // cout<<"Allocated inode id: "<<inode_id<<"\n";
        // Inode inode=it.read_inode(inode_id);
        // cout<<"used: "<<inode.used<<"\n";

        // vector<char> data(Disk::BLOCK_SIZE,0);
        // string msg="Hello,this is abhinav!";
        // std::copy(msg.begin(),msg.end(),data.begin());
        // disk.write_block(5,data);
        // vector<char> read_data;
        // disk.read_block(5,read_data);
        // string output(read_data.begin(),read_data.end());
        // cout<<"Written message: "<<msg<<"\n";
        // cout<<"output message: "<<output<<"\n";
    }
    catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<"\n";
        return 1;
    }
    return 0;

}