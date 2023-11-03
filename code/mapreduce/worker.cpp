#include"worker.hpp"
#include<memory>
#include"buttonrpc"

int main(){
    zmq::buttonrpc work_client;
    work_client.as_client("127.0.0.1", 5555);
    work_client.set_timeout(5000);
    int map_task_num = work_client.call<int>("getMapNum").val();
    int reduce_task_num = work_client.call<int>("getReduceNum").val();
    auto worker_point=std::make_unique<Worker>(map_task_num,reduce_task_num);
    




}


