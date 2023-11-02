#include<iostream>
#include<memory>
#include<algorithm>
#include<unordered_map>
#include<vector>
#include<fstream>
#include<sstream>
#include<deque>
#ifndef WORKER_HPP
#define WORKER_HPP

struct KeyMap{
    std::string key;
    char ch;
};
struct KeyShuffle{
    std::string key;
    std::string val;
};

class Worker{
public:
    explicit Worker(int8_t mode):m_mode(mode){
        
    };
    //pass file fd to this function to split words
    void work_read(std::ifstream& fd){
        std::string _str;
        while(fd>>_str){
            str.push_back(_str);
            _str.clear();
        }
    }
    std::deque<KeyMap> work_spilt(std::ifstream& fd){
        work_read(fd);
        for(auto &str_:str){
            map.push_back((KeyMap){str_,'1'});
        }
        return map;
    }
    std::deque<KeyShuffle> work_shuffle(std::deque<KeyMap> key_map){
        
    }

private:
    std::vector<std::string> str;
    std::deque<KeyMap> map;

    int8_t m_mode;  





};



#endif // !WORKER_HPP




