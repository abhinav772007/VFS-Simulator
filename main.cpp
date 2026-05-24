#include "disk.hpp"
#include "fs.hpp"
#include "inode.hpp"
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
        InodeTable it(disk,1);
        int inode_id=it.allocate_inode();
        cout<<"Allocated inode id: "<<inode_id<<"\n";
        Inode inode=it.read_inode(inode_id);
        cout<<"used: "<<inode.used<<"\n";
        
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