#ifndef __IO_SEESION_MGR_H__
#define __IO_SEESION_MGR_H__
#include "ace/ace_header.h"
#include "IoSession.h"

//@Scope("singleton")
class IoSessionMgr
{
public:
    IoSessionMgr();
    ~IoSessionMgr();

    CIOSession* getAvailableIoSession(const int sock);
    void freeIoSessionByID(const int& sessionId);
    
    CIOSession* getOccupancyIoSessionByID(const int& handleChannel);

    bool setIoEventHandlerBySockFd( int hIoHanlder, IoEventHandler ioHandlerFunc, void* userdata);

private:
    bool _SetIoSessionBusy( CIOSession* pIoSession );
    bool _SetIoSessionIdle(CIOSession* pIoSession);
    CIOSession* _GetIdleSession();

    int m_iSessionIdentity;
    ACE_Mutex m_iSessionMutex;
    std::hash_map<int, CIOSession*> m_sessionMap;
};

typedef ACE_Singleton<IoSessionMgr , ACE_Null_Mutex> GlobalIoSessionMgr; 

#endif