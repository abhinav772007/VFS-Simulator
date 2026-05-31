#include "disk.hpp"
#include <iostream>
using std::cout;
Disk::Disk(const std::string &filename):filename(filename),cache_enabled(false),cache_capacity(64),cache_hits(0),cache_misses(0){
    file.open(filename,std::ios::in | std::ios::out | std::ios::binary);
    if(!file){
        std::cout<<"Disk not found :(,\n creating disk\n";
        initialize_disk();
        file.open(filename,std::ios::in | std::ios::out | std::ios::binary);
    }
}
void Disk::initialize_disk(){
std::fstream newfile(filename,std::ios::out | std::ios::binary);
std::vector<char> zero_block(BLOCK_SIZE,0);
for(int i=0;i<TOTAL_BLOCKS;i++){
    newfile.write(zero_block.data(),BLOCK_SIZE);
}
newfile.close();
std::cout<<"Disk initialized successfully\n";
}
void Disk::read_block_uncached(int block_num,std::vector<char> & buffer){
    buffer.resize(BLOCK_SIZE);
    file.seekg(block_num*BLOCK_SIZE,std::ios::beg);
    file.read(buffer.data(),BLOCK_SIZE);
    if(!file){
        throw std::runtime_error("Failed to read block");
    }
}
void Disk::write_block_uncached(int block_num,const std::vector<char>& buffer){
    file.seekp(block_num*BLOCK_SIZE,std::ios::beg);
    file.write(buffer.data(),BLOCK_SIZE);
    file.flush();
    if(!file){
        throw std::runtime_error("Failed to write block");
    }
}
void Disk::clear_touch(int block_num){
    ll key=block_num;
    for(auto it=cache_lru.begin();it!=cache_lru.end();++it){
        if(*it==key){
            cache_lru.erase(it);
            break;
        }
    }
    cache_lru.push_front(key);
}
void Disk::cache_put(int block_num,const std::vector<char>& data){
    ll key=block_num;
    if(cache_map.find(key)!=cache_map.end()){
        cache_map[key]=data;
        clear_touch(block_num);
        return;
    }
    while((int)cache_lru.size()>=cache_capacity){
        ll last=cache_lru.back();
        cache_map.erase(last);
        cache_lru.pop_back();
    }
    cache_map[key]=data;
    cache_lru.push_front(key);
}
void Disk::read_block(int block_num,std::vector<char> & buffer){
    if(block_num<0 || block_num>=TOTAL_BLOCKS){
        throw std::invalid_argument("Invalid block number");
    }
    if(cache_enabled){
        ll key=block_num;
        auto it=cache_map.find(key);
        if(it!=cache_map.end()){
            buffer=it->second;
            cache_hits++;
            clear_touch(block_num);
            return;
        }
        cache_misses++;
        }
        read_block_uncached(block_num,buffer);
        if(cache_enabled){
            cache_put(block_num,buffer);
        }
}
void Disk::write_block(int block_num,const std::vector<char>& buffer){
    if(block_num<0 || block_num>=TOTAL_BLOCKS){
        throw std::invalid_argument("Invalid block number");
    }
    write_block_uncached(block_num,buffer);
    if(cache_enabled){
        cache_put(block_num,buffer);
    }
}

std::string &Disk::path(){return filename;}

void Disk::close_disk(){
    if(file.is_open())file.close();
   
}

void Disk::reopen_disk(){
    if(file.is_open())file.close();
    file.open(filename,std::ios::in | std::ios::out | std::ios::binary);
    if(!file) throw std::runtime_error("failed to close disk file");
    clear_cache();
}

void Disk::set_cache_enabled(bool enabled){
    cache_enabled=enabled;
    if(!enabled)clear_cache();
}

bool Disk::is_cache_enabled(){return cache_enabled;}

void Disk::set_cache_capacity(int cap){
    if(cap<1)throw std::invalid_argument("cache capacity must be at least 1");
    if(cap>TOTAL_BLOCKS)throw std::invalid_argument("cache capacity must be less than total blocks");
    cache_capacity=cap;
    while((int)cache_lru.size()>cache_capacity){
        ll last=cache_lru.back();
        cache_map.erase(last);
        cache_lru.pop_back();
    }
}

void Disk::clear_cache(){
    cache_map.clear();
    cache_lru.clear();
    cache_hits=0;
    cache_misses=0;
}

void Disk::cache_stats(){
    ll tot=cache_hits+cache_misses;
    double rate=0.0;
    if(tot>0)rate=(double)cache_hits/tot;
    cout<<"-----------STATS-----------\n";
    cout<<"cache enabled: "<<(cache_enabled?"on":"off")<<"\n";
    cout<<"capacity: "<<cache_capacity<<"\n";
    cout<<"hits: "<<cache_hits<<"\n";
    cout<<"misses: "<<cache_misses<<"\n";
    cout<<"hit rate: "<<rate*100.0<<"%\n";
    cout<<"-----------------------------------\n";
}