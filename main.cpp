#include "disk.hpp"
#include "fs.hpp"
#include "inode.hpp"
#include "bitmap.hpp"
#include "directory.hpp"
#include <iostream>
#include<cstring>
using std::vector;
using std::cout;
using std::cin;
using std::cerr;
using std::string;
using std::fstream;

void write_file(Disk &disk,InodeTable &it,Bitmap & bm,Directory& dir){
    char name[32];
    cout<<"enter filename : ";
    cin>>name;
    int inode_id=dir.find_entry(name);
    if(inode_id==-1){
        cout<<"file not found\n";
        return;
    }
    Inode inode=it.read_inode(inode_id);
    int block;
    if(inode.direct_blocks[0]!=0){
        block=inode.direct_blocks[0];
    }
    else{
        block=bm.allocate_block();
        if(block<0){
            cout<<"no free blocks\n";
            return;
        }
        inode.direct_blocks[0]=block;
    }
    cout<<"enter data : ";
    cin.ignore();
    string data;
    std::getline(cin,data);
    size_t n=data.size();
    vector<char> buffer(Disk::BLOCK_SIZE,0);
    memcpy(buffer.data(),data.c_str(),n);
    disk.write_block(block,buffer);
    inode.size=(int)n;
    inode.used=1;
    inode.is_dir=0;
    it.write_inode(inode_id,inode);
    cout<<"written...\n";
}
void read_file(Disk &disk,InodeTable &it,Directory& dir){
    char name[32];
    cout<<"\n\n\n";
    cout<<"-----------------------------\n";
    cout<<"enter filename : ";
    cin>>name;
    int inode_id=dir.find_entry(name);
    if(inode_id==-1){
        cout<<"file not found\n";
        return;
    }
    Inode inode=it.read_inode(inode_id);
    if(inode.direct_blocks[0]==0){
        cout<<"file is empty...\n";
        return;
    }
    vector<char> buffer;
    disk.read_block(inode.direct_blocks[0],buffer);
    string data(buffer.begin(),buffer.begin()+inode.size);
    cout<<"file content: "<<data<<"\n";
    cout<<"-----------------------------\n";
    cout<<"\n\n";
}
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
        int bitmap_block=sb.inode_start+sb.inode_blocks;
        Bitmap bm(disk,bitmap_block,sb.total_blocks);
        bm.load();
        InodeTable it(disk,sb.inode_start);
        Inode root=it.read_inode(0);
        Directory dir(disk,root.direct_blocks[0]);
        dir.load();

        while(true){
            cout<<"create file ---> 1\n";
            cout<<"list files ---> 2\n";
            cout<<"exit ----> 3\n";
            cout<<"write in file --->4\n";
            cout<<"read from file -->5\n";
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
            else if(op==4){
                write_file(disk,it,bm,dir);
            }
            else if(op==5){
                read_file(disk,it,dir);
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