#ifndef __MESSAGE_CALLBACK_MGR_H__
#define __MESSAGE_CALLBACK_MGR_H__
#include "ace/ace_header.h"
#include "npp.h"
#include <map>

//**************definition for message callback function***********

//@Scope("singleton")
class CMessageCallbackMgr 
{
public:
	CMessageCallbackMgr();
	~CMessageCallbackMgr();

    int registerMessageCallback(int messageCode, DoMessageExecute onMessageFunc);
    DoMessageExecute getMessageCallbackByCode( int messageCode );

    void setMessageConstructor( nppMessageConstructor defaultCreateMessage );
    nppMessageConstructor getMessageConstructor();

    //**
    //*it's reverse interface
    //**
    void FireIoEventHandler( int sessionId, ENUM_IO_EVENT event );

    void setIoEventHandler( IoEventHandler sessionEventHandler );
    IoEventHandler getIoEventHandler();
private:
    ACE_Mutex m_iMutex;
    std::map<int, DoMessageExecute> m_mapDoExecuteFunc;
    nppMessageConstructor m_nppMessageConstructor;
    IoEventHandler m_sessionEventHandler;
};

typedef ACE_Singleton<CMessageCallbackMgr , ACE_Null_Mutex> GlobalMessageCallbackMgr; 


#endif