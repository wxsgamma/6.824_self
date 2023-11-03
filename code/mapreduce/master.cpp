#include"master.hpp"
#include"zmq.hpp"
#include "buttonrpc.hpp"
int main(int argc,char *argv[]){
    if(argc < 2){
        std::cout<<"missing parameter! The format is ./Master pg*.txt"<<std::endl;
        exit(-1);
    }
    // alarm(10);
    zmq::buttonrpc server;
    server.as_server(5555);
    Master master(13, 9);
    master.getfilename(argv, argc);
    server.bind("get_work_num",&Master::get_work_num,&master);
    server.run();
    return 0;



}