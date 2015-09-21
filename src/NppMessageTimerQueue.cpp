#include "NppMessageTimerQueue.h"

CNppMessageTimerQueue::CNppMessageTimerQueue():
m_bInitialized(false)
{
}
CNppMessageTimerQueue::~CNppMessageTimerQueue()
{
}

bool CNppMessageTimerQueue::Initialize()
{
    if(m_bInitialized)
        return true;
    m_hTimeQueue = ::CreateTimerQueue();
    if (INVALID_HANDLE_VALUE == m_hTimeQueue)
        return false;
    m_bInitialized = true;
    return true;
}

bool CNppMessageTimerQueue::CreateTimer(NPP_HANDLE& hTimer,NppWaitOrTimerCallBack timerCallback, void* userdata, unsigned int period)
{
    if (!m_bInitialized)
        return false;
    hTimer = INVALID_HANDLE_VALUE;
    BOOL ret = ::CreateTimerQueueTimer(&hTimer, m_hTimeQueue, (WAITORTIMERCALLBACK)timerCallback, userdata, 
        0,                        //due time
        period, 
        WT_EXECUTEDEFAULT); //how does the timer work, into-thread or run-once ... ...
    if (!ret || INVALID_HANDLE_VALUE == hTimer)
        return false;
    return true;
}

bool CNppMessageTimerQueue::DestroyTimer(NPP_HANDLE hTimer)
{
    if (!m_bInitialized)
        return false;

    BOOL ret = FALSE;
    if (INVALID_HANDLE_VALUE != hTimer && INVALID_HANDLE_VALUE != m_hTimeQueue)
    {
        ret = ::DeleteTimerQueueTimer(m_hTimeQueue, hTimer, INVALID_HANDLE_VALUE);
    }
    return ret;
}

void CNppMessageTimerQueue::Finalize()
{
    if (!m_bInitialized)
        return;
    if (m_hTimeQueue != INVALID_HANDLE_VALUE)
    {
        ::DeleteTimerQueueEx(m_hTimeQueue, INVALID_HANDLE_VALUE);
    }
    m_bInitialized = false;
}