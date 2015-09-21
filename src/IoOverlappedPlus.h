#ifndef __IO_OVERLAPPED_PLUS_H__
#define __IO_OVERLAPPED_PLUS_H__
#include <winsock2.h>
//**************io operation type definition******************
typedef enum _ENUM_OPERATION_TYPE
{  
    OP_ACCEPT=0,
    OP_SEND,
    OP_RECV,
    OP_NON,
    OP_CONN
}ENUM_OPERATION_TYPE;

//*************io overlapped******************
typedef struct _IO_OVERLAPPEDPLUS{
    OVERLAPPED     _overlapped;
    SOCKET             _s, _sclient;
    WSABUF            _wsabuf;
    ENUM_OPERATION_TYPE _opertype;
    
    //the original buffer address, indicate wsabuf.buf firstly, never change it again, or cause memory leak
    //[********wsabuf.buf may not be the same as _pOriginalBuffer*********]
    char* _pOriginalBuffer;
    // the actual buffer size, use to deallocate from buffer pool, assigned when allocate from buffer pool
    //[***********do not change its value during the runtime**********]
    int _nActuallyBufferSize;

    int _nRemainBytesLastTime;       //total bytes remaining in our buffer for recv, bytes already sent for send 
    int _nBufferTotalBytes;               //total bytes in our buffer for recv, total bytes for one send operation
    int _nBytesTransferedThisTime;  //total bytes transfered on one specific time

    volatile int _nRef;
    unsigned int _identity;

    _IO_OVERLAPPEDPLUS():_nBufferTotalBytes(0),
        _nBytesTransferedThisTime(0),
        _nRemainBytesLastTime(0),
        _nActuallyBufferSize(0),
        _opertype(OP_NON),
        _s(INVALID_SOCKET),
        _sclient(INVALID_SOCKET),
        _pOriginalBuffer(NULL),
        _nRef(0),
        _identity(0)
    {
        ZeroMemory(&_overlapped, sizeof(OVERLAPPED));
        _wsabuf.buf = NULL;
        _wsabuf.len = 0;
    }
    ~_IO_OVERLAPPEDPLUS()
    {

    }
}IO_OVERLAPPEDPLUS, *PIO_OVERLAPPEDPLUS;

#endif