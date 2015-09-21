#ifndef __IO_OVERLAPPED_MANGAGER_H__
#define __IO_OVERLAPPED_MANGAGER_H__
#include "ace/ace_header.h"
#include "IoOverlappedPlus.h"
#include "IoOverlappedBufPool.h"
#include <hash_map>

class CIoOverlappedplusManager
{
public:
    CIoOverlappedplusManager();
    ~CIoOverlappedplusManager();

    IO_OVERLAPPEDPLUS* getAvailableIoOverlapped(const SOCKET& sock=INVALID_SOCKET, const int& bufferSize = 0);
    void freeOccupancyIoOverlapped(IO_OVERLAPPEDPLUS* ioOverlappedPlus);

    void dumpIoOverlapped();

protected:
private:
    unsigned int m_uniqueId;
    ACE_Mutex m_mutex;
    std::hash_map<unsigned int, IO_OVERLAPPEDPLUS*> m_hash;
};

typedef ACE_Singleton<CIoOverlappedplusManager , ACE_Null_Mutex> GlobalOverlappedPlusMgr; 

#endif