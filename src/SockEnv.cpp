#include "SockEnv.h"
#include "winsock.h"

bool CWinSockEnvironment::InitWinSock()
{    
    WSADATA wsaData;
    int nResult;
    nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (NO_ERROR != nResult)
    {
        return false; 
    }

    return true;
}

void CWinSockEnvironment::FiniWinSock()
{
    WSACleanup();
}

