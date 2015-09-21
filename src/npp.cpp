#include "npp.h"
#include "NppLog.h"
#include "Configure.h"
#include "WinAysncIOCP.h"
#include "NppThreadPool.h"
#include "IoOverlappedManager.h"
#include "IoSessionMgr.h"
#include "MessageCallbackMgr.h"
#include "NppMessageFactory.h"
#include "SockEnv.h"
#include "NppMessageTimerQueue.h"
#include "NppDumpSupport.h"

int NPP_DECLARE Npp_Init( int numOfProcessors, int numOfThreadPools)
{
    if(GlobalConfigure::instance()->isInitialized())
        return NPP_NOERROR;
    
    bool dump = true;
    if (dump)
        InitMinDump();
    

    ACE::init();
    AlogInit("log4cplus.properties");

    if(!CWinSockEnvironment::InitWinSock())
    {
        LOG_ERROR("initilize network environment error");
        return NPP_ENETWORKENV;
    }

    //initialize the buffer pool firstly
    GlobalOverlappedBufPool::instance()->initialize(true);

    //initialize the timer queue
    GlobalTimerQueue::instance()->Initialize();

    if (!GlobalSvrAysncMgr::instance()->InitSvrMgr())
    {
         LOG_ERROR("initilize asynchronous network environment error");
        return NPP_ENETWORKENV;
    }

    //initialize the thread pool
    GlobalNppThreadPool::instance()->Initialize(numOfThreadPools, numOfThreadPools+100);

    GlobalConfigure::instance()->setInitialized(true);

    LOG_INFO("initialize npp successfully ... ...");

    return NPP_NOERROR;
}
    
void NPP_DECLARE Npp_SetNppMessageConstructor(nppMessageConstructor defaultCreateMessage)
{
    GlobalMessageCallbackMgr::instance()->setMessageConstructor(defaultCreateMessage);
}

NPP_API bool NPP_DECLARE Npp_SetSessionOnExceptionAndClosed(IoEventHandler onExceptAndClosedFunc, void* userdata, int hHandler)
{
    if (INVALID_SOCKET != hHandler)
    {
        return GlobalIoSessionMgr::instance()->setIoEventHandlerBySockFd(hHandler, onExceptAndClosedFunc, userdata);
    }
    return false;
}

void NPP_DECLARE Npp_Fini()
{
    if(!GlobalConfigure::instance()->isInitialized())
        return;

    GlobalSvrAysncMgr::instance()->FiniSvrMgr();

    CWinSockEnvironment::FiniWinSock();

    GlobalOverlappedBufPool::instance()->finalize();

    GlobalTimerQueue::instance()->Finalize();

    ACE::fini();
}

int NPP_DECLARE Npp_RegisterCallback(int messageCode, DoMessageExecute onMessageFunc)
{
    if(!GlobalConfigure::instance()->isInitialized())
        return NPP_ENOTINITLIB;
    return GlobalMessageCallbackMgr::instance()->registerMessageCallback(messageCode, onMessageFunc);
}
    
NPP_SESSION_HANDLE NPP_DECLARE Npp_OpenLocalSvr(const char* hostname, int port, int& error, 
    bool bnpp, EncodeMessage encodeFunc , DecodeMessage decodeFunc )
{
    if(!GlobalConfigure::instance()->isInitialized())
    {
        error = NPP_ENOTINITLIB;
        return INVALID_NPP_HANDLE;
    }

    int sessionIdentity = GlobalSvrAysncMgr::instance()->StartServer(hostname, port);
    if (!bnpp && encodeFunc != NULL && decodeFunc != NULL)
    {
        CIOSession* pSession = GlobalIoSessionMgr::instance()->getOccupancyIoSessionByID(sessionIdentity);
        if (NULL != pSession)
        {
            pSession->setMessageSticker(decodeFunc);
            pSession->setEncodeWrapperFunc((encodeWrapperFunc)encodeFunc);
        }
    }

    return (NPP_SESSION_HANDLE)sessionIdentity;
}


int NPP_DECLARE Npp_CloseLocalSvr(NPP_SESSION_HANDLE handleSvr)
{
    if(!GlobalConfigure::instance()->isInitialized())
        return NPP_ENOTINITLIB;
    return GlobalSvrAysncMgr::instance()->StopServer((int)handleSvr);
}
    
NPP_SESSION_HANDLE NPP_DECLARE Npp_ConnRemoteSvr(
    const char* _in hostname, int _in port, int& _out error, 
    bool _in bnpp, bool bKeepalive , bool bAsync,
    OnSessionEvent _in sessionEvent , void* userdata, 
    EncodeMessage _in encodeFunc, DecodeMessage _in decodeFunc,
    OnNonNppMessage _in onMessageFunc )
{
    if(!GlobalConfigure::instance()->isInitialized())
    {
        error = NPP_ENOTINITLIB;
    }
    int sessionType = bnpp? SES_TYPE_NPP: SES_TYPE_NONNPP;
    int sessionIdentity = INVALID_NPP_HANDLE;
    if (bAsync)
    {
        sessionIdentity = GlobalSvrAysncMgr::instance()->ConnAsyncRemoteSvr(
            hostname, port, bKeepalive, 
            sessionEvent, userdata,
            encodeFunc,decodeFunc,
            sessionType, onMessageFunc);
    }
    else
    {
        sessionIdentity = GlobalSvrAysncMgr::instance()->ConnSyncRemoteSvr(
            hostname, port, bKeepalive, 
            sessionEvent, userdata,
            encodeFunc,decodeFunc,
            sessionType, onMessageFunc);
    }


    return (NPP_SESSION_HANDLE)sessionIdentity;
}


int NPP_DECLARE Npp_CloseRemoteSvr(NPP_SESSION_HANDLE handleChannel)
{
    if(!GlobalConfigure::instance()->isInitialized())
        return NPP_ENOTINITLIB;
    return GlobalSvrAysncMgr::instance()->DisconnRemoteSvr((int)handleChannel);
}

int NPP_DECLARE Npp_SendViaChannel(const void* rawMessage, const int& len, const NPP_SESSION_HANDLE handleChannel)
{
    if(!GlobalConfigure::instance()->isInitialized())
        return NPP_ENOTINITLIB;

    CIOSession* pChannelSession = GlobalIoSessionMgr::instance()->getOccupancyIoSessionByID(handleChannel);
    if (NULL == pChannelSession)
        return NPP_ENOTAVALIABLESRC;
    if (pChannelSession->getSessionType() == SES_TYPE_NPP)
        return NPP_EINVALIDSESSIONTYPE;

    bool bRet = GlobalSvrAysncMgr::instance()->AsyncSendWithBuffer(pChannelSession, (const char*)rawMessage, len, 0);
    if (!bRet)
        return NPP_ENOTSUPPORT;

    return NPP_NOERROR;
}
    
int NPP_DECLARE Npp_SendEncodeViaChannel(const void* rawMessage, const int& len , EncodeMessage encodeFunc , const NPP_SESSION_HANDLE handleChannel)
{
    if(!GlobalConfigure::instance()->isInitialized())
        return NPP_ENOTINITLIB;

    return NPP_ENOTSUPPORT;
}
    

int NPP_DECLARE Npp_SendNppMSGViaChannel(void* nppMessage, const unsigned long messageCode , 
    const NPP_SESSION_HANDLE& handleChannel, bool bWithRsp, int sequenceNum)
{
    if(!GlobalConfigure::instance()->isInitialized())
        return NPP_ENOTINITLIB;
    if (NULL == nppMessage)
        return NPP_EINVALIDPARAM;

    CIOSession* pChannelSession = GlobalIoSessionMgr::instance()->getOccupancyIoSessionByID(handleChannel);
    if (NULL == pChannelSession)
        return NPP_ENOTAVALIABLESRC;
    if (pChannelSession->getSessionType() != SES_TYPE_NPP)
        return NPP_EINVALIDSESSIONTYPE;

    MessageType msgType = UNKNOWN_MSGTYPE;
    google::protobuf::Message* nppMessageEntity = reinterpret_cast<google::protobuf::Message*>(nppMessage);
    std::string sname = nppMessageEntity->GetTypeName();

    NutsWrapperMessage wrapperMessage;
    wrapperMessage.set_messagecode(messageCode);
    int sequenceNumT = sequenceNum;
    if (-1 == sequenceNum)
        sequenceNumT = CNppUtils::getSequence();
    wrapperMessage.set_sequencenum(sequenceNumT);
    wrapperMessage.set_messagename(nppMessageEntity->GetTypeName());
    if (bWithRsp) msgType = REQUEST_WITH_RSP;
    else msgType = REQUEST_WITHOUT_RSP;
    wrapperMessage.set_type(msgType);
    wrapperMessage.set_messageentity(nppMessageEntity->SerializeAsString());

    encodeWrapperFunc encodeFunc = pChannelSession->getEncodeWrapperFunc();
    if (NULL == encodeFunc)
        return NPP_ELACKNECESSARYSRC;

    char* sendbuffer = NULL;
    int bytesActuallySize = 0;
    int bytesWriteThisTime = 0;
    if(!encodeFunc(&wrapperMessage, 0, &sendbuffer, bytesActuallySize, bytesWriteThisTime) )
        return NPP_EINVALIDPARAM;

    char* timeoutMessage = NULL;
    int expectedSize = bytesWriteThisTime + 12;
    int actualSize = 0;
    DoMessageExecute callback = GlobalMessageCallbackMgr::instance()->getMessageCallbackByCode(messageCode);
    if (REQUEST_WITH_RSP == msgType && NULL != callback && pChannelSession->isOpenTimeoutMechanism())
    {//this message want to received response , should stay into time-out queue if callback is set
        timeoutMessage = GlobalOverlappedBufPool::instance()->allocateRawBuffer(expectedSize, actualSize);
        int referencecnt = 0;
        memcpy(timeoutMessage ,&referencecnt, 4);                     //reference count number
        memcpy(timeoutMessage+4, &sequenceNumT, 4); //sequence
        memcpy(timeoutMessage+8, &actualSize, 4);          //buffer len
        memcpy(timeoutMessage+12, sendbuffer, bytesWriteThisTime);
        pChannelSession->insertMessageTimeoutQueue(sequenceNumT , timeoutMessage);
    }

    if(!GlobalSvrAysncMgr::instance()->AsyncSendWithBuffer(pChannelSession, sendbuffer, bytesWriteThisTime, bytesActuallySize))
    {
        if (NULL != timeoutMessage)
            pChannelSession->ejectMessageTimeoutQueueWithSeq(sequenceNumT);
        
        return NPP_ENOTAVALIABLESRC;
    }
    
    return NPP_NOERROR;
}

int NPP_DECLARE Npp_SendNppMSG(WrapperNppMessage* nppWrapperMessage)
{
    if(!GlobalConfigure::instance()->isInitialized())
        return NPP_ENOTINITLIB;
    if (NULL == nppWrapperMessage || NULL == nppWrapperMessage->getMessageEntity())
        return NPP_EINVALIDPARAM;

    CIOSession* pChannelSession = GlobalIoSessionMgr::instance()->getOccupancyIoSessionByID(nppWrapperMessage->getDestination());
    if (NULL == pChannelSession)
        return NPP_ENOTAVALIABLESRC;
    if (pChannelSession->getSessionType() != SES_TYPE_NPP)
        return NPP_EINVALIDSESSIONTYPE;

    MessageType msgType = UNKNOWN_MSGTYPE;
    msgType = (MessageType)nppWrapperMessage->getMessageType();
    if(msgType != REQUEST_WITHOUT_RSP && msgType != REQUEST_WITH_RSP && msgType != RESPONSE_NORMAL)
    {
        return NPP_EINVALIDPARAM;
    }

    if(nppWrapperMessage->getDestination() == INVALID_SOCKET)
    {
        return NPP_EINVALIDPARAM;
    }

    google::protobuf::Message* nppMessageEntity = reinterpret_cast<google::protobuf::Message*>(nppWrapperMessage->getMessageEntity());

    NutsWrapperMessage wrapperMessage;
    int messageCode = nppWrapperMessage->getMessageCode();
    wrapperMessage.set_messagecode(messageCode);
    int sequenceNumT = nppWrapperMessage->getMessageSeq();
    if (sequenceNumT < 0) sequenceNumT = CNppUtils::getSequence();
    wrapperMessage.set_sequencenum(sequenceNumT);
    wrapperMessage.set_messagename(nppMessageEntity->GetTypeName());
    wrapperMessage.set_type(msgType);
    wrapperMessage.set_messageentity(nppMessageEntity->SerializeAsString());

    encodeWrapperFunc encodeFunc = pChannelSession->getEncodeWrapperFunc();
    if (NULL == encodeFunc)
        return NPP_ELACKNECESSARYSRC;
    
    char* sendbuffer = NULL;
    int bytesActuallySize = 0;
    int bytesWriteThisTime = 0;
    if(!encodeFunc(&wrapperMessage, 0, &sendbuffer, bytesActuallySize, bytesWriteThisTime) )
    {
        return NPP_EINVALIDPARAM;
    }

    char* timeoutMessage = NULL;
    int expectedSize = bytesWriteThisTime + 12;
    int actualSize = 0;
    DoMessageExecute callback = GlobalMessageCallbackMgr::instance()->getMessageCallbackByCode(messageCode);
    if (REQUEST_WITH_RSP == msgType && NULL != callback && pChannelSession->isOpenTimeoutMechanism())
    {//this message want to received response , should stay into time-out queue
        timeoutMessage = GlobalOverlappedBufPool::instance()->allocateRawBuffer(expectedSize, actualSize);
        memcpy(timeoutMessage ,0, 4);                     //reference count number
        memcpy(timeoutMessage+4, &sequenceNumT, 4); //sequence
        memcpy(timeoutMessage+8, &actualSize, 4); //buffer len
        memcpy(timeoutMessage+12, sendbuffer, bytesWriteThisTime);
        pChannelSession->insertMessageTimeoutQueue(sequenceNumT, timeoutMessage);
    }

    if(!GlobalSvrAysncMgr::instance()->AsyncSendWithBuffer(pChannelSession, sendbuffer, bytesWriteThisTime, bytesActuallySize))
    {
        if (NULL != timeoutMessage)
            pChannelSession->ejectMessageTimeoutQueueWithSeq(sequenceNumT);
        
        return NPP_ENOTAVALIABLESRC;
    }
    
    return NPP_NOERROR;
}

int CNppUtils::getSequence()
{
    static int ipos = 1;
    if (ipos > 65535)
        ipos = 1;
    return ipos++;
}

int CNppUtils::getCpuUsage(){
    //////////////////////////////////////////////////////////////////////////
    return 0;
}
int CNppUtils::getMemUsage(){
    //////////////////////////////////////////////////////////////////////////
    return 0;
}
unsigned long long CNppUtils::getCurrentTime(){
    return GetCurrentTime();
}

WrapperNppMessage::WrapperNppMessage():m_code(0),
    m_destination(INVALID_SOCKET),
    m_source(INVALID_SOCKET),
    m_sequence(-1),
    m_type(UNKNOWN_MSGTYPE),
    m_nppmessage(NULL)
{

}

WrapperNppMessage::~WrapperNppMessage()
{

}

void WrapperNppMessage::setMessageType(const int& type)
{
    m_type = type;
}
int WrapperNppMessage::getMessageType()
{
    return m_type;
}

void WrapperNppMessage::setMessageCode(const int& code)
{
    m_code = code;
}
int WrapperNppMessage::getMessageCode()
{
    return m_code;
}

void WrapperNppMessage::setMessageSeq(const int& sequence)
{
    m_sequence = sequence;
}
int WrapperNppMessage::getMessageSeq()
{
    return m_sequence;
}

void WrapperNppMessage::setDestination(const int& dst)
{
    m_destination = dst;
}
int WrapperNppMessage::getDestination()
{
    return m_destination;
}

void WrapperNppMessage::setSource(const int& src)
{
    m_source = src;
}
int WrapperNppMessage::getSource()
{
    return m_source;
}

void WrapperNppMessage::setMessageEntity(void* npp)
{
    m_nppmessage = npp;
}
void* WrapperNppMessage::getMessageEntity()
{
    return m_nppmessage;
}

bool NPP_DECLARE Npp_CreateTimer(NPP_HANDLE& _out hTimer,NppWaitOrTimerCallBack _in timerCallback, void* _in userdata, unsigned int _in period)
{
    return GlobalTimerQueue::instance()->CreateTimer(hTimer, timerCallback, userdata, period);
}

bool NPP_DECLARE Npp_DestroyTimer(NPP_HANDLE _in hTimer)
{
    return GlobalTimerQueue::instance()->DestroyTimer(hTimer);
}


void __stdcall nppWaitOrTimerCallBack(void* pUserdata, bool bTimeOrWait)
{
    char* buffer = reinterpret_cast<char*>(pUserdata);
    LOG_DEBUG("%s", buffer);
}

void NPP_DECLARE Npp_InnerTEST()
{

#ifdef TEST_TIMER
    NPP_HANDLE hTimer = INVALID_HANDLE_VALUE;
    char* buffer = new char[64];
    memset(buffer, 0, 64);
    ACE_OS::snprintf(buffer, 64, "%s", "this is a timer test application");
    GlobalTimerQueue::instance()->CreateTimer(hTimer, nppWaitOrTimerCallBack, buffer , 3000);

    system("PAUSE");
    if (INVALID_HANDLE_VALUE != hTimer)
    {
        GlobalTimerQueue::instance()->DestroyTimer(hTimer);
        delete buffer;
    }
#endif
}