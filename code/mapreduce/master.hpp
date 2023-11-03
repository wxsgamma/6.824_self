#include<iostream>
#include<memory>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<deque>
#ifndef MASTER_HPP
#define MASTER_HPP

class Master{
public:
    explicit Master(int thread_work=6,int thread_reduce=6){};
    void getfilename(char *ch,int argc){
        for(int i=1;i<argc;i++){
            list.emplace_back(ch[i]);
        }
        file_num=argc-1;
    }
    
    int get_work_num(){
        return m_work_num;
    }
    int get_reduce_num(){
        return m_reduce_num;
    }
    bool is_map_Done(){
        std::lock_guard<std::mutex> lock(mtx);
        int len=m_work_num;
        return file_num==m_reduce_num;
    }
    bool is_reduce_Done(){
        std::lock_guard<std::mutex> lock(mtx);
        int len=m_reduce_num;
        return file_num==m_reduce_num;
    }


    
private:
    bool done;
    int file_num;
    int m_work_num;
    int m_reduce_num;
    std::mutex mtx;
    std::deque<char*> list;


};

#endif