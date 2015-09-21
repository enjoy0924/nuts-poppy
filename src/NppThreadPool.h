#ifndef __NPP_THREAD_POOL_H__
#define __NPP_THREAD_POOL_H__
#include "IoSession.h"
#include "NppThread.h"
#include "NppMessageFactory.h"
#include <set>
#include <list>

class CNppThreadPool
{
public:
    CNppThreadPool(void);
    ~CNppThreadPool(void);

    //**initialize the thread pool, permit to invoke only once
    //*minThreads, the number of min threads
    //*maxThreads, the number of max threads
    //**return true if no error happened, or return false
    bool Initialize(unsigned int minThreads, unsigned int maxThreads);

    //**terminate the thread pool
    void Finalize();

    //*************************************************************************************
    //**execute one runnable task, this is a reverse interface, it's not implemented**
    //*pRunnable, the handle of runnable task
    //**return true if successful, or return false
    bool Execute(Runnable * pRunnable);

    //**get thread pool size
    unsigned int GetThreadPoolSize();

    //**session will be in line here if some session has the request of execute task[npp message]
    //*session, one specified io session, there's some message carried by it
    //**return true if successful, or return false
    bool waitAtSessionQueue(CIOSession* session);

private:
    class CWorker : 
        public CNppThread 
    {
    public:
        CWorker(CNppThreadPool * pThreadPool, Runnable * pFirstTask = NULL);
        ~CWorker();
        virtual void Run();
    private:
        void _OnMessage(NutsWrapperMessage* message);
        void _OnNonNppMessage(const void* nonnppmessage);
        CNppThreadPool * m_pThreadPool;
    };

private:
    static unsigned int WINAPI _StaticThreadFunc(void * arg);
    bool _WorkerJumpIntoPool(CWorker * worker);
    unsigned int _GetCurrentThreadPoolSize();
    bool _isRunning();
    CIOSession* _fetchWaitingSession();

    //****************************************************************
    //*it's a reverse interface, maybe implemented later
    //** flexible thread pool
    //****************************************************************
    bool _AddWorkIfNeccessary();
private:
    typedef std::set<CWorker *> ThreadPool;
    typedef ThreadPool::iterator ThreadPoolItr;

    //the mutex for thread pool
    ACE_Mutex m_mutexPool;
    ThreadPool m_ThreadPool;
    //ThreadPool m_TrashThread;

    volatile bool m_bInitialized;
    volatile bool m_bRun;
    volatile bool m_bEnableInsertTask;

    //the thread pool configuration
    volatile unsigned int m_minThreads;
    volatile unsigned int m_maxThreads;
    
    //volatile unsigned int m_maxPendingTasks;
    
    //make sure the order
    ACE_Mutex m_mutexWaiting;
    ACE_Semaphore m_semaphoreWaiting;
    std::list<CIOSession*> m_waitingSessions;
};

typedef ACE_Singleton<CNppThreadPool , ACE_Null_Mutex> GlobalNppThreadPool; 

#endif