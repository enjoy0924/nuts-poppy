#include "NppThread.h"

CNppThread::CNppThread(void) : 
m_pRunnable(NULL),
    m_bRun(false)
{
}

CNppThread::~CNppThread(void)
{
}

CNppThread::CNppThread(Runnable * pRunnable) : 
    m_ThreadName(""),
    m_pRunnable(pRunnable),
    m_bRun(false)
{
}

CNppThread::CNppThread(const char * ThreadName, Runnable * pRunnable) : 
m_ThreadName(ThreadName),
    m_pRunnable(pRunnable),
    m_bRun(false)
{
}

CNppThread::CNppThread(std::string ThreadName, Runnable * pRunnable) : 
m_ThreadName(ThreadName),
    m_pRunnable(pRunnable),
    m_bRun(false)
{
}

bool CNppThread::Start(bool bSuspend)
{
    if(m_bRun)
    {
        return true;
    }
    if(bSuspend)
    {
        m_handle = (HANDLE)_beginthreadex(NULL, 0, StaticThreadFunc, this, CREATE_SUSPENDED, &m_ThreadID);
    }
    else
    {
        m_handle = (HANDLE)_beginthreadex(NULL, 0, StaticThreadFunc, this, 0, &m_ThreadID);
    }
    m_bRun = (NULL != m_handle);
    return m_bRun;
}

void CNppThread::Run()
{
    if(!m_bRun)
    {
        return;
    }
    if(NULL != m_pRunnable)
    {
        m_pRunnable->Run();
    }
    m_bRun = false;
}

void CNppThread::Join(int timeout)
{
    if(NULL == m_handle || !m_bRun)
    {
        return;
    }
    if(timeout <= 0)
    {
        timeout = INFINITE;
    }
    ::WaitForSingleObject(m_handle, timeout);
}

void CNppThread::Resume()
{
    if(NULL == m_handle || !m_bRun)
    {
        return;
    }
    ::ResumeThread(m_handle);
}

void CNppThread::Suspend()
{
    if(NULL == m_handle || !m_bRun)
    {
        return;
    }
    ::SuspendThread(m_handle);
}

bool CNppThread::Terminate(unsigned long ExitCode)
{
    if(NULL == m_handle || !m_bRun)
    {
        return true;
    }
    if(::TerminateThread(m_handle, ExitCode))
    {
        ::CloseHandle(m_handle);
        return true;
    }
    return false;
}

unsigned int CNppThread::GetThreadID()
{
    return m_ThreadID;
}

std::string CNppThread::GetThreadName()
{
    return m_ThreadName;
}

void CNppThread::SetThreadName(std::string ThreadName)
{
    m_ThreadName = ThreadName;
}

void CNppThread::SetThreadName(const char * ThreadName)
{
    if(NULL == ThreadName)
    {
        m_ThreadName = "";
    }
    else
    {
        m_ThreadName = ThreadName;
    }
}

unsigned int CNppThread::StaticThreadFunc(void * arg)
{
    if (NULL == arg)
        return 0;
    CNppThread * pThread = reinterpret_cast<CNppThread*>(arg);
    pThread->Run();
    return 0;
}