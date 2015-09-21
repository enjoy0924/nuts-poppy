#include "WinAysncIOCP.h"
#include "IoSessionMgr.h"
#include "NppLog.h"
#include "Configure.h"
#include "mstcpip.h"
#include "IoOverlappedManager.h"
#include "NppMessageFactory.h"
#include "NppThreadPool.h"

#define WORKER_THREADS_PER_PROCESSOR 2
#define MAX_POST_ACCEPT 10

#define EXIT_CODE NULL

// release pointer
#define RELEASE(x) {if(x != NULL ){delete x;x=NULL;}}
// release handle
#define RELEASE_HANDLE(x) {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
// release socket
#define RELEASE_SOCKET(x) {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}



CAysncIOCPSvrMgr::CAysncIOCPSvrMgr(void):
                            m_nThreads(0),
                            m_hShutdownEvent(INVALID_HANDLE_VALUE),
                            m_hIOCompletionPort(INVALID_HANDLE_VALUE),
                            m_phWorkerThreads(NULL),
                            m_lpfnAcceptEx( NULL ),
                            m_lpfnGetAcceptExSockAddrs(NULL)
{
}


CAysncIOCPSvrMgr::~CAysncIOCPSvrMgr(void)
{
}

//io processor execute here
DWORD WINAPI CAysncIOCPSvrMgr::_IoProcessorThread(LPVOID lpParam)
{
    if (NULL == lpParam)
        return 0;
    THREADPARAMS_WORKER* pParam =reinterpret_cast<THREADPARAMS_WORKER*>(lpParam);
    CAysncIOCPSvrMgr* pIOCPModel = (CAysncIOCPSvrMgr*)pParam->pIOCPModel;
    int nThreadNo = (int)pParam->nThreadNo;

    LOG_DEBUG("io processor [%d] started ... ...", nThreadNo);

    OVERLAPPED *pOverlapped = NULL;  //one pointer point to overlapped
    CIOSession *pIoSession = NULL;         //one pointer point to io session
    DWORD dwBytesTransfered = 0;        //how many bytes have been transfered on some socket in different operation type

    //loop forever until exit or shutdown single received
    while (WAIT_OBJECT_0 != WaitForSingleObject(pIOCPModel->m_hShutdownEvent, 0))
    {
        BOOL bReturn = GetQueuedCompletionStatus(
            pIOCPModel->m_hIOCompletionPort,
            &dwBytesTransfered,
            (PULONG_PTR)&pIoSession,
            &pOverlapped,
            INFINITE);
        
        if ( EXIT_CODE==(DWORD)pIoSession ) // get a exit single, break right now, this single sent by inner system
        {
            LOG_DEBUG("accept exit code, iocp will exit soon!");
            break;
        }

        //require the value of m_overlapped (member of PER_IO_CONTEXT)
        IO_OVERLAPPEDPLUS* pIoOverlapped = CONTAINING_RECORD(pOverlapped, IO_OVERLAPPEDPLUS, _overlapped);  

        if (NULL != pIoSession &&OP_SEND == pIoOverlapped->_opertype)
            pIoSession->decreaseSendRef();

        if( !bReturn )
        {//the io completion port isn't return true
            //GlobalOverlappedPool::instance()->freeOverlapped(pIoOverlapped);
            DWORD dwErr = GetLastError();
            if( !pIOCPModel->_HandleError( pIoSession, dwErr))
            {
                LOG_ERROR("critical error happened, application exit");
                break;
            }
            continue;
        }  
        else  
        {
            if (NULL == pIoSession)
            {
                LOG_ERROR("completion port key is null! break iocp handler");
                break;
            }

            if((0 == dwBytesTransfered) && ( OP_RECV==pIoOverlapped->_opertype || OP_SEND==pIoOverlapped->_opertype))  
            {//remove this session
                LOG_ERROR( "client %s disconnected.", pIoSession->getRemote());
                pIoSession->closeSession();
                
                continue;
            }
            else
            {
                pIoOverlapped->_nBytesTransferedThisTime = dwBytesTransfered;
                switch( pIoOverlapped->_opertype )  
                {  
                case OP_ACCEPT:
                    pIOCPModel->_DoAccpet( pIoSession, pIoOverlapped);
                    break;
                case OP_CONN:
                    pIOCPModel->_DoConn( pIoSession, pIoOverlapped);
                    break;
                case OP_RECV:
                    pIoOverlapped->_nBufferTotalBytes+= dwBytesTransfered;
                    pIOCPModel->_DoRecv( pIoSession, pIoOverlapped);
                    break;
                case OP_SEND:
                    GlobalOverlappedPlusMgr::instance()->freeOccupancyIoOverlapped(pIoOverlapped);
                    break;
                default:
                    LOG_ERROR("should not be here, io overlap parameters error!.");
                    break;
                } 
            }
        }
    }

    LOG_ERROR("io processor [%d] exit!", nThreadNo);

    RELEASE(lpParam);
    return 0;
}


bool CAysncIOCPSvrMgr::InitSvrMgr()
{
    m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (INVALID_HANDLE_VALUE == m_hShutdownEvent)
    {
        return false;
    }

    if (!_InitializeIOCP())
    {
        LOG_ERROR("initialize iocp error!");
        return false;
    }
        
    return true;
}


bool CAysncIOCPSvrMgr::StopServer(int hSvr)
{
    if (INVALID_SOCKET != hSvr)
    {
        GlobalIoSessionMgr::instance()->freeIoSessionByID(hSvr);
    }
    
    return true;
}

////////////////////////////////////////////////////////////////////
void CAysncIOCPSvrMgr::FiniSvrMgr()
{
    //Fire a shutdown notification out
    SetEvent(m_hShutdownEvent);

    for (int i = 0; i < m_nThreads; i++)
    {//notify all processor thread to exit
        PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
    }

    WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);

    _DeInitialize();
}

int CAysncIOCPSvrMgr::StartServer(const char* hostname, const int& port, const int acceptnum)
{
    // AcceptEx  GetAcceptExSockaddrs GUID, export the function pointer 
    GUID GuidAcceptEx = WSAID_ACCEPTEX;  
    GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 

    // IO overlapped, must invoke WSASocket to get new socket or not support overlapped
    SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == listenSocket) 
    {
        LOG_ERROR("Initialize asynchronous socket failed£¬lasterror: %d", WSAGetLastError());
        return INVALID_NPP_HANDLE;
    }

    CIOSession* pListenSession = GlobalIoSessionMgr::instance()->getAvailableIoSession(listenSocket);
    if (NULL == pListenSession)
    {
        LOG_ERROR("can not create io session");
        closesocket(listenSocket);
        return INVALID_NPP_HANDLE;
    }

    pListenSession->FireSessionCreated();

    bool isEverythingOk = false;
    do 
    {
        if (!_AssociateWithIOCP(pListenSession))
        {
            LOG_ERROR("assocaiate listen socket into iocp port error£¡lasterror: %d", WSAGetLastError());
            break;
        }

        struct sockaddr_in ServerAddress;
        ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
        ServerAddress.sin_family = AF_INET;
        ServerAddress.sin_addr.s_addr = inet_addr(hostname);
        ServerAddress.sin_port = htons(port);

        pListenSession->setRemote(inet_addr(hostname), port);
        pListenSession->FireSessionCreated();
        if (SOCKET_ERROR == bind(pListenSession->getSockFd(), (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))) 
        {
            LOG_ERROR("bind() socket system function invoke error!");
            break;
        }

        if (SOCKET_ERROR == listen(pListenSession->getSockFd(), SOMAXCONN))
        {
            LOG_ERROR("listen() system function invoke error!");
            break;
        }

        DWORD dwBytes = 0;
        if(SOCKET_ERROR == WSAIoctl(
            pListenSession->getSockFd(), SIO_GET_EXTENSION_FUNCTION_POINTER, 
            &GuidAcceptEx, sizeof(GuidAcceptEx), 
            &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), 
            &dwBytes, NULL, NULL))  
        {  
            LOG_ERROR("WSAIoctl can not aquire AcceptEx() function pointer, lasterror: %d", WSAGetLastError()); 
            break;
        }  

        if(SOCKET_ERROR == WSAIoctl(
            pListenSession->getSockFd(),  SIO_GET_EXTENSION_FUNCTION_POINTER, 
            &GuidGetAcceptExSockAddrs, sizeof(GuidGetAcceptExSockAddrs), 
            &m_lpfnGetAcceptExSockAddrs, sizeof(m_lpfnGetAcceptExSockAddrs),   
            &dwBytes, NULL, NULL))  
        {  
            LOG_ERROR("WSAIoctl can not aquire GuidGetAcceptExSockAddrs() function pointer, lasterror: %d\n", WSAGetLastError());  
            break;
        }  
        pListenSession->FireSessionOpened();

        for( int i=0;i<acceptnum;i++ )
        {
            IO_OVERLAPPEDPLUS* pAcceptIoOverlapped = GlobalOverlappedPlusMgr::instance()->getAvailableIoOverlapped(
                pListenSession->getSockFd(),
                GlobalConfigure::instance()->getRecvBuffer()
                );
            if(!_PostAccept(pListenSession, pAcceptIoOverlapped ) )
            {
                break;
            }
        }

        isEverythingOk = true;
    } while (false);
    
    if (isEverythingOk)
    {
        LOG_DEBUG("open service successfully, listen on [%s]", pListenSession->getRemote());
        pListenSession->setStatus(SESSION_OPENED);
        return pListenSession->getIdentity();
    }
    
    pListenSession->closeSession();

    return INVALID_NPP_HANDLE;
}

bool CAysncIOCPSvrMgr::_InitializeIOCP()
{
    m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 );
    if ( INVALID_HANDLE_VALUE == m_hIOCompletionPort)
    {
        LOG_ERROR("create io completion port failed, lasterror: %d!", WSAGetLastError());
        return false;
    }

    m_nThreads = WORKER_THREADS_PER_PROCESSOR * _GetNoOfProcessors();
    //handle array for io processor thread handle
    m_phWorkerThreads = new(std::nothrow) HANDLE[m_nThreads];

    DWORD nThreadID = 0;
    for (int i = 1; i <= m_nThreads; i++)
    {
        m_phWorkerThreads[i] = INVALID_HANDLE_VALUE;
        THREADPARAMS_WORKER* pThreadParams = new THREADPARAMS_WORKER;
        pThreadParams->pIOCPModel = this;
        pThreadParams->nThreadNo  = i+1;
        m_phWorkerThreads[i] = ::CreateThread(0, 0, _IoProcessorThread, (void *)pThreadParams, 0, &nThreadID);
    }

    return true;
}


void CAysncIOCPSvrMgr::_DeInitialize()
{
    LOG_DEBUG("recycle iocp resources ... ...");
}


bool CAysncIOCPSvrMgr::_PostAccept( CIOSession* pListenIoSession, IO_OVERLAPPEDPLUS* pAcceptOverlapped)
{
    if (NULL == pListenIoSession || INVALID_SOCKET==pListenIoSession->getSockFd())
    {
        LOG_ERROR("invalid socket handle or io overlap is null, invalid parameters");
        return false;
    }
    
    if (NULL == pAcceptOverlapped)
        return false;
    //preallocate socket file description for client
    pAcceptOverlapped->_opertype = OP_ACCEPT;
    pAcceptOverlapped->_sclient = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);  
    if( INVALID_SOCKET==pAcceptOverlapped->_sclient)  
    {  
        LOG_ERROR("preallocate client Accept socket failed, lasterror: %d", WSAGetLastError()); 
        return false; 
    }

    DWORD dwBytes = 0;
    pAcceptOverlapped->_opertype = OP_ACCEPT;
    if(!m_lpfnAcceptEx( pListenIoSession->getSockFd(), 
        pAcceptOverlapped->_sclient, 
        pAcceptOverlapped->_wsabuf.buf, 
        pAcceptOverlapped->_wsabuf.len - ((sizeof(SOCKADDR_IN)+16)*2),
        sizeof(SOCKADDR_IN)+16, 
        sizeof(SOCKADDR_IN)+16, 
        &dwBytes, 
        &(pAcceptOverlapped->_overlapped)))
    {  
        if(WSA_IO_PENDING != WSAGetLastError())  
        {  
            LOG_ERROR("post AcceptEx request failed, lasterror: %d", WSAGetLastError());  
            return false;  
        }
    }

    return true;
}

bool CAysncIOCPSvrMgr::_DoConn( CIOSession* pSocketSession, IO_OVERLAPPEDPLUS* pIoOverlappedPlus)
{
    pSocketSession->setStatus(SESSION_OPENED);
    pSocketSession->FireSessionOpened();

    if (!pSocketSession->isAvailbale())
        return false;
    if(!_PostRecv(pSocketSession, pIoOverlappedPlus))
    {
        LOG_ERROR("post receive operation on %s failed!", pSocketSession->getRemote());
        return false;
    }
    return true;
}

bool CAysncIOCPSvrMgr::_DoAccpet( CIOSession* pIoSession, IO_OVERLAPPEDPLUS* pIoOverlapped)
{
    SOCKADDR_IN* ClientAddr = NULL;
    SOCKADDR_IN* LocalAddr = NULL;
    int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);  

    //get remote and local address information here, get first data from the remote peer here
    this->m_lpfnGetAcceptExSockAddrs(pIoOverlapped->_wsabuf.buf,   //data buffer
        pIoOverlapped->_wsabuf.len - ((sizeof(SOCKADDR_IN)+16)*2),  //data buffer size
        sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16,             //address length
        (LPSOCKADDR*)&LocalAddr, &localLen,                           //local address information here
        (LPSOCKADDR*)&ClientAddr, &remoteLen);              //remote address information here
    
    do 
    {
        CIOSession* pIoClientSession = GlobalIoSessionMgr::instance()->getAvailableIoSession(pIoOverlapped->_sclient);
        if (NULL == pIoClientSession)
        {
            LOG_ERROR("can not create io session !");
            closesocket(pIoOverlapped->_sclient);
            break;
        }
        pIoClientSession->setRemote(ClientAddr->sin_addr.S_un.S_addr, ntohs(ClientAddr->sin_port));

        if( !_AssociateWithIOCP(pIoClientSession) )
        {
            LOG_DEBUG("associate this session failed! destroy this session");
            pIoClientSession->closeSession();
            break;
        }

        IO_OVERLAPPEDPLUS* pClientIoOverlapped = GlobalOverlappedPlusMgr::instance()->getAvailableIoOverlapped(
            pIoClientSession->getSockFd(),
            GlobalConfigure::instance()->getRecvBuffer()
            );
        if (NULL == pClientIoOverlapped)
            break;
        pIoClientSession->setStatus(SESSION_OPENED);
        pClientIoOverlapped->_opertype = OP_RECV;
        
        //if buffer is usable, then copy it, better not handle things here*************
        if (pIoOverlapped->_nBytesTransferedThisTime > 0)
        {//if there is some data in buffer, handle it here, then post a receive operation
            memcpy(pClientIoOverlapped->_pOriginalBuffer,pIoOverlapped->_pOriginalBuffer, pIoOverlapped->_nBytesTransferedThisTime);
            pClientIoOverlapped->_nBufferTotalBytes = pIoOverlapped->_nBytesTransferedThisTime;
            pClientIoOverlapped->_nBufferTotalBytes = pIoOverlapped->_nBytesTransferedThisTime;
            //is it necessary here? memset the buffer??????????????????????
            ZeroMemory(pIoOverlapped->_pOriginalBuffer, pIoOverlapped->_nActuallyBufferSize);

            if(!_DoRecv(pIoClientSession, pClientIoOverlapped))
            {
                 //LOG_ERROR("prepare to handle the first data from client , error happened, destroy this session");
                 break;
            }
        }
        else if( !_PostRecv(pIoClientSession, pClientIoOverlapped) )
        {
            LOG_ERROR("post recieve operation to client , error happened, destroy this session");
             break;
        }
    } while (false);
    
    return _PostAccept(pIoSession, pIoOverlapped);
}

bool CAysncIOCPSvrMgr::_PostRecv(CIOSession* pSocketContext, IO_OVERLAPPEDPLUS* pIoOverlapped )
{
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    if (NULL == pIoOverlapped || NULL == pSocketContext)
        return false;

    if (!pSocketContext->isAvailbale())
    {
        LOG_DEBUG("unavailable session, can not send any message!");
        return false;
    }

    //is it necessary here? memset the buffer??????????????????????
    //ZeroMemory(pIoOverlapped->_wsabuf.buf, pIoOverlapped->_wsabuf.len);
    pIoOverlapped->_opertype = OP_RECV;

    int nBytesRecv = WSARecv( pIoOverlapped->_s, &pIoOverlapped->_wsabuf, 1, 
        &dwBytes, &dwFlags, &pIoOverlapped->_overlapped, NULL );

    if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
    {
        pSocketContext->closeSession();
        LOG_ERROR("post WSARecv() failed! lasterror: %d", WSAGetLastError());
        return false;
    }

    return true;
}
//need to modify here
bool CAysncIOCPSvrMgr::_PostSend(CIOSession* pSocketContext, IO_OVERLAPPEDPLUS* pIoOverlapped )
{
    //if (NULL == pIoOverlapped || NULL == pSocketContext || 
    //    pSocketContext->getStatus() != SESSION_CONNECTED)
    //    return false;

    //DWORD dwFlags = 0;
    //DWORD dwSendNumBytes = 0;
    //pIoOverlapped->_opertype = OP_SEND;

    //SOCKET sock = pIoOverlapped->_s;
    //WSABUF* buffer = &(pIoOverlapped->_wsabuf);
    //OVERLAPPED* overlapped = &(pIoOverlapped->_overlapped);

    //int nRet = WSASend(sock, buffer, 1, &dwSendNumBytes, dwFlags, overlapped, NULL);

    //if ((SOCKET_ERROR == nRet) && (WSA_IO_PENDING != WSAGetLastError()))
    //{
    //    int lasterror = WSAGetLastError();
    //    //shutdown(pIoOverlapped->_s, SD_SEND);
    //    //closesocket(pIoOverlapped->_s);
    //    //pSocketContext->setStatus(ENUM_IOSESSION_STATUS::SESSION_CLOSING);
    //    LOG_ERROR("post send failed, return code: %d, lasterror: %d", nRet, lasterror);
    //    //LOG_ERROR("network error happened, recycle the send overlapped plus information!");
    //    //GlobalOverlappedPool::instance()->freeOverlapped(pIoOverlapped);
    //    return false;
    //}
    LOG_ERROR("this interface is not supported!");
    return true;
}

bool CAysncIOCPSvrMgr::_DoRecv( CIOSession* pClientSession, IO_OVERLAPPEDPLUS* pIoOverlapped)
{
    bool bParseOk = true;
    int stickpos = 0;        //the stick position
    int pos = 0;               //the entire buffer's position
    stickMessageHandler strickerFunc = pClientSession->getMessageSticker();
    if (NULL == strickerFunc)
    {
        pos = pIoOverlapped->_nBufferTotalBytes;
        GlobalNppMessageFactory::instance()->FireMessageEvent(
            pIoOverlapped->_pOriginalBuffer,
            pos,pClientSession);
        bParseOk = false;
    }

    bool skip = false;      //whether should ship this message
    while(bParseOk && pClientSession->isAvailbale())
    {
        stickpos = 0;
        skip = false;
        
        if (pIoOverlapped->_nBufferTotalBytes - pos == 0)
            break;

        bParseOk = strickerFunc(pIoOverlapped->_pOriginalBuffer+pos, //buffer with specified position
            pIoOverlapped->_nBufferTotalBytes - pos,  //the remaining buffer size
            stickpos,           //it's an output value, tag the message length
            skip);               //if message check sum is wrong, set this to be true

        if (bParseOk && pos+stickpos <= pIoOverlapped->_nBufferTotalBytes)
        {
            //throw out one completed message
            GlobalNppMessageFactory::instance()->FireMessageEvent(
                pIoOverlapped->_pOriginalBuffer+pos,
                stickpos,pClientSession);
            
            pos+=stickpos;            //move the buffer position to next buffer start
        }
    }

    if ( stickpos < 0)
    {    
        //pClientSession->ResetBuffer();
        LOG_ERROR("reset recieve buffer here, maybe should cut its connection here!");
    }

    if (stickpos>=0)
    {//handle the remaining information here
        int bufferRemainSize = pIoOverlapped->_nBufferTotalBytes - pos;
        if (bufferRemainSize >= 0)
        {
            memmove(pIoOverlapped->_pOriginalBuffer, pIoOverlapped->_pOriginalBuffer+pos, bufferRemainSize);
            //is it necessary here? memset the buffer
            //memset(pIoOverlapped->_pOriginalBuffer+bufferRemainSize, 0, pIoOverlapped->_nActuallyBufferSize-bufferRemainSize);
            pIoOverlapped->_nBufferTotalBytes = bufferRemainSize;
            pIoOverlapped->_nRemainBytesLastTime = bufferRemainSize;
            pIoOverlapped->_wsabuf.buf = pIoOverlapped->_pOriginalBuffer+bufferRemainSize;
            pIoOverlapped->_wsabuf.len = pIoOverlapped->_nActuallyBufferSize - bufferRemainSize;
        }
        else
        {
            LOG_ERROR("something wrong here, reset recv buffer here ... ... !!!");
            pIoOverlapped->_nBufferTotalBytes = 0;
            pIoOverlapped->_wsabuf.buf = pIoOverlapped->_pOriginalBuffer;
            pIoOverlapped->_wsabuf.len = pIoOverlapped->_nActuallyBufferSize;
        }
    }

    GlobalNppThreadPool::instance()->waitAtSessionQueue(pClientSession);
    
    return _PostRecv(pClientSession, pIoOverlapped );
}

bool CAysncIOCPSvrMgr::_DoSend( CIOSession* pClientSession, IO_OVERLAPPEDPLUS* pIoOverlapped )
{
    int startPos = pIoOverlapped->_nRemainBytesLastTime; //buffer start position
    int remainSize = pIoOverlapped->_nBufferTotalBytes - pIoOverlapped->_nRemainBytesLastTime;
    if (remainSize > 0)
    {//there is still some data in this buffer, post send again
        pIoOverlapped->_wsabuf.buf = pIoOverlapped->_pOriginalBuffer + startPos;
        pIoOverlapped->_wsabuf.len = remainSize;

        return _PostSend(pClientSession, pIoOverlapped);
    }

    return true;
}


bool CAysncIOCPSvrMgr::_AssociateWithIOCP( CIOSession *pIoSession )
{
    HANDLE hTemp = CreateIoCompletionPort((HANDLE)pIoSession->getSockFd(), m_hIOCompletionPort,
        (DWORD)pIoSession, 0);

    if (INVALID_HANDLE_VALUE == hTemp)
    {
        LOG_ERROR("associate with iocp error, lasterror code %d",WSAGetLastError());
        return false;
    }

    return true;
}

int CAysncIOCPSvrMgr::_GetNoOfProcessors()
{
    SYSTEM_INFO si;

    GetSystemInfo(&si);

    return si.dwNumberOfProcessors;
}

//another peer can't receive notification if some socket has exception, like crush or poll off network wire
bool CAysncIOCPSvrMgr::_IsSocketAlive(SOCKET s)
{
    int nByteSent=send(s,"",0,0);
    if (-1 == nByteSent) return false;
    return true;
}


bool CAysncIOCPSvrMgr::_HandleError( CIOSession *pIoSession, const DWORD& dwErr )
{
    if(WAIT_TIMEOUT == dwErr)
    {
        if( !_IsSocketAlive( pIoSession->getSockFd()) )
        {
            pIoSession->closeSession();
            LOG_DEBUG("peer exit without any notification, dead connect will be kicked out!");
            return true;
        }
        else
        {
            LOG_DEBUG("network operation time out, retrying... ...");
            return true;
        }
    }
    else if( ERROR_NETNAME_DELETED==dwErr )
    {//if there're many io overlapped here, it will be invoke many times
        pIoSession->closeSession();
        return true;
    }
    else if (ERROR_OPERATION_ABORTED==dwErr)
    {
        pIoSession->closeSession();
        return true;
    }
    else if(ERROR_CONNECTION_REFUSED)
    {
        LOG_DEBUG("remote refuse this connection");
        pIoSession->closeSession();
        return true;
    }
    else
    {
        LOG_DEBUG("io completion operation error happened, io processor thread will exit, error code: %d!", dwErr);
        return false;
    }
}

bool CAysncIOCPSvrMgr::AsyncSendWithBuffer(CIOSession* pIoSession,const char* buffer, const int buferSize, const int actullyBufSize )
{
    if (NULL == pIoSession || !pIoSession->isAvailbale())
        return false;

    DWORD dwFlags = 0;
    DWORD dwSendNumBytes = 0;

    IO_OVERLAPPEDPLUS* pIoOverlapped = GlobalOverlappedPlusMgr::instance()->getAvailableIoOverlapped(pIoSession->getSockFd());
    if (NULL == pIoOverlapped)
        return false;
    pIoOverlapped->_wsabuf.buf = (char*)buffer;
    pIoOverlapped->_wsabuf.len = buferSize;
    pIoOverlapped->_opertype = OP_SEND;
    pIoOverlapped->_nActuallyBufferSize = actullyBufSize;
    pIoOverlapped->_pOriginalBuffer = (char*)buffer;

    int nRet = WSASend(pIoOverlapped->_s, &(pIoOverlapped->_wsabuf), 1, &dwSendNumBytes, dwFlags, &(pIoOverlapped->_overlapped), NULL);
    if ((SOCKET_ERROR == nRet) && (WSA_IO_PENDING != WSAGetLastError()))
    {
        int lasterror = WSAGetLastError();
        pIoSession->closeSession();
        LOG_ERROR("post send failed, return code: %d, lasterror: %d", nRet, lasterror);
        return false;
    }
    pIoSession->increaseSendRef();

    return true;
}

bool CAysncIOCPSvrMgr::AsyncSend(CIOSession* pIoSession, IO_OVERLAPPEDPLUS* pIoOverlappedPlus )
{
    //if (NULL == pIoOverlappedPlus || NULL == pIoSession)
    //    return false;
    //pIoOverlappedPlus->_opertype = OP_SEND;
    //pIoOverlappedPlus->_s = pIoSession->getSockFd();
    LOG_ERROR("not support this interface, try AsyncSendWithBuffer()");
    //return _PostSend(pIoSession, pIoOverlappedPlus);
    return false;
}

//int CAysncIOCPSvrMgr::ConnRemoteSvr(const char* hostname, const int& port, bool bKeepalive /* = true */)
//{
//    //////////////////////////////////////////////////////////////////////////
//    return INVALID_NPP_HANDLE;
//}

int CAysncIOCPSvrMgr::ConnAsyncRemoteSvr(const char* hostname, const int& port , bool bKeepalive, 
    OnSessionEvent sessionEventFunc, void* userdata ,
    EncodeMessage encodeFunc ,
    DecodeMessage decodeFunc,
    int sessionType,
    OnNonNppMessage onNonNppMessage)
{
    SOCKET connSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == connSocket) 
    {
        LOG_ERROR("Initialize socket failed£¬lasterror: %d", WSAGetLastError());
        return INVALID_NPP_HANDLE;
    }

    DWORD dwBytes = 0;
    GUID GuidConnectEx = WSAID_CONNECTEX ;
    if (SOCKET_ERROR == WSAIoctl (connSocket , SIO_GET_EXTENSION_FUNCTION_POINTER ,
        &GuidConnectEx , sizeof (GuidConnectEx ),
        &m_lpfnConnectEx , sizeof (m_lpfnConnectEx ), &dwBytes , 0, 0))
    {
        closesocket(connSocket);
        return INVALID_NPP_HANDLE;
    }

    //bind local address here
    sockaddr_in local_addr ;
    ZeroMemory (&local_addr , sizeof (sockaddr_in ));
    local_addr .sin_family = AF_INET ;
    int irt = ::bind (connSocket , (sockaddr *)(&local_addr ), sizeof (sockaddr_in ));
    if (irt != 0)
    {
        LOG_ERROR("bind local socket error£¡lasterror: %d", WSAGetLastError());
        closesocket(connSocket);
        return INVALID_NPP_HANDLE;
    }

    CIOSession* pConnSession = GlobalIoSessionMgr::instance()->getAvailableIoSession(connSocket);
    if (NULL == pConnSession)
    {
        closesocket(connSocket);
        return INVALID_NPP_HANDLE;
    }

    pConnSession->setIoSessionEventHandler(sessionEventFunc, userdata);
    if (NULL != encodeFunc || NULL != decodeFunc)
    {
        pConnSession->setEncodeWrapperFunc((encodeWrapperFunc)encodeFunc);
        pConnSession->setMessageSticker(decodeFunc);
    }
    
    pConnSession->setOnMessageFunc(onNonNppMessage);
    pConnSession->setSessionType(sessionType);

    PVOID lpSendBuffer = NULL;
    DWORD dwSendDataLength = 0;
    DWORD dwBytesSent = 0;
    bool isEverythingOk = false;
    do 
    {
        //**keep-alive mechanism start**
        int nRet = 0;
        if (bKeepalive)
        {
            int bKeepAlive = 1;
            nRet = ::setsockopt(connSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive));
            if (nRet == SOCKET_ERROR)
                LOG_WARN("open socket keep alive failed, last error %d", WSAGetLastError());
            else
            {
                tcp_keepalive alive_in   = {0};
                tcp_keepalive alive_out = {0};
                alive_in.keepalivetime      = 5000;
                alive_in.keepaliveinterval = 1000;
                alive_in.onoff                  = 1;

                unsigned long ulBytesReturn = 0;
                nRet = WSAIoctl(connSocket, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
                    &alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL);
                if (nRet == SOCKET_ERROR)
                    LOG_WARN("set socket keep alive parameters failed, last error %d", WSAGetLastError());
            }
        }
        //**keep-alive mechanism end**

        if (!_AssociateWithIOCP(pConnSession))
        {
            LOG_ERROR("bind listen socket into iocp port error£¡lasterror: %d", WSAGetLastError());
            break;
        }

        IO_OVERLAPPEDPLUS* ioOverlappedPlus = GlobalOverlappedPlusMgr::instance()->getAvailableIoOverlapped(
            pConnSession->getSockFd(),
            GlobalConfigure::instance()->getRecvBuffer()
            );
        if (NULL == ioOverlappedPlus)
            break;
        else
            ioOverlappedPlus->_opertype = OP_CONN;

        sockaddr_in addrPeer;
        ZeroMemory (&addrPeer , sizeof (sockaddr_in ));
        addrPeer .sin_family = AF_INET ;
        addrPeer .sin_addr .s_addr = inet_addr(hostname);
        addrPeer .sin_port = htons(port);
        int naddrLen = sizeof(addrPeer);

        BOOL bResult = m_lpfnConnectEx (connSocket ,(sockaddr *)&addrPeer ,naddrLen,
            lpSendBuffer, dwSendDataLength, &dwBytesSent ,&ioOverlappedPlus->_overlapped); 
        if (!bResult )
        {
            if ( WSAGetLastError () != ERROR_IO_PENDING )
            {
                LOG_ERROR("connecting server asynchronously failed, lasterror code %d", WSAGetLastError());
                break;
            }
        }
        pConnSession->setRemote(inet_addr(hostname), port);

        isEverythingOk = true;
    } while (false);

    if (isEverythingOk )
    {
        if (pConnSession->getSessionType() == SES_TYPE_NPP)
        {
            pConnSession->startLookupTimeoutRequest();
        }
        
        return pConnSession->getIdentity();
    }

    GlobalIoSessionMgr::instance()->freeIoSessionByID(pConnSession->getIdentity());

    return INVALID_NPP_HANDLE;
}

int CAysncIOCPSvrMgr::ConnSyncRemoteSvr( 
    const char* hostname, const int& port, bool bKeepalive,
    OnSessionEvent sessionEventFunc, 
    void* userdata ,
    EncodeMessage encodeFunc,
    DecodeMessage decodeFunc,
    int sessionType,
    OnNonNppMessage onNonNppMessage)
{
    // IO overlapped, must invoke WSASocket to get new socket or not support overlapped
    SOCKET connSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == connSocket) 
    {
        LOG_ERROR("Initialize socket failed£¬lasterror: %d", WSAGetLastError());
        return INVALID_NPP_HANDLE;
    }
    CIOSession* pConnSession = GlobalIoSessionMgr::instance()->getAvailableIoSession(connSocket);
    if (NULL == pConnSession)
    {
        closesocket(connSocket);
        return INVALID_NPP_HANDLE;
    }

    pConnSession->setIoSessionEventHandler(sessionEventFunc, userdata);
    if (NULL != encodeFunc || NULL != decodeFunc)
    {
        pConnSession->setEncodeWrapperFunc((encodeWrapperFunc)encodeFunc);
        pConnSession->setMessageSticker(decodeFunc);
    }

    pConnSession->setOnMessageFunc(onNonNppMessage);
    pConnSession->setSessionType(sessionType);

    bool isEverythingOk =false;

    do 
    {
        //****************************************************************************************
        int nRet = 0;
        if (bKeepalive)
        {
            int bKeepAlive = 1;
            nRet = ::setsockopt(connSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive));
            if (nRet == SOCKET_ERROR)
                LOG_WARN("open socket keep alive failed, last error %d", WSAGetLastError());
            else
            {
                // set KeepAlive parameter
                tcp_keepalive alive_in   = {0};
                tcp_keepalive alive_out = {0};
                alive_in.keepalivetime      = 5000;
                alive_in.keepaliveinterval = 1000;
                alive_in.onoff                  = 1;

                unsigned long ulBytesReturn = 0;
                nRet = WSAIoctl(connSocket, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
                    &alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL);
                if (nRet == SOCKET_ERROR)
                    LOG_WARN("set socket keep alive parameters failed, last error %d", WSAGetLastError());
            }
        }
        //***************************************************************************************
        if (!_AssociateWithIOCP(pConnSession))
        {
            LOG_ERROR("bind listen socket into iocp port error£¡lasterror: %d", WSAGetLastError());
            break;
        }

        struct sockaddr_in ServerAddress;
        ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
        ServerAddress.sin_family = AF_INET;
        //ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        ServerAddress.sin_addr.s_addr = inet_addr(hostname);
        ServerAddress.sin_port = htons(port);

        //@status changed
        IO_OVERLAPPEDPLUS* ioOverlappedPlus = GlobalOverlappedPlusMgr::instance()->getAvailableIoOverlapped(
            pConnSession->getSockFd(),
            GlobalConfigure::instance()->getRecvBuffer()
            );
        if (NULL == ioOverlappedPlus)
            break;

        if(0 !=WSAConnect(pConnSession->getSockFd(),
            (struct sockaddr *)&ServerAddress, sizeof(ServerAddress),
            &ioOverlappedPlus->_wsabuf,
            NULL,
            NULL, 
            NULL))
        {
            LOG_ERROR("connecting server synchronously failed, lasterror code %d", WSAGetLastError());
            pConnSession->FireSessionClosed();
            break;
        }
        //@status changed
        pConnSession->setStatus(SESSION_OPENED);
        pConnSession->setRemote(inet_addr(hostname), port);

        
        if(!_PostRecv(pConnSession, ioOverlappedPlus))
        {
            LOG_ERROR("post recieve operation on %s failed!", pConnSession->getRemote());
            break;
        }

        pConnSession->FireSessionOpened();

        isEverythingOk = true;
    } while (false);
    
    if (isEverythingOk )
    {
        if (pConnSession->getSessionType() == SES_TYPE_NPP)
        {
            pConnSession->startLookupTimeoutRequest();
        }
        
        return pConnSession->getIdentity();
    }

    GlobalIoSessionMgr::instance()->freeIoSessionByID(pConnSession->getIdentity());
    
    return INVALID_NPP_HANDLE;
}

bool CAysncIOCPSvrMgr::DisconnRemoteSvr( int hChannel )
{
    CIOSession* pClientSession = GlobalIoSessionMgr::instance()->getOccupancyIoSessionByID(hChannel);
    if (NULL != pClientSession)
    {
        pClientSession->closeSession();//GlobalIoSessionMgr::instance()->freeIoSessionByID(hChannel);
    }
    return true;
}




