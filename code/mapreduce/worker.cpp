#include"worker.hpp"
#include<memory>
#include"buttonrpc.hpp"

// in fact this can take the thread pool to avoid the time 
void Worker::map_task(){
    auto task([&](){
//1、初始化client连接用于后续RPC;获取自己唯一的MapTaskID
    buttonrpc client;
    client.as_client("127.0.0.1", 5555);
    pthread_mutex_lock(&map_mutex);
    int mapTaskIdx = MapId++;
    pthread_mutex_unlock(&map_mutex);
    bool ret = false;
    while(1){
    //2、通过RPC从Master获取任务
    //client.set_timeout(10000);
        ret = client.call<bool>("isMapDone").val();
        if(ret){
            cv.notify_all();
            return NULL;
        }
        string taskTmp = client.call<string>("assignTask").val();   //通过RPC返回值取得任务，在map中即为文件名
        if(taskTmp == "empty") continue; 
        
        mtx.lock();

        //------------------------自己写的测试超时重转发的部分---------------------
        //注：需要对应master所规定的map数量，因为是1，3，5被置为disabled，相当于第2，4，6个拿到任务的线程宕机
        //若只分配两个map的worker，即0工作，1宕机，我设的超时时间比较长且是一个任务拿完在拿一个任务，所有1的任务超时后都会给到0，
        if(disabledMapId == 1 || disabledMapId == 3 || disabledMapId == 5){
            disabledMapId++;
            mtx.unlock();
            printf("%d recv task : %s  is stop\n", mapTaskIdx, taskTmp.c_str());
            while(1){
                sleep(2);
            }
        }else{
            disabledMapId++;   
        }
        mtx.unlock();
        //------------------------自己写的测试超时重转发的部分---------------------

    //3、拆分任务，任务返回为文件path及map任务编号，将filename及content封装到kv的key及value中
        char task[taskTmp.size() + 1];
        strcpy(task, taskTmp.c_str());
        KeyValue kv = getContent(task);

    //4、执行map函数，然后将中间值写入本地
        vector<KeyValue> kvs = mapF(kv);
        writeInDisk(kvs, mapTaskIdx);

    //5、发送RPC给master告知任务已完成
        printf("%d finish the task : %s\n", mapTaskIdx, taskTmp.c_str());
        client.call<void>("setMapStat", taskTmp);

    }
    });
    for(int i=0;i<thread_work_num;i++){
        std::thread(task()).detach();
    }
}

void Worker::reduce_task(){
    auto task([&](){
    //removeFiles();
    buttonrpc client;
    client.as_client("127.0.0.1", 5555);
    bool ret = false;
    while(1){
        //若工作完成直接退出reduce的worker线程
        ret = client.call<bool>("Done").val();
        if(ret){
            return NULL;
        }
        int reduceTaskIdx = client.call<int>("assignReduceTask").val();
        if(reduceTaskIdx == -1) continue;
        mtx.lock();

        //人为设置的crash线程，会导致超时，用于超时功能的测试
        if(disabledReduceId == 1 || disabledReduceId == 3 || disabledReduceId == 5){
            disabledReduceId++;
            mtx.unlock();
            while(1){
                sleep(2);
            }
        }else{
            disabledReduceId++;   
        }
        mtx.lock();

        //取得reduce任务，读取对应文件，shuffle后调用reduceFunc进行reduce处理
        vector<KeyValue> kvs = Myshuffle(reduceTaskIdx);
        vector<string> ret = reduceF(kvs, reduceTaskIdx);
        vector<string> str;
        for(int i = 0; i < kvs.size(); i++){
            str.push_back(kvs[i].key + " " + ret[i]);
        }
        string filename = "mr-out-" + to_string(reduceTaskIdx);
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0664);
        myWrite(fd, str);
        close(fd);
        client.call<bool>("setReduceStat", reduceTaskIdx);  //最终文件写入磁盘并发起RPCcall修改reduce状态
    }        
    });
    for(int i=0;i<reduce_task;i++){
        std::thread(task()).detach();
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
    worker_point->mtx.lock();
    worker_point->cv.wait();
    worker_point->mtx.unlock();
    worker_point->reduce_task();
    while(1){
        if(work_client.call<bool>("Done").val());
        sleep(1);
    }

}


