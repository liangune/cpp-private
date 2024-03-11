
#include <iostream>
#include "libpublic/rand.h"
#include "libpublic/datetime.h"
#include "libpublic/syncQueue.h"
#include <memory>
#include "libpublic/workpool/taskExample.h"
#include "libpublic/workpool/worker.h"
#include "libpublic/workpool/workerPool.h"
#include "libpublic/workpool/header.h"
#include <thread>
#include "libpublic/localtime.h"
//#include "libpublic/byteBuffer.h"

using namespace std;

template<typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

int main(int argc, const char** argv) {
    uint32_t nNum = Rand::randInt(1, 20);
    string randStr = Rand::randString(5);

    cout << " randInt: " << nNum << " randStr: " << randStr << endl;

    size_t beginTime = CTime::getTodayBeginTime();
    size_t endTime = CTime::getTodayEndTime();
    int32_t lsec = CTime::getTodayLeftSecond();

    cout << "beginTime: " << beginTime << " endTime: " << endTime << " lsec: " << lsec << endl;


    CDatetime datetime1;
    cout << "datetime1: " << datetime1.getSec() << " " << datetime1.getStrTime() << endl;

    CDatetime datetime2(time(NULL));
    cout << "datetime2: " << datetime2.getSec() << " " << datetime2.getStrTime() << endl;

    CDatetime datetime3("2020-11-20 11:00:00", "%Y-%m-%d %H:%M:%S");
    cout << "datetime3: " << datetime3.getSec() << " " << datetime3.getStrTime() << endl;

    CDuration duration(1, 0, 0, 0);
    //CDatetime datetime4 = datetime3 - duration;
    CDatetime datetime4 = datetime3.sub(duration);
    cout << "datetime4: " << datetime4.getSec() << " " << datetime4.getStrTime() << endl;

    std::string str = "test task1";
    SyncQueue<std::shared_ptr<TaskExample>> queue;
    auto p(std::make_shared<TaskExample> ());
    p->SetValue(str);
    queue.push(p);

    std::cout << "==== task1 p1 ====" << std::endl;
    auto p1 = std::move(queue.pop());
    p1->Execute(NULL);

    std::shared_ptr<TaskExample> p2 = std::make_shared<TaskExample> ();
    std::cout << "==== task1 p2 ====" << std::endl;
    queue.tryPop(p2);
    p2->Execute(NULL);


    std::shared_ptr<TaskExample> p3 = std::make_shared<TaskExample> ();
    std::string str2 = "test task2";
    p3->SetValue(str2);


    std::cout << "=====" << std::endl;
    WorkerPool *workerpool = new WorkerPool();
    workerpool->Init(new WorkerFactory<IWORKER_INSTANCE>(), 3, nullptr);
    workerpool->Start();
    
    workerpool->AddTask(p1);
    workerpool->AddTask(p2);
    workerpool->AddTask(p3);

    CLocaltime clocaltime;
    std::cout << "=====stm.is_dst= " << clocaltime.getDaylightActive() << std::endl;

    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}