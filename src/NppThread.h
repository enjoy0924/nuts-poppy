#ifndef __NPP_THREAD_H__
#define __NPP_THREAD_H__

#include <string>
#include <windows.h>
#include <process.h>

class Runnable
{
public:
    virtual ~Runnable() {};
    virtual void Run() = 0;
};

class CNppThread
{
private:
    explicit CNppThread(const CNppThread & rhs);

public:
    CNppThread();
    CNppThread(Runnable * pRunnable);
    CNppThread(const char * ThreadName, Runnable * pRunnable = NULL);
    CNppThread(std::string ThreadName, Runnable * pRunnable = NULL);
    ~CNppThread(void);

    //** start thread
    //* bSuspend, whether should suspend this thread when it start
    //**true if successfully start
    bool Start(bool bSuspend = false);

    //**wait the thread exit
    //*timeout, how long should wait, -1 means forever waiting
    //**
    void Join(int timeout = -1);

    //**resume one suspended thread
    //*
    //**
    void Resume();

    //**suspend one thread
    //*
    //**
    void Suspend();
    //**terminate one thread
    //*
    //**
    bool Terminate(unsigned long ExitCode);

    bool isRun(){return m_bRun;}

    unsigned int GetThreadID();
    std::string GetThreadName();
    void SetThreadName(std::string ThreadName);
    void SetThreadName(const char * ThreadName);

private:
    //**static thread function, the system will invoke here
    //*
    //**
    static unsigned int WINAPI StaticThreadFunc(void * arg);

    //**if the thread is running, it will invoke this function
    //*
    //**
    virtual void Run();

private:
    HANDLE              m_handle;            //thread handle
    Runnable * const m_pRunnable;      //runnable instance
    unsigned int        m_ThreadID;        //thread identity
    std::string            m_ThreadName;   //thread name
    volatile bool         m_bRun;              //thread status
};

#endif