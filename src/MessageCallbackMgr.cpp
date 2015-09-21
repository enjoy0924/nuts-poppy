#include "MessageCallbackMgr.h"

CMessageCallbackMgr::CMessageCallbackMgr():m_nppMessageConstructor(NULL),
    m_sessionEventHandler(NULL)
{}
CMessageCallbackMgr::~CMessageCallbackMgr(){}

int CMessageCallbackMgr::registerMessageCallback(int messageCode, DoMessageExecute onMessageFunc)
{
    ACE_Guard<ACE_Mutex> guard(m_iMutex);
    m_mapDoExecuteFunc[messageCode] = onMessageFunc;
    return NPP_NOERROR;
}

DoMessageExecute CMessageCallbackMgr::getMessageCallbackByCode( int messageCode ) 
{
    ACE_Guard<ACE_Mutex> guard(m_iMutex);
    return m_mapDoExecuteFunc[messageCode];
}

void CMessageCallbackMgr::setMessageConstructor( nppMessageConstructor defaultCreateMessage ) 
{
    m_nppMessageConstructor = defaultCreateMessage;
}

nppMessageConstructor CMessageCallbackMgr::getMessageConstructor()
{
    return m_nppMessageConstructor;
}

void CMessageCallbackMgr::FireIoEventHandler( int sessionId, ENUM_IO_EVENT event ) 
{
    //throw std::exception("The method or operation is not implemented.");
    //we should give another thread to fire this kind of event
    return ;
}

void CMessageCallbackMgr::setIoEventHandler( IoEventHandler sessionEventHandler )
{
    m_sessionEventHandler = sessionEventHandler;
}

IoEventHandler CMessageCallbackMgr::getIoEventHandler()
{
    return m_sessionEventHandler;
}
