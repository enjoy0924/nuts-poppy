#include "IoOverlappedBufPool.h"
#include "NppLog.h"
#include "npp.h"

IoOverlappedBufPool::IoOverlappedBufPool():
        m_rawBufferPool(NULL),
        m_useMemPool(false)
{
}

IoOverlappedBufPool::~IoOverlappedBufPool()
{
}

//**initialize look-aside struct information proved by micro-soft
//**return npp no error if no error happened when initialize, or return error code
int IoOverlappedBufPool::initialize(bool useMemPool)
{
    m_useMemPool = useMemPool;
    //if not use memory pool mechanism, just return here
    if (!m_useMemPool)
        return NPP_NOERROR;
            
    m_rawBufferPool = new MemPool::CMemoryPool(
        DEFAULT_RAW_DATA_BUF_SIZE,   //default total size = 1M = 1024K = 1048576 bytes
        DEFAULT_MIN_RAW_DATA_SIZE*(8)*(4),   //every chunk's size = 4k = 4096 bytes
        DEFAULT_MIN_RAW_DATA_SIZE);    //default min data bulk size = 128 bytes
    if (NULL == m_rawBufferPool)
    {
        return NPP_ENOTENOUGHSRC;
    }
    return NPP_NOERROR;
}

//**finalize this overlapped pool
void IoOverlappedBufPool::finalize()
{
    if (NULL != m_rawBufferPool)
    {
        delete m_rawBufferPool;
        m_rawBufferPool = NULL;
    }
    m_useMemPool = false;
    return;
}
//**apply to allocate raw buffer from raw buffer pool alone
//*inExpectedBufSize, it's the size that you really need to get
//*outActualBufSize, the size that buffer pool allocate for you
//**NULL if no enough buffer, or buffer you can use
char* IoOverlappedBufPool::allocateRawBuffer(int inExpectedBufSize, int& outActualBufSize)
{
    if (inExpectedBufSize <= 0)
        return NULL;
    char* buffer = NULL;
    outActualBufSize = _CaculateBufSize(inExpectedBufSize);
    if (m_useMemPool)
        buffer = reinterpret_cast<char*>(m_rawBufferPool->GetMemory(outActualBufSize));
    else
        buffer = new char[outActualBufSize];

    return buffer;
}

void IoOverlappedBufPool::freeRawBuffer( char* buffer, int actualSize )
{
    if (NULL != buffer)
    {
        if (m_useMemPool)
        {
            if (actualSize>0)
            {
                m_rawBufferPool->FreeMemory(buffer, actualSize);
            }
        }
        else
            delete[] buffer;
    }
}

void IoOverlappedBufPool::writeDumpfile()
{
    if (NULL != m_rawBufferPool && m_useMemPool)
    {
        m_rawBufferPool->WriteMemoryDumpToFile("c:\\t.txt");
    }
}

//**calculate the suitable buffer size according to inExpectedSize
//*inExpectedSize,
//**return the suitable size
int IoOverlappedBufPool::_CaculateBufSize(int inExpectedSize)
{
    if (m_useMemPool)
    {
        int remain = inExpectedSize%DEFAULT_MIN_RAW_DATA_SIZE;
        if (remain != 0)
        {
            inExpectedSize = inExpectedSize + (DEFAULT_MIN_RAW_DATA_SIZE - remain);
        }
    }

    return inExpectedSize;
}
