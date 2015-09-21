#include "IoSession.h"
#include "IoSessionMgr.h"
#include "NppLog.h"
#include "NppMessageFactory.h"
#include "NppThreadPool.h"

CIOSession::CIOSession(SOCKET sock):m_sticker(CDefaultMessageParse::defalutStickerParse),
    m_encoderFunc(CDefaultEncodeWrapper::defaultEncodeWrapper),
    m_status(SESSION_CLOSED),
    m_peeraddr(0),
    m_port(0),
    m_hTimer(INVALID_HANDLE_VALUE),
    m_openTimeout(false),
    m_ioHandlerFunc(NULL),
    m_ioUserdata(NULL),
    m_identity(-1),
    m_iSessionType(0),
    m_lSockSendRef(0),
    m_idleRef(0)
{
    m_socket = sock;
    m_strAddr[0] = 0;
}

CIOSession::CIOSession(SOCKET sock, stickMessageHandler sticker):
m_encoderFunc(CDefaultEncodeWrapper::defaultEncodeWrapper),
    m_status(SESSION_CLOSED),
    m_peeraddr(0),
    m_port(0),
    m_hTimer(INVALID_HANDLE_VALUE),
    m_openTimeout(false),
    m_ioHandlerFunc(NULL),
    m_ioUserdata(NULL),
    m_identity(-1),
    m_iSessionType(0),
    m_lSockSendRef(0),
    m_idleRef(0)
{
    m_socket = sock;
    m_sticker = sticker;
    m_strAddr[0] = 0;
}
CIOSession::~CIOSession()
{
}

const char* CIOSession::getRemote()
{   
    if (m_strAddr[0] != 0)
        return m_strAddr;
    sprintf_s(m_strAddr, "%d.%d.%d.%d:%d", 
        (0x000000FF&m_peeraddr), (0x0000FF00&m_peeraddr)>>8,
        (0x00FF0000&m_peeraddr)>>16, (0xFF000000&m_peeraddr)>>24, m_port);
    return m_strAddr;
}

void CIOSession::setMessageSticker(stickMessageHandler sticker)
{
    m_sticker = sticker;
}
stickMessageHandler CIOSession::getMessageSticker() 
{ 
    return m_sticker;
}

void CIOSession::setEncodeWrapperFunc(encodeWrapperFunc encode) 
{ 
    m_encoderFunc = encode;
}

encodeWrapperFunc CIOSession::getEncodeWrapperFunc() 
{
    return m_encoderFunc;
}

SOCKET CIOSession::getSockFd()
{
    return m_socket;
}

void CIOSession::setRemote( unsigned long addr, int port )
{
    m_strAddr[0] = 0;
    m_peeraddr = addr;
    m_port = port;
}

void CIOSession::setStatus(ENUM_IOSESSION_STATUS status) 
{ 
    if (SESSION_CLOSING == status || SESSION_CLOSED == status)
    {
        stopLookupTimeoutRequest();
        m_peeraddr = 0;
        m_port = 0;
        m_strAddr[0] = 0;
        m_ioHandlerFunc = NULL;
        m_ioUserdata = NULL;
    }
    m_status = status;
}

ENUM_IOSESSION_STATUS CIOSession::getStatus()
{ 
    return m_status; 
}

void CIOSession::ResetBuffer()
{
    {//delete all the messages that not handled
        int actuaSize = 0;
        ACE_Guard<ACE_Mutex> guard(m_mutexQueue);
        while(!m_messageQueue.empty())
        {
            void* buffer = m_messageQueue.front();
            if (m_iSessionType == SES_TYPE_NPP)
            {
                NutsWrapperMessage* msg = reinterpret_cast<NutsWrapperMessage*>(buffer);
                if (NULL != msg)
                {
                    delete msg;
                    msg = NULL;
                }
            }else
            {
                char* rawbuf = reinterpret_cast<char*>(buffer);
                memcpy(&actuaSize, rawbuf+4, 4);
                GlobalOverlappedBufPool::instance()->freeRawBuffer(rawbuf, actuaSize);//delete raw message here
            }

            m_messageQueue.pop_front();
        }
        m_messageQueue.clear();
    }
}

bool CIOSession::insertMessageTask( void* pNutsWrapperMessage )
{
    if(NULL == pNutsWrapperMessage)
        return false;

    ACE_Guard<ACE_Mutex> guard(m_mutexQueue);
    m_messageQueue.push_back(pNutsWrapperMessage);

    return true;
}

NutsWrapperMessage* CIOSession::getMessageTask()
{
    NutsWrapperMessage* pNutsWrapperMessage = NULL;
    ACE_Guard<ACE_Mutex> guard(m_mutexQueue);
    if (!m_messageQueue.empty())
    {
        pNutsWrapperMessage = reinterpret_cast<NutsWrapperMessage*>(m_messageQueue.front());
        m_messageQueue.pop_front();
    }
    return pNutsWrapperMessage;
}

int CIOSession::getMessageTaskNum()
{
    ACE_Guard<ACE_Mutex> guard(m_mutexQueue);
    return m_messageQueue.size();
}

void CIOSession::setSockFd( int sessionId )
{
    m_socket = sessionId;
}

bool CIOSession::getMessageTaskQueue(std::list<void*>& messageQueue)
{
    ACE_Guard<ACE_Mutex> guard(m_mutexQueue);
    messageQueue = m_messageQueue;
    m_messageQueue.clear();
    return true;
}

void CIOSession::insertMessageTimeoutQueue(int sequence, char* timeoutMessage )
{
    if (!m_openTimeout)
        return;
    ACE_Guard<ACE_Mutex> guard(m_mutexTimeout);
    m_messageTimeoutQueue[sequence] = timeoutMessage;
}

bool CIOSession::ejectMessageTimeoutQueueWithSeq(int sequence)
{
    char* node = NULL;
    int len = 0;
    std::hash_map<int, char*>::iterator iter;
    bool bFind = false; 
    {
        ACE_Guard<ACE_Mutex> guard(m_mutexTimeout);
        iter =m_messageTimeoutQueue.find(sequence);
        if (iter != m_messageTimeoutQueue.end())
        {
            bFind = true;
            node = iter->second;
            m_messageTimeoutQueue.erase(iter);
        }
    }
    if (bFind)
    {
        memcpy(&len, node+8, sizeof(int));
        GlobalOverlappedBufPool::instance()->freeRawBuffer(node, len);
    }

    return bFind;
}

void CIOSession::cleanLocalRequest()
{
    char* node = NULL;
    int len = 0;
    //copy time-out queue, release queue lock
    ACE_Guard<ACE_Mutex> guard(m_mutexTimeout);
    std::hash_map<int, char*>::iterator iter = m_messageTimeoutQueue.begin();
    while (iter != m_messageTimeoutQueue.end())
    {//lookup from the queue head
        node = iter->second;
        memcpy(&len, node+8, sizeof(int));
        GlobalOverlappedBufPool::instance()->freeRawBuffer(node, len);
        iter++;
    }
    m_messageTimeoutQueue.clear();
}

void CIOSession::lookupLocalRequest()
{
    char* node = NULL;
    bool btimeout = false;
    std::hash_map<int, char*> timeoutQueue;
    {//copy time-out queue, release queue lock
        ACE_Guard<ACE_Mutex> guard(m_mutexTimeout);
        timeoutQueue = m_messageTimeoutQueue;
    }
    std::hash_map<int, char*>::iterator iter = timeoutQueue.begin();
    while (iter != timeoutQueue.end() && m_openTimeout)
    {//lookup from the queue head
        node = iter->second;

        int referencecnt = 0;
        int len = 0;
        memcpy(&referencecnt, node, sizeof(int));
        if (referencecnt > GlobalConfigure::instance()->getTimeoutLimit())
        {
            memcpy(&len, node+8, sizeof(int));
            GlobalNppMessageFactory::instance()->FireMessageEvent(node, len , this, true);
            //iter = m_messageTimeoutQueue.erase(iter);
            ejectMessageTimeoutQueueWithSeq(iter->first);
            btimeout = true;
        }
        else
        {
            referencecnt++;
            memcpy(node, &referencecnt, sizeof(int));
        }
        iter++;
    }
    if (btimeout)
    {
        GlobalNppThreadPool::instance()->waitAtSessionQueue(this);
    }
}

void __stdcall CIOSession::TimeoutQueueTimer(void* userdata, bool waitortime)
{
    if (NULL == userdata)
        return;
    CIOSession* pThis = reinterpret_cast<CIOSession*>(userdata);
    if (NULL == pThis)
        return;
    pThis->lookupLocalRequest();
}

void CIOSession::startLookupTimeoutRequest()
{
    if ( INVALID_HANDLE_VALUE == m_hTimer)
    {
        bool bret = GlobalTimerQueue::instance()->CreateTimer( m_hTimer, 
            TimeoutQueueTimer, this,  1000);
    }
    m_openTimeout = true;
}

void CIOSession::stopLookupTimeoutRequest()
{
    if (m_hTimer != INVALID_HANDLE_VALUE)
    {
        GlobalTimerQueue::instance()->DestroyTimer(m_hTimer);
        m_hTimer = INVALID_HANDLE_VALUE;
    }
    m_openTimeout = false;

    cleanLocalRequest();
}

bool CIOSession::isOpenTimeoutMechanism()
{
    return m_openTimeout;
}

void CIOSession::setIoSessionEventHandler( OnSessionEvent ioHandlerFunc, void* userdata )
{
    m_ioHandlerFunc = ioHandlerFunc;
    m_ioUserdata = userdata;
}

void CIOSession::FireSessionClosed()
{
    if (NULL != m_ioHandlerFunc)
    {
        m_ioHandlerFunc(m_identity, IO_CLOSED, m_ioUserdata, NPP_NOERROR);
    }
}
void CIOSession::FireSessionCreated()
{

}

void CIOSession::FireMessageRecved(const char* buffer, const int len)
{
    if (NULL == m_onNonNppMessageFunc)
        return;

    m_onNonNppMessageFunc(buffer, len, m_identity, m_ioUserdata);
}

void CIOSession::FireSessionOpened()
{
    if (NULL != m_ioHandlerFunc)
    {
        m_ioHandlerFunc(m_identity, IO_OPENED, m_ioUserdata, NPP_NOERROR);
    }
}

void CIOSession::FireSessionExcepted()
{
    ;
}

//**can be set only once
//*iSessionIdentity, session identity
//**no return
void CIOSession::setIdentity(const int& iSessionIdentity )
{
    if (-1 == m_identity)
    {
        m_identity = iSessionIdentity;
    }
}
//**acquire session identity
//**return identity of this session
int CIOSession::getIdentity()
{
    return m_identity;
}

void CIOSession::setSessionType( const int sessionType )
{
    m_iSessionType = sessionType;
}

int CIOSession::getSessionType()
{
    return m_iSessionType;
}

void CIOSession::increaseSendRef(){
    ACE_Guard<ACE_Mutex> guard(m_lSendRefMutex);
    m_lSockSendRef++;
}
void CIOSession::decreaseSendRef(){
    ACE_Guard<ACE_Mutex> guard(m_lSendRefMutex);
    m_lSockSendRef--;
    if (m_lSockSendRef <= 0 && m_status == SESSION_CLOSING)
    {
        closeSession(true);
    }
}

bool CIOSession::shutdownSession()
{
    LINGER linger = {1,0};
    int ret = ::setsockopt(m_socket, SOL_SOCKET, SO_LINGER,
        (char *)&linger, sizeof(linger));

    return (0==ret);
    //shutdown(m_socket, _SEND);
}

bool CIOSession::closeSession(bool bImediately)
{
    if (bImediately && INVALID_SOCKET != m_socket)
    {
        FireSessionClosed();
        shutdownSession();
        closesocket(m_socket);
        GlobalIoSessionMgr::instance()->freeIoSessionByID(m_identity);
    }
    else
    {
        m_status = SESSION_CLOSING;
        if (m_lSockSendRef <= 0 && INVALID_SOCKET != m_socket)
        {
            FireSessionClosed();
            shutdownSession();
            closesocket(m_socket);
            GlobalIoSessionMgr::instance()->freeIoSessionByID(m_identity);
        }
    }
    return true;
}

bool CIOSession::isAvailbale()
{
    if (m_status == SESSION_CLOSED || m_status == SESSION_CLOSING || m_socket == INVALID_SOCKET)
        return false;
    return true;
}

void CIOSession::setOnMessageFunc( OnNonNppMessage onNonNppMessage )
{
    m_onNonNppMessageFunc = onNonNppMessage;
}

OnNonNppMessage CIOSession::getOnMessageFunc()
{
    return m_onNonNppMessageFunc;
}
