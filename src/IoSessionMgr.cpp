#include "IoSessionMgr.h"
#include "MessageCallbackMgr.h"
#include "NppLog.h"

IoSessionMgr::IoSessionMgr():
m_iSessionIdentity(0)
{}

IoSessionMgr::~IoSessionMgr()
{}

CIOSession* IoSessionMgr::getAvailableIoSession(const int sock)
{
    if (INVALID_SOCKET == sock)
        return NULL;
    ACE_Guard<ACE_Mutex> guard(m_iSessionMutex);
    CIOSession* ioSession = NULL;
    std::hash_map<int, CIOSession*>::iterator iter = m_sessionMap.begin();
    while(iter != m_sessionMap.end())
    {
        ioSession = iter->second;
        if (NULL != ioSession && ioSession->getSockFd() == INVALID_SOCKET 
            && ioSession->getStatus() == SESSION_CLOSED)
        {
            ioSession->setSockFd(sock);
            return ioSession;
        }
        iter++;
    }

    ioSession = new(std::nothrow) CIOSession(sock);
    if (NULL != ioSession)
    {
        m_iSessionIdentity++;
        ioSession->setIdentity(m_iSessionIdentity);
        ioSession->setSockFd(sock);
        m_sessionMap[m_iSessionIdentity] = ioSession;
    }
    
    return ioSession;
}

CIOSession* IoSessionMgr::getOccupancyIoSessionByID( const int& handleChannel ) 
{
    ACE_Guard<ACE_Mutex> guard(m_iSessionMutex);
    return m_sessionMap[handleChannel];
}

void IoSessionMgr::freeIoSessionByID(const int& sessionId)
{
    CIOSession* pDeadSession = NULL;
    {
        ACE_Guard<ACE_Mutex> guard(m_iSessionMutex);
        std::hash_map<int, CIOSession*>::iterator iter = m_sessionMap.find(sessionId);
        if (iter != m_sessionMap.end())
        {
            pDeadSession = iter->second;
            if (NULL == pDeadSession)
                return;
            if (pDeadSession->isOpenTimeoutMechanism())
                pDeadSession->stopLookupTimeoutRequest();

            pDeadSession->setStatus(SESSION_CLOSED);
            pDeadSession->setSockFd(INVALID_SOCKET);
        }
    }
}

bool IoSessionMgr::setIoEventHandlerBySockFd(int hIoHanlder, IoEventHandler ioHandlerFunc, void* userdata)
{
    //CIOSession* session = getOccupancyIoSessionByID(hIoHanlder);
    //if (NULL == session)
    //    return false;

    //session->setIoSessionEventHandler(ioHandlerFunc, userdata);

    return false;
}
