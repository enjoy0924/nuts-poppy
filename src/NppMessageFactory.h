#ifndef __NPP_MESSAGE_FACTORY_H__
#define __NPP_MESSAGE_FACTORY_H__
#include "MessageCallbackMgr.h"
#include "IoSession.h"
#include "NutsWrapperMessage.pb.h"

//************every wrapper message is a runnable object*********
//@Scope("prototype")
class CNppMessageFactory
{
public:
    CNppMessageFactory();
    ~CNppMessageFactory();

    //**fire a npp message event received from remote peer
    //*serilized, information carry on message entity
    //*len, the length of information
    //*pClientSession, 
    //*timeout, it's a boolean value, may be set true in the local peer
    //**return true if successful, or return false
    bool FireMessageEvent(const void * serilized,const int len, CIOSession* pClientSession, bool timeout= false);

    //**fire IO Event , like as session create or session idle
    //*pClientSession, one specified session
    //**return true if successful, or return false
    bool FireIOEvent(CIOSession* pClientSession);
};

typedef ACE_Singleton<CNppMessageFactory , ACE_Null_Mutex> GlobalNppMessageFactory; 

#endif