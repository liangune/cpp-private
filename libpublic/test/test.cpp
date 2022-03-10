
#include <iostream>
#include "libpublic/rand.h"
#include "libpublic/datetime.h"
#include "libpublic/syncQueue.h"
#include <memory>
#include "libpublic/workpool/taskExample.h"
#include "libpublic/workpool/worker.h"
#include "libpublic/workpool/workerPool.h"
#include "libpublic/workpool/header.h"

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

    //auto p1 = queue.pop();
    std::shared_ptr<TaskExample> p1 = std::make_shared<TaskExample> ();
    std::cout << "=====" << std::endl;
    queue.tryPop(p1);
    //p1->execute();

    std::shared_ptr<TaskExample> p2 = std::make_shared<TaskExample> ();
    std::string str2 = "test task2";
    p2->SetValue(str2);

    //Worker *worker = new Worker();
    //worker->start();
    //worker->addTask(p2);

    std::string str3 = "test task3";
    // p2->setValue(str3);
    // worker->addTask(p2);
    std::cout << "=====" << std::endl;
    WorkerPool *workerpool = new WorkerPool();
    workerpool->Init(new WorkerFactory<IWORKER_INSTANCE>(), 1, nullptr);
    workerpool->Start();
    workerpool->AddTask(p1);
    workerpool->AddTask(p2);

    while(1) {}
    return 0;
}