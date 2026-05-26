
#include "ThreadManager.h"

ThreadManager::ThreadManager(int threadNum)
{
    maxNum = threadNum;

    threadIDs.resize(threadNum);
    for (int i = 0; i < threadNum; ++i)
    {
        threadIDs[i] = thread(Worker, this);
    }
}

ThreadManager::~ThreadManager()
{

    cond.notify_all();
    for (int i = 0; i < maxNum; ++i)
    {
        if (threadIDs[i].joinable()) threadIDs[i].join();
    }
}

void ThreadManager::Worker(void* arg)
{
    ThreadManager* pool = static_cast<ThreadManager*>(arg);

    while (true)
    {
        unique_lock<mutex> lk(pool->mutexPool);

        // 当前任务队列是否为空
        while (pool->taskQ.empty())
        {
            pool->cond.wait(lk);
        }

        CustomNode* pNode = pool->taskQ.front();
        
        pool->taskQ.pop();

        lk.unlock();
     
        if (pNode != nullptr)
        {
            pNode->Init();
        }
    }
}


void ThreadManager::Add(CustomNode* arg)
{
    unique_lock<mutex> lk(mutexPool);
    //添加任务
    taskQ.push(arg);

    cond.notify_all();
}