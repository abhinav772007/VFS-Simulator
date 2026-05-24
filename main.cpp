#include "disk.hpp"
#include <iostream>
using std::vector;
using std::cout;
using std::cerr;
using std::string;
using std::fstream;

int main(){
    try{
        Disk disk("disk.img");
        vector<char> data(Disk::BLOCK_SIZE,0);
        string msg="Hello,this is abhinav!";
        std::copy(msg.begin(),msg.end(),data.begin());
        disk.write_block(5,data);
        vector<char> read_data;
        disk.read_block(5,read_data);
        string output(read_data.begin(),read_data.end());
        cout<<"Written message: "<<msg<<"\n";
        cout<<"output message: "<<output<<"\n";
    }
    catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<"\n";
        return 1;
    }
    return 0;

}