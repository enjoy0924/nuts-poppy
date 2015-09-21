#ifndef __IO_OVERLAPPED_BUFFER_POOL_H__
#define __IO_OVERLAPPED_BUFFER_POOL_H__

#include "mempool/CMemoryPool.h"
#include "Configure.h"

//if this is a object pool, the initialized object number is DEFAULT_OBJ_MEM_POOL_NUM
#define DEFAULT_OBJ_MEM_POOL_NUM (4*1024)

//if this is a raw data buffer pool, default size is 1M = 1024*1024 bytes
#define DEFAULT_RAW_DATA_BUF_SIZE   (1024*1024*40)
//the chunk size is 128 bytes
#define DEFAULT_MIN_RAW_DATA_SIZE   (128)

//** here we are going to use buffer pool
//* this class is singleton, responsible to allocate io overlapped information
//* not free its memory space really, just put it into a list managed by windows
//* improve its allocation performance
//** every overlapped information can be reallocated
//@scope("singleton")
class IoOverlappedBufPool
{
public:
    IoOverlappedBufPool();
    ~IoOverlappedBufPool();

    //**initialize look-aside struct information proved by micro-soft
    //**return npp no error if no error happened when initialize, or return error code
    int initialize(bool useMemPool = false);
    //**finalize this overlapped pool
    void finalize();

    //**apply to allocate raw buffer from raw buffer pool alone
    //*inExpectedBufSize, it's the size that you really need to get
    //*outActualBufSize, the size that buffer pool allocate for you
    //**NULL if no enough buffer, or buffer you can use
    char* allocateRawBuffer(int inExpectedBufSize, int& outActualBufSize);
    void freeRawBuffer( char* buff, int actualSize );

    void writeDumpfile();
private:
    int _CaculateBufSize(int inExpectedSize);
    
    MemPool::CMemoryPool* m_rawBufferPool;        //raw data buffer pool
    bool m_useMemPool;
};
typedef ACE_Singleton<IoOverlappedBufPool , ACE_Null_Mutex> GlobalOverlappedBufPool; 

#endif