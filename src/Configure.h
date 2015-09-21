#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__
#include "ace/ace_header.h"

//**********************default macro configuration******************
// io buffer size (1024*8)£¬because memory page size is 4K (1024*4)£¬
// this value would be better set double or triple 4K
#define MAX_BUFFER_LEN                      8192  
#define MAX_MSG_LEN (MAX_BUFFER_LEN/2)
#define MAX_SESSEION_SEQ                  65535
#define DEFAULT_ALLOC_BUFSIZE          (1024*4*1024*4)   //16M

#define DEFAULT_TIMEOUT_LIMIT          10

class CConfigure
{
public:
    CConfigure():m_isInitialized(false)
    {
        m_iTimeoutLimit = DEFAULT_TIMEOUT_LIMIT;
        m_iRecvBufSize = MAX_BUFFER_LEN;
        m_iMemPoolSize = DEFAULT_ALLOC_BUFSIZE;
    }
    ~CConfigure()
    {

    }
    
    
    void setInitialized(bool initialized) {m_isInitialized = initialized;}
    bool isInitialized(){return m_isInitialized;}

    int getMaxSessionSequence() 
    {
        throw std::exception("The method or operation is not implemented.");
    }

    unsigned long long getAllocHeapSize() 
    {
        return m_iMemPoolSize;
    }

    int getRecvBuffer() 
    {
        return m_iRecvBufSize;
    }

    int getTimeoutLimit() 
    {
        return m_iTimeoutLimit+2;
    }
protected:
    
private:
    unsigned long long    m_iMemPoolSize; //default memory pool size

    int    m_iRecvBufSize;                              //receive buffer length
    int    m_iTimeoutLimit;                            //time-out limit
    int    m_iIoProcessorNum;                       //the number of io processors
    int    m_iWorkerPoolSize;                        //worker thread pool size
    bool m_bKeepAlive;                                //tcp keep-alive mechanism
    bool m_bRecordLog;                               //open logging mechanism

    bool m_isInitialized;
};

typedef ACE_Singleton<CConfigure , ACE_Null_Mutex> GlobalConfigure; 

#endif