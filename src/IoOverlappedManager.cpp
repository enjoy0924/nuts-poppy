#include "IoOverlappedManager.h"
#include "NppLog.h"

CIoOverlappedplusManager::CIoOverlappedplusManager():m_uniqueId(0){}
CIoOverlappedplusManager::~CIoOverlappedplusManager(){}

IO_OVERLAPPEDPLUS* CIoOverlappedplusManager::getAvailableIoOverlapped(const SOCKET& sock, const int& bufferSize )
{
    IO_OVERLAPPEDPLUS* ioOverlappedPlus = NULL;
    int actualSize = 0;
    char* buffer = NULL;
    if (bufferSize > 0)
    {
        buffer = GlobalOverlappedBufPool::instance()->allocateRawBuffer(bufferSize, actualSize);
        if (NULL == buffer)
            return NULL;
    }

    ACE_Guard<ACE_Mutex> guard(m_mutex);
    std::hash_map<unsigned int, IO_OVERLAPPEDPLUS*>::iterator iter = m_hash.begin();
    while (iter != m_hash.end())
    {
        ioOverlappedPlus = iter->second;
        if (ioOverlappedPlus != NULL && ioOverlappedPlus->_nRef == 0)
        {
            ioOverlappedPlus->_nRef = 1;
            //set its re-occupancy
            ioOverlappedPlus->_nBufferTotalBytes = 0;
            ioOverlappedPlus->_nBytesTransferedThisTime = 0;
            ioOverlappedPlus->_nRemainBytesLastTime = 0;
            ioOverlappedPlus->_opertype = OP_NON;
            ioOverlappedPlus->_s = sock;
            ioOverlappedPlus->_wsabuf.buf = buffer;
            ioOverlappedPlus->_wsabuf.len = bufferSize;
            ioOverlappedPlus->_nActuallyBufferSize = actualSize;
            ioOverlappedPlus->_pOriginalBuffer = buffer;
            ZeroMemory(&ioOverlappedPlus->_overlapped, sizeof(OVERLAPPED));

            return ioOverlappedPlus;
        }
        iter++;
    }

    ioOverlappedPlus = new IO_OVERLAPPEDPLUS;
    if (NULL == ioOverlappedPlus)
    {
        GlobalOverlappedBufPool::instance()->freeRawBuffer(buffer, actualSize);
        return NULL;
    }
    //allocate a new identity for new io overlapped plus
    ioOverlappedPlus->_identity = m_uniqueId++;

    //set its occupancy
    ioOverlappedPlus->_s = sock;
    ioOverlappedPlus->_wsabuf.buf = buffer;
    ioOverlappedPlus->_wsabuf.len = bufferSize;
    ioOverlappedPlus->_nActuallyBufferSize = actualSize;
    ioOverlappedPlus->_pOriginalBuffer = buffer;
    ioOverlappedPlus->_nRef = 1;

    //insert into hash map for management
    m_hash[ioOverlappedPlus->_identity] = ioOverlappedPlus;

    return ioOverlappedPlus;
}

void CIoOverlappedplusManager::freeOccupancyIoOverlapped(IO_OVERLAPPEDPLUS* ioOverlappedPlus)
{
    if (NULL == ioOverlappedPlus || ioOverlappedPlus->_nRef == 0)
        return;

    ACE_Guard<ACE_Mutex> guard(m_mutex);
    std::hash_map<unsigned int, IO_OVERLAPPEDPLUS*>::iterator iter = m_hash.find(ioOverlappedPlus->_identity);
    if (iter != m_hash.end())
    {
        ioOverlappedPlus = iter->second;
        GlobalOverlappedBufPool::instance()->freeRawBuffer(ioOverlappedPlus->_pOriginalBuffer,
            ioOverlappedPlus->_nActuallyBufferSize);
    }
    ioOverlappedPlus->_nRef = 0;
}

void CIoOverlappedplusManager::dumpIoOverlapped()
{
    //GlobalOverlappedBufPool::instance()->writeDumpfile();

    //IO_OVERLAPPEDPLUS* ioOverlappedPlus = NULL;
    //ACE_Guard<ACE_Mutex> guard(m_mutex);
    //std::hash_map<unsigned int, IO_OVERLAPPEDPLUS*>::iterator iter = m_hash.begin();
    //while (iter != m_hash.end())
    //{
    //    ioOverlappedPlus = iter->second;
    //    if (ioOverlappedPlus != NULL)
    //    {
    //        //set its re-occupancy
    //    }
    //    iter++;
    //}
}