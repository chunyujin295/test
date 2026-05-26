#pragma once

#include "Base.h"
#include "CustomNode.h"
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class CustomNode;

class ThreadManager {

public:
    ThreadManager(int threadNum);
    ~ThreadManager();

    void Add(CustomNode* arg);
private:
    // 任务队列
    std::queue<CustomNode*> taskQ;
    vector<thread> threadIDs;
    mutex mutexPool;    //整个线程池的锁
    int maxNum;
    condition_variable cond;     //任务队列是否为空,阻塞工作者线程
    static void Worker(void* arg);   //工作线程

    osg::ref_ptr<osg::Switch> m_pRootSwitch;
};