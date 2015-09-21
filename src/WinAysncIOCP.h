#ifndef __WIN_AYSNCIOCP_H__
#define __WIN_AYSNCIOCP_H__
#include "ace/ace_header.h"
#include "IoSessionMgr.h"
#include <MSWSock.h>    //this header file include windows Expansed function like AcceptEx

//****************parameters struct for io processor thread******************
class CAysncIOCPSvrMgr;
typedef struct _tagThreadParams_WORKER
{
    CAysncIOCPSvrMgr* pIOCPModel;     // 
    int         nThreadNo;       //
} THREADPARAMS_WORKER,*PTHREADPARAM_WORKER; 

//@Scope("singleton")
class CAysncIOCPSvrMgr
{
public:
    CAysncIOCPSvrMgr(void);
    ~CAysncIOCPSvrMgr(void);
public:
    //initialize the singleton 
    bool InitSvrMgr();
    //finalize the singleton
    void FiniSvrMgr();

public:
    //open a new listen port
    int StartServer(const char* hostname, const int& port, const int acceptnum = 10);
    //stop one listen port
    bool StopServer(int hSvr);

    //int ConnRemoteSvr(const char* hostname, const int& port, bool bKeepalive = true);

    int ConnAsyncRemoteSvr(const char* hostname, const int& port , 
        bool bKeepalive = false,
        OnSessionEvent sessionEventFunc = NULL, 
        void* userdata = NULL,
        EncodeMessage encodeFunc = NULL,
        DecodeMessage decodeFunc = NULL,
        int sessionType = SES_TYPE_NPP,
        OnNonNppMessage onNonNppMessage= NULL);

    int ConnSyncRemoteSvr(const char* hostname, const int& port, 
        bool bKeepalive = true,
        OnSessionEvent sessionEventFunc = NULL, 
        void* userdata = NULL,
        EncodeMessage encodeFunc = NULL,
        DecodeMessage decodeFunc = NULL,
        int sessionType = SES_TYPE_NPP,
        OnNonNppMessage onNonNppMessage= NULL);

    bool DisconnRemoteSvr(int hChannel);

    bool AsyncSend(CIOSession* pIoSession, IO_OVERLAPPEDPLUS* pIoOverlapped);

    bool AsyncSendWithBuffer(CIOSession* pIoSession,const char* buffer, const int buferSize, const int actullyBufSize = 0);
private:
    //initialize iocp environment
    bool _InitializeIOCP();
    //finalize iocp environment
    void _DeInitialize();

    //io processor execute here
    static DWORD WINAPI _IoProcessorThread(LPVOID lpParam);

    //bind io session into iocp
    bool _AssociateWithIOCP( CIOSession *pIoSession);

    //post asynchronous accept request on the acceptor io overlapped
    bool _PostAccept(CIOSession* pSocketSession, IO_OVERLAPPEDPLUS* pAcceptOverlapped); 
    //post asynchronous receive operation on one specified socket
    bool _PostRecv(CIOSession* pSocketSession, IO_OVERLAPPEDPLUS* pIoOverlappedPlus );
    //post asynchronous send operation on one specified socket
    bool _PostSend(CIOSession* pSocketSession, IO_OVERLAPPEDPLUS* pIoOverlappedPlus );

    //do the information arrived on some specified socket
    bool _DoAccpet( CIOSession* pSocketSession, IO_OVERLAPPEDPLUS* pIoOverlappedPlus);
    bool _DoRecv( CIOSession* pSocketSession, IO_OVERLAPPEDPLUS* pIoOverlappedPlus);
    bool _DoSend( CIOSession* pSocketSession, IO_OVERLAPPEDPLUS* pIoOverlappedPlus);
    bool _DoConn( CIOSession* pSocketSession, IO_OVERLAPPEDPLUS* pIoOverlappedPlus);

private:
    //get the number of available cpu processor 
    int _GetNoOfProcessors();
    //is socket alive?
    bool _IsSocketAlive(SOCKET s);
    //handle completion port error
    bool _HandleError( CIOSession *pIoSession,const DWORD& dwErr );
private:

    // notify the processor thread to exit
    HANDLE m_hShutdownEvent;
    //handle of io completion port
    HANDLE m_hIOCompletionPort;
    //handle of thread
    HANDLE* m_phWorkerThreads;
    //the number of processor
    int m_nThreads;

    //windows socket expanse function pointer
    LPFN_CONNECTEX m_lpfnConnectEx;
    LPFN_ACCEPTEX m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS    m_lpfnGetAcceptExSockAddrs; 
};

typedef ACE_Singleton<CAysncIOCPSvrMgr , ACE_Null_Mutex> GlobalSvrAysncMgr; 

#endif
