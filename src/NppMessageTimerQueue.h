#ifndef __NPP_MESSAGE_TIMER_QUEUE_H__
#define __NPP_MESSAGE_TIMER_QUEUE_H__
#include "ace/ace_header.h"
#include "npp.h"
#include <list>

//@Scope("singleton")
class CNppMessageTimerQueue
{
public:
	CNppMessageTimerQueue();
	~CNppMessageTimerQueue();

    bool Initialize();
    bool CreateTimer(NPP_HANDLE& hTimer,NppWaitOrTimerCallBack timerCallback, void* userdata, unsigned int period);
    bool DestroyTimer(NPP_HANDLE hTimer);
    void Finalize();
	
private:
    bool m_bInitialized;
    HANDLE m_hTimeQueue;
};

typedef ACE_Singleton<CNppMessageTimerQueue , ACE_Null_Mutex> GlobalTimerQueue; 
#endif