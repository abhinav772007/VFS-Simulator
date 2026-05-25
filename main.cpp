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
        

        while(true){
            cout<<"create file ---> 1\n";
            cout<<"list files ---> 2\n";
            cout<<"exit ----> 3\n";
            cout<<"write in file --->4\n";
            cout<<"read from file -->5\n";
            cout<<"delete file --->6\n";
            int op;
            cin>>op;
            if(op==3){
                cout<<"exiting.....\n";
                break;}
            if(op==1){
                char name[32];
                cout<<"enter filename : \n";
                cin>>name;
                fs.create_file(name);
            }
            else if(op==2){
                fs.list_files();
            }
            else if(op==4){
                char name[32];
                cout<<"enter filename: \n";
                cin>>name;
                cin.ignore(10000,'\n');
                string data;
                cout<<"enter data: ";
                getline(cin,data);
                fs.write_file(name,data);
            }
            else if(op==5){
                char name[32];
                cout<<"enter filename: \n";
                cin>>name;
                string content;
                if(fs.read_file(name,content)){
                    cout<<"file content: "<<content<<"\n";
                }
            }
            else if(op==6){
                char name[32];
                cout<<"enter filename to delete: ";
                cin>>name;
                fs.delete_file(name);
            }
            else{
                cerr<<"invalid option,try again...\n";
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