#ifndef __IO_SESSION_H__
#define __IO_SESSION_H__

#include "DefaultMessageParse.h"
#include "DefaultEncodeWrapper.h"
#include "Configure.h"
#include "npp.h"
#include "IoOverlappedManager.h"
#include "NppMessageTimerQueue.h"
#include <list>
#include <hash_map>

#define NPP_MSG_TIMEOUT 10

//*************************************************************
typedef bool (*encodeWrapperFunc)(void* inRawMessage,const int inLen, char** outBuffer, int& inMaxBufLen , int& outLen);
typedef bool (*stickMessageHandler)(const char* rawbuf, const int buflen, int& stickpos, bool& skip);

typedef enum _IO_SESSION_STATUS
{
    SESSION_OPENED,      //session has been allocated
    SESSION_CLOSING,     //session is closing, there should not be any operation on this specific socket
    SESSION_CLOSED,       //session is closed, this session is unavailable
}ENUM_IOSESSION_STATUS;

//**1-io session is not on business with io overlapped
//**2-io session offers request time-out mechanism, every request client session has a timer, none for response server session
//**3-manage the connection's information, like socket file descriptor, remote peer address
//**4-manage the message encode and decode function, store current message task corresponding to this session
//**5-offer the interface to manage session life cycle time
class CIOSession
{
public:
    CIOSession(SOCKET sock);
    CIOSession(SOCKET sock, stickMessageHandler sticker);
    ~CIOSession();

    const char* getRemote();

    void setMessageSticker(stickMessageHandler sticker);
    stickMessageHandler getMessageSticker();

    void setEncodeWrapperFunc(encodeWrapperFunc encode);
    encodeWrapperFunc getEncodeWrapperFunc();

    SOCKET getSockFd();
    void setRemote( unsigned long addr, int port );
    void setStatus(ENUM_IOSESSION_STATUS status);
    ENUM_IOSESSION_STATUS getStatus();

    void ResetBuffer();

    //message received from remote peer
    bool insertMessageTask( void* pNutsWrapperMessage );
    NutsWrapperMessage* getMessageTask();
    int getMessageTaskNum();
    bool getMessageTaskQueue(std::list<void*>& messageQueue);

    void setSockFd( int sessionId );

    //timer for checking out time-out messages and throw them out
    static void __stdcall TimeoutQueueTimer(void* userdata, bool waitortime);

    //local sent message that need to response
    void insertMessageTimeoutQueue(int sequence, char* timeoutMessage );
    bool ejectMessageTimeoutQueueWithSeq(int sequence);
    
    //timeout function
    void startLookupTimeoutRequest();
    void stopLookupTimeoutRequest();
    void lookupLocalRequest();
    void cleanLocalRequest();

    bool isOpenTimeoutMechanism();
    void setIoSessionEventHandler( OnSessionEvent ioHandlerFunc, void* userdata );

    bool closeSession(bool bImediately = false);

    //fire session event here
    void FireSessionClosed();
    void FireSessionCreated();
    void FireSessionOpened();
    void FireSessionExcepted();

    void FireMessageRecved(const char* buffer, const int len);

    void setIdentity(const int& m_iSessionIdentity );
    int getIdentity();

    void increaseSendRef();
    void decreaseSendRef();
    void setSessionType( const int sessionType );
    int getSessionType();
    bool isAvailbale();

    void setOnMessageFunc( OnNonNppMessage onNonNppMessage );
    OnNonNppMessage getOnMessageFunc();
private:
    bool shutdownSession();
private:
    //store npp message received from remote peer
    ACE_Mutex m_mutexQueue;
    std::list<void*> m_messageQueue;

    //store request message need to response
    ACE_Mutex m_mutexTimeout;
    std::hash_map<int, char*> m_messageTimeoutQueue;

    SOCKET           m_socket;                //socket file descriptor, only can be set in construct function
    unsigned long m_peeraddr;             //peer address
    unsigned long m_createtime;           //the time when this session created
    char                m_strAddr[48];
    int                   m_port;                     //peer port

    NPP_HANDLE  m_hTimer;

    bool               m_openTimeout;      //use the timeout mechanism
    //the reference idle count, [1, 2, 3, 4, 5, 6, 7, 8],means idle time, unit's seconds
    //if there is a connection's reference idle count larger than 8 seconds,
    //timer [run per second] will kick it out from server, should be implemented
    int m_idleRef;

    //if this session must be closed, make sure all the send io overlapped return, 
    //then close the socket is safe
    ACE_Mutex m_lSendRefMutex;
    long m_lSockSendRef;

    //identity, be set only once
    int m_identity;

    //session type, 0-npp session, 1-non-npp session
    int m_iSessionType;
    
    ENUM_IOSESSION_STATUS   m_status;    //symbol tagged to judge this object should be recycle
    stickMessageHandler m_sticker;              //the callback function to split packages up
    encodeWrapperFunc m_encoderFunc;    //the callback function to wrapper a npp message
    
    OnSessionEvent m_ioHandlerFunc;
    void* m_ioUserdata;
    OnNonNppMessage m_onNonNppMessageFunc;
};

#endif