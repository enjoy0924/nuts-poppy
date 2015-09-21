#ifndef __TBB_THREAD_POOL_H__
#define __TBB_THREAD_POOL_H__
#include "tbb/parallel_do.h"
#include <functional>
#include "tbb/task_scheduler_init.h"

//@Scope("singleton")
class CTBBThreadPool
{
public:
    CTBBThreadPool(){}
    ~CTBBThreadPool(){}

    void initializeThreadPool(int numOfThreads = -1)
    {
        numOfThreads = 1000;
        m_taskScheduler.initialize(numOfThreads);
    }

    bool isThreadPoolActive()
    {
        return m_taskScheduler.is_active();
    }

    bool wakeupThread(tbb::task* runnableTask)
    {
        //////////////////////////////////////////////////////////////////////////
        return true;
    }

    void finalizeThreadPool()
    {
        m_taskScheduler.terminate();
    }


protected:
    
private:
    tbb::task_scheduler_init m_taskScheduler;
};

typedef ACE_Singleton<CTBBThreadPool , ACE_Null_Mutex> GlobalThreadPoolMgr; 


#endif