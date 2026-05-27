#include "disk.hpp"
#include "fs.hpp"
#include "inode.hpp"
#include "bitmap.hpp"
#include "directory.hpp"
#include <iostream>
#include<cstring>
#include<sstream>
#include<string>
#include<algorithm>
using std::vector;
using std::cout;
using std::cin;
using std::cerr;
using std::string;
using std::fstream;
using std::istringstream;
void trim(string &s){
    while(!s.empty() && (s.back()==' ' || s.back()=='\t'))s.pop_back();
    size_t i=0;
    while(i<s.size() && (s[i]==' '||s[i]=='\t'))i++;
    s=s.substr(i);
}
void help(){
    cout<<"------------Commands--------\n";
    cout<<" ls             list current directory\n";
    cout<<" pwd            print current directory\n";
    cout<<" mkdir          create directory\n";
    cout<<" cd    <dir>    change directory\n";
    cout<<" touch <file>   create empty file\n";
    cout<<" write <file>   write file\n";
    cout<<" cat <file>     read file\n";
    cout<<" rm <file>      delete file\n";
    cout<<" debug          show filesystem info\n";
    cout<<" clear          clear screen\n";
    cout<<" exit           exiting the program\n";
    cout<<"----------------------------------------\n\n";
}

int main(){
    try{
        Disk disk("disk.img");
        FileSystem fs(disk);
        cout<<"Virtual Filesystem Simlator\n";
        cout<<"Type help for commands\n";
        cout<<"format | load\n";
        cout<<"vfs> ";
        string choice;
        cin>>choice;
        cin.ignore(10000,'\n');
        if(choice=="format"){
            fs.format();
        }
        else if(choice=="load"){
            fs.load();
        }
        else{
            cerr<<"invalid choice !!\n";
            return 1;
        }
        string word;    
        while(true){
            cout<<"vfs> ";
            if(!std::getline(cin,word))break;
            trim(word);
            if(word.empty())continue;
            istringstream iss(word);
            string com;
            iss>>com;
            if(com=="help")help();
            else if(com=="exit"){cout<<"exiting...\n";break;}
            else if(com=="clear"){
                cout<<"\033[2J\033[H"; //ansi clear
            }
            else if(com=="debug")fs.debug();
            else if(com=="ls" || com=="list")fs.list_files();
            else if(com=="pwd")fs.pwd();
            else if(com=="mkdir"){
                string name;
                iss>>name;
                if(name.empty())cerr<<"empty string\n";
                else fs.make_dir(name.c_str());
            }
            else if(com=="cd"){
                string name;
                iss>>name;
                if(name.empty())cerr<<"empty string\n";
                else fs.change_dir(name.c_str());
            }
            else if(com=="touch" || com=="create"){
                string name;
                iss>>name;
                if(name.empty())cerr<<"empty string\n";
                else fs.create_file(name.c_str());
            }
            else if(com=="write"){
                string name;
                iss>>name;
                if(name.empty()){
                    cerr<<"empty string\n";
                continue;}
                string rem;
                std::getline(iss,rem);
                trim(rem);
                if(rem.empty()){
                cout<<"enter content : ";
                std::getline(cin,rem);
            }
            fs.write_file(name.c_str(),rem);
            }
            else if(com=="cat" || com=="read"){
                string name;
                iss>>name;
                if(name.empty()){
                    cerr<<"empty string\n";
                    continue;
                }
                string data;
                if(fs.read_file(name.c_str(),data))cout<<data<<"\n";
            }
            else if(com=="rm" || com=="delete"){
                string name;
                iss>>name;
                if(name.empty())cerr<<"empty string\n";
                else fs.delete_file(name.c_str());
            }
            else{
        cerr<<"unknown command,type help to know.\n";}
    }}
    catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<"\n";
        return 1;
    }
  


return 0;}