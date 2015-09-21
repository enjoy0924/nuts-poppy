#include "NppThreadPool.h"
#include "NppLog.h"


//**************************the implement of class CWork**************************
CNppThreadPool::
    CWorker::CWorker(CNppThreadPool * pThreadPool, Runnable * pFirstTask) : 
        m_pThreadPool(pThreadPool)
{
    
}

CNppThreadPool::
    CWorker::~CWorker()
{
}

//**how does the worker work
void CNppThreadPool::
    CWorker::Run()
{
    CIOSession * pTaskSession = NULL;
    while(isRun() && NULL != m_pThreadPool)
    {//the worker get one task from thread pool
        pTaskSession = m_pThreadPool->_fetchWaitingSession();

        if(NULL == pTaskSession)
        {//there's no task assigned to the worker
            LOG_DEBUG("there is no message task in queue, stuck worker here");
            continue;
        }
        else
        {//let the work execute task
            std::list<void*> messageQueue;
            pTaskSession->getMessageTaskQueue(messageQueue);

            if (pTaskSession->getSessionType() == SES_TYPE_NPP)
            {
                while (!messageQueue.empty() && pTaskSession->isAvailbale())
                {
                    void* message = messageQueue.front();
                    NutsWrapperMessage* taskMessage = reinterpret_cast<NutsWrapperMessage*>(message);
                    _OnMessage(taskMessage);
                    messageQueue.pop_front();
                    delete taskMessage;
                    taskMessage = NULL;
                }
            }
            else
            {
                int messagelen = 0;
                int actualSize = 0;
                while (!messageQueue.empty() && pTaskSession->isAvailbale())
                {
                    void* message = messageQueue.front();
                    char* taskMessage = reinterpret_cast<char*>(message);
                    memcpy(&messagelen, taskMessage, 4);
                    memcpy(&actualSize, taskMessage+4, 4);
                    pTaskSession->FireMessageRecved(taskMessage+8, messagelen);
                    messageQueue.pop_front();
                    GlobalOverlappedBufPool::instance()->freeRawBuffer(taskMessage, actualSize);
                }
            }


        }
    }
}

void CNppThreadPool::
    CWorker::_OnMessage(NutsWrapperMessage* message)
{
    if (NULL == message)
        return;
    WrapperNppMessage wrapperReq;
    int _messagecode = message->messagecode();
    int _messagetype = message->type();
    wrapperReq.setDestination(message->destination());
    wrapperReq.setMessageCode(_messagecode);
    wrapperReq.setMessageSeq(message->sequencenum());
    wrapperReq.setMessageType(_messagetype);
    wrapperReq.setSource(message->source());
    nppMessageConstructor messageConstrcutor = GlobalMessageCallbackMgr::instance()->getMessageConstructor();

    do 
    {
        if (NULL == messageConstrcutor)
        {
            LOG_DEBUG("can not find npp message constructor!");
            break;
        }

        void* pMsg = messageConstrcutor(message->messagename().c_str());
        if (NULL == pMsg)
        {
            LOG_DEBUG("message constrcutor construct npp message failed!");
            break;
        }
        google::protobuf::Message* pRealMsg = reinterpret_cast<google::protobuf::Message*> (pMsg);
        if (pRealMsg->ParseFromString(message->messageentity()))
            wrapperReq.setMessageEntity(pRealMsg);
        else
            wrapperReq.setMessageEntity(NULL);

        DoMessageExecute _onMessageFunc = GlobalMessageCallbackMgr::instance()->getMessageCallbackByCode(_messagecode);
        if (NULL != _onMessageFunc)
        {
            WrapperNppMessage* wrapperRsp = _onMessageFunc(&wrapperReq);
            if (NULL != wrapperRsp && MessageType::REQUEST_WITH_RSP == _messagetype)
            {//this message need to be returned
                Npp_SendNppMSG(wrapperRsp);
            }
        }

        if (NULL != pRealMsg)
        {
            delete pRealMsg;
            pRealMsg = NULL;
        }
    } while (false);
}

//**************************the implement of class CNppThreadPool**************************
CNppThreadPool::CNppThreadPool(void) : 
m_bRun(false),
m_bEnableInsertTask(false),
m_bInitialized(false)
{
}

CNppThreadPool::~CNppThreadPool(void)
{
    Finalize();
}

bool CNppThreadPool::Initialize(unsigned int minThreads, unsigned int maxThreads)
{
    if (m_bInitialized)
        return false;
    
    if(minThreads <= 0)
    {
        minThreads = 1;
    }
    if(maxThreads < minThreads)
    {
        return false;
    }
    m_minThreads = minThreads;
    m_maxThreads = maxThreads;
    unsigned int i = _GetCurrentThreadPoolSize();
    for(; i<minThreads; i++)
    {
        //create thread workers
        CWorker * pWorker = new(std::nothrow) CWorker(this);
        if(NULL == pWorker)
        {
            return false;
        }
        pWorker->Start();
        _WorkerJumpIntoPool(pWorker);
    }
    m_bInitialized = true;
    m_bRun = true;
    m_bEnableInsertTask = true;
    return true;
}

bool CNppThreadPool::_WorkerJumpIntoPool(CWorker * worker)
{
    ACE_Guard<ACE_Mutex> guard(m_mutexPool);
    m_ThreadPool.insert(worker);
    return true;
}


bool CNppThreadPool::_AddWorkIfNeccessary()
{
    //////////////////////////////////////////////////////////////////////////
    return true;
}

unsigned int CNppThreadPool::_GetCurrentThreadPoolSize()
{
    ACE_Guard<ACE_Mutex> guard(m_mutexPool);
    return m_ThreadPool.size();
}

bool CNppThreadPool::Execute(Runnable * pRunnable)
{
    //////////////////////////////////////////////////////////////////////////
    return true;
}

void CNppThreadPool::Finalize()
{
    //refuse new task
    m_bEnableInsertTask = false;
    m_bRun = false;
    m_minThreads = 0;
    m_maxThreads = 0;
}

bool CNppThreadPool::_isRunning()
{
    return m_bRun;
}

unsigned int CNppThreadPool::GetThreadPoolSize()
{
    return _GetCurrentThreadPoolSize();
}

bool CNppThreadPool::waitAtSessionQueue(CIOSession* session)
{
    {
        if (NULL == session || session->getMessageTaskNum() == 0)
            return false;
        {
            ACE_Guard<ACE_Mutex> guard(m_mutexWaiting);
            m_waitingSessions.push_back(session);
        }

        m_semaphoreWaiting.release();

        return true;
    }
}

CIOSession* CNppThreadPool::_fetchWaitingSession()
{
    CIOSession* session = NULL;
    //wait semaphore here forever unless accept EXIT_SIGLE
    m_semaphoreWaiting.acquire();

    ACE_Guard<ACE_Mutex> guard(m_mutexWaiting);
    if (!m_waitingSessions.empty())
    {
        session = m_waitingSessions.front();
        m_waitingSessions.pop_front();
    }
    return session;
}