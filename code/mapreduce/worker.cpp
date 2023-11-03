#include"worker.hpp"
#include<memory>
#include"buttonrpc.hpp"

// in fact this can take the thread pool to avoid the time 
void Worker::map_task(){
    auto task([&](){

    });
    for(int i=0;i<thread_work_num;i++){
        std::thread(task());
    }
}

void Worker::reduce_task(){
    auto task([&](){
        
    });
    for(int i=0;i<reduce_task;i++){
        std::thread(task());
    }
}

int main(){
    zmq::buttonrpc work_client;
    work_client.as_client("127.0.0.1", 5555);//async can't be taken here
    work_client.set_timeout(5000);
    int map_task_num = work_client.call<int>("getMapNum").val();
    int reduce_task_num = work_client.call<int>("getReduceNum").val();
    auto worker_point=std::make_unique<Worker>(map_task_num,reduce_task_num);
    worker_point->map_task();
    worker_point->reduce_task();
    while(1){
        if(work_client.call<bool>("Done").val());
        sleep(1);
    }

}


