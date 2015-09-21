#ifndef __SOCK_ENV_H__
#define __SOCK_ENV_H__

//****************************************
class CWinSockEnvironment
{
public:
    CWinSockEnvironment(){}
    ~CWinSockEnvironment(){}

    static bool InitWinSock();
    static void FiniWinSock();
};

#endif 