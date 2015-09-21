#ifndef __NUTS_POPPY_PROTO_H__
#define __NUTS_POPPY_PROTO_H__

#ifdef NPP_EXPORTS
#define NPP_API __declspec(dllexport)
#else
#define NPP_API __declspec(dllimport)
#endif

#define NPP_DECLARE __stdcall
#define _out 
#define _in 

typedef enum _SES_TYPE
{
    SES_TYPE_NPP = 0,       //npp session type
    SES_TYPE_NONNPP      //non-npp session type
}SES_TYPE;

//************************error code****************************
#define  HEX_MINUS (-1)
#define NPP_NOERROR                                                                       0    //no error happened, it's ok
#define NPP_ENOTFOUNDLOGCFG                                NPP_NOERROR    //log not initialized correctly, it's just a warning message, not critical
#define NPP_ENETWORKENV                          (0x0000001 | HEX_MINUS)   //network environment initialization error
#define NPP_EUPPERTHREADPOOL                 (0x0000002 | HEX_MINUS)   //upper thread pool error
#define NPP_EPROCESSORTHREAD                 (0x0000003 | HEX_MINUS)   //processor thread error
#define NPP_ENOTSUPPORT                           (0x0000004 | HEX_MINUS)   //not support currently, coming soon ... ...
#define NPP_EINVALIDPARAM                        (0x0000005 | HEX_MINUS)   //invalid parameter
#define NPP_ENOTAVALIABLESRC                   (0x0000006 | HEX_MINUS)   //not available resource
#define NPP_ELACKNECESSARYSRC                 (0x0000006 | HEX_MINUS)   //lack necessary resource
#define NPP_EASYNCCONN                            (0x0000007 | HEX_MINUS)   //asynchronously connect failed
#define NPP_ENOTINITLIB                               (0x0000008 | HEX_MINUS)   //not initialize npp library
#define NPP_ENOTENOUGHSRC                      (0x0000009 | HEX_MINUS)   //there is no enough system resource
#define NPP_ENETWORKBUSY                         (0x000000A | HEX_MINUS)   //there is another asynchronous post not competing right now, maybe posted send
#define NPP_EINVALIDSESSIONTYPE               (0x000000B | HEX_MINUS)   //invalid session type


//***********************message type***************************
typedef enum _MessageType {
    NPP_UNKNOWN_MSGTYPE     = 0x0000,
    NPP_REQUEST_WITH_RSP        = 0x0001,
    NPP_REQUEST_WITHOUT_RSP = 0x0002,
    NPP_RESPONSE_NORMAL       = 0x0003
}NppMessageType;

#define INVALID_NPP_HANDLE 0

typedef int NPP_SESSION_HANDLE;

//************************session event***************************
typedef enum _IO_EVENT
{
    IO_OPENED  = 0x0001,    //io has listened or connected
    IO_CLOSED   = 0x0002    //io has been closed, the handle is not available
}ENUM_IO_EVENT;

#define NON_NPP_MSG_CODE 0x0000

//********session event handler, callback function****************
typedef void (*IoEventHandler)(const ENUM_IO_EVENT ioEvent, void* userdata, const int sessionHandle);

//******encode and decode message callback function*********
typedef void* (*EncodeMessage)(void* _in inRawMessage,const int _in inLen, char** _out outBuffer, int& _out inMaxBufLen , int& _out outLen);
typedef bool (*DecodeMessage)(const char*, const int, int&, bool&);

typedef bool (*OnNonNppMessage)(const char* data, const int len, NPP_SESSION_HANDLE hsession, void* userdata);

typedef void (*OnSessionEvent)(NPP_SESSION_HANDLE hSession, ENUM_IO_EVENT ioEvent, void* userdata, int error);

class WrapperNppMessage;

//***********message callback and npp message constructor callback***********
typedef WrapperNppMessage* (*DoMessageExecute)( WrapperNppMessage*);
typedef void* (*nppMessageConstructor)(const char*);

//***********************************OS Utils****************************************
typedef void* NPP_HANDLE;
typedef void (__stdcall *NppWaitOrTimerCallBack)(void* pUserdata, bool bTimeOrWait); //the callback function of timer

#ifdef __cplusplus
extern "C" {
#endif
    //**brief ,initialize the nuts poppy library, if you want to use it, invoke this function firstly
    //*numOfProcessors ,how many threads to process io event
    //*numOfThreadPools, how many thread to process upper message event
    //**return 0 if ok, or return non-0 value error code
    NPP_API int   NPP_DECLARE Npp_Init(int _in numOfProcessors = 0, int _in numOfThreadPools = 0);
    
    //**brief , finalize the nuts poppy library
    NPP_API void NPP_DECLARE Npp_Fini();

    //********************************Communication Function*******************************************
    //
    //*******************************************************************************************************

    //**brief, this callback function will be invoked when session is created and opened
    //*sessionEventHandler, callback function pointer
    //**no return
    //NPP_API void NPP_DECLARE Npp_SetSessionOnCreatedAndOpened(IoEventHandler _in sessionEventHandler);
    
    //**brief, this callback function will be invoked when session is closed or encounter some exception(s)
    //*onExceptAndClosedFunc, callback function pointer
    //*userdata, data that user defined for callback function parameter
    //*hHandler, one session specified
    //**return true if successful or return false
    //NPP_API bool NPP_DECLARE Npp_SetSessionOnExceptionAndClosed(IoEventHandler _in onExceptAndClosedFunc, void* _in userdata, int hHandler);

    //**brief , message constructor, there is a recommended realization
    NPP_API void NPP_DECLARE Npp_SetNppMessageConstructor(nppMessageConstructor _in defaultCreateMessage);

    //**brief ,register callback function , bind it on unique message code
    //*messageCode, specify one kind of message
    //*onMessageFunc, callback function pointer 
    //** return 0 if ok, or return non-0 value error code
    NPP_API int   NPP_DECLARE Npp_RegisterCallback(int _in messageCode, DoMessageExecute _in onMessageFunc);
    

    //**brief, open a new port accept clients and their request
    //*hostname , specify a host , local host
    //*port , specify a port, local port
    //** return error code if error, or return non-0 handle value specify a service
    NPP_API NPP_SESSION_HANDLE NPP_DECLARE Npp_OpenLocalSvr(const char* _in hostname, int _in port, int& _out error, 
        bool _in bnpp=true, EncodeMessage _in encodeFunc = 0, DecodeMessage _in decodeFunc = 0);

    //**brief ,close a service specified
    //*handleSvr ,specify a service
    //**return 0 if ok, or return non-value error code
    NPP_API int NPP_DECLARE Npp_CloseLocalSvr(NPP_SESSION_HANDLE _in handleSvr);
    
    //**brief, connect one specified remote service
    //*hostname , specify a host , remote host
    //*port , specify a port, remote port
    //*error, there is something error happened, this value will be set
    //*bnpp, use npp session if it's true, or use non-npp session
    //*bKeepalive, use tcp keep-alive mechanism if it's true, or not
    //*bAsync, use asynchronous connection or use synchronous connection
    //*sessionEvent, this function will be invoked if session opened or closed
    //*userdata, user data, callback will send it back
    //*encodeFunc, encode function will be invoked if non-npp message need to be packed
    //*decodeFunc, decode function will be invoked if non-npp message need to be decode
    //*onMessageFunc, this function will be invoked if non-npp message is coming
    //** return error code if error, or return non-0 handle value specify a channel between client and server
    NPP_API NPP_SESSION_HANDLE NPP_DECLARE Npp_ConnRemoteSvr(
        const char* _in hostname, int _in port, int& _out error, 
        bool _in bnpp=true, bool bKeepalive = true, bool bAsync= true,
        OnSessionEvent _in sessionEvent = 0, void* userdata = 0, 
        EncodeMessage _in encodeFunc = 0, DecodeMessage _in decodeFunc = 0,
        OnNonNppMessage _in onMessageFunc = 0);

    //**brief ,close a service specified
    //*handleChannel ,specify a client
    //**return 0 if ok, or return non-value error code
    NPP_API int NPP_DECLARE Npp_CloseRemoteSvr(NPP_SESSION_HANDLE _in handleChannel);

    //**brief, message send without any outer package
    //*rawMessage, byte stream
    //*len , byte stream length
    //*handleChannel , message channel
    //*return 0 if ok, or return non-0 value error code
    NPP_API int NPP_DECLARE Npp_SendViaChannel(const void* _in rawMessage, const int& _in len, const NPP_SESSION_HANDLE _in handleChannel);
    
    //**brief, message send with specified outer package
    //*rawMessage, byte stream
    //*encodeFunc, encode function
    //*len , byte stream length
    //*handleChannel , message channel
    //*return 0 if ok, or return non-0 value error code
    //NPP_API int NPP_DECLARE Npp_SendEncodeViaChannel(const void* rawMessage, const int& len , EncodeMessage encodeFunc , const NPP_SESSION_HANDLE handleChannel);
    
    //**brief, message send with default outer package
    //*nppMessage, nuts poppy message/protocol buffer message
    //*handleChannel , message channel
    //*return 0 if ok, or return non-0 value error code
    NPP_API int NPP_DECLARE Npp_SendNppMSGViaChannel(void* _in nppMessage, const unsigned long _in messageCode , 
        const NPP_SESSION_HANDLE& _in handleChannel, bool _in bWithRsp = true, int _in sequenceNum = -1);

    //**brief, message send with default outer package
    //*nppWrapperMessage, nuts poppy message/protocol buffer message, a wrapper one
    //*return 0 if ok, or return non-0 value error code
    NPP_API int NPP_DECLARE Npp_SendNppMSG(WrapperNppMessage* _in nppWrapperMessage);

    
    //*********************************OS Utils Function API*****************************************
    //
    //*************************************************************************************************

    //**brief, create a timer
    //*hTimer, the handle of timer
    //*timerCallback, callback function 
    //*userdata, user data
    //*period, the frequency of execution
    //**return true if successful, or return false
    NPP_API bool NPP_DECLARE Npp_CreateTimer(NPP_HANDLE& _out hTimer,NppWaitOrTimerCallBack _in timerCallback, void* _in userdata, unsigned int _in period);

    //**brief, stop a timer , <destroy one specified timer>
    //*hTimer, the handle of timer
    //**return true if successful, or return false
    NPP_API bool NPP_DECLARE Npp_DestroyTimer(NPP_HANDLE _in hTimer);


    NPP_API void NPP_DECLARE Npp_InnerTEST();
#ifdef __cplusplus
}
#endif

    class NPP_API CNppConnector
    {
    public:
        CNppConnector();
        ~CNppConnector();
    protected:

    private:
    };

    class NPP_API CNppAcceptor
    {
    public:
        CNppAcceptor();
        ~CNppAcceptor();
    protected:

    private:
    };

    class NPP_API CNppUtils
    {
    public:
        static int getCpuUsage();
        static int getMemUsage();
        static unsigned long long getCurrentTime();
        static int getSequence();
    };

    class NPP_API WrapperNppMessage
    {
    public:
        WrapperNppMessage();
        ~WrapperNppMessage();

        void setMessageType(const int& type);
        int getMessageType();

        void setMessageCode(const int& code);
        int getMessageCode();

        void setMessageSeq(const int& sequence);
        int getMessageSeq();

        void setDestination(const int& dst);
        int getDestination();

        void setSource(const int& src);
        int getSource();

        void setMessageEntity(void* npp);
        void* getMessageEntity();

    private:
        int m_type;
        int m_code;
        int m_sequence;
        int m_destination;
        int m_source;
        void* m_nppmessage;
    };

#endif