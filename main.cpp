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
        cout<<"1. Format filesystem (this erases all the data !!)\n";
        cout<<"2. Load filesystem\n";
        cin>>choice;
        if(choice==1){
            fs.format();
        }
        else if(choice==2){
            fs.load();
        }
        else{
            cerr<<"invalid choice !!\n";
            return 1;
        }
        fs.debug();
        const superblock& sb=fs.get_superblock();
        //int bitmap_block=sb.inode_start+sb.inode_blocks;
        // Bitmap bm(disk,11,1024);
        // bm.load();
        InodeTable it(disk,sb.inode_start);
        Inode root=it.read_inode(0);
        Directory dir(disk,root.direct_blocks[0]);
        dir.load();

        while(true){
            cout<<"create file ---> 1\n";
            cout<<"list files ---> 2\n";
            cout<<"exit ----> 3\n";
            int op;
            cin>>op;
            if(op==3){
                cout<<"exiting.....\n";
                break;}
            if(op==1){
                char name[32];
                cout<<"enter filename : \n";
                cin>>name;
                int inode_id=it.allocate_inode();
                if(inode_id<0){
                    cerr<<"sorry,no free inode\n";
                    continue;
                }
                if(!dir.add_entry(name,inode_id)){
                    cerr<<"could not add entry(duplicate entry or dir full)..\n";
                    continue;
                }
                dir.save();
                cout<<"file created -> "<<name <<" (inode "<<inode_id<<")\n";
            }
            else if(op==2){
                dir.list();
            }
            else{
                cerr<<"invalid option\n";
            }

        }
        


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