#pragma once
#ifndef ___SECC_MANAGE__H__
#define ___SECC_MANAGE__H__
#include "CThreadBase.h"
class CSlacManage;
class CUdpServer;
class CTcpIp6Server;
class C15118Protocol;
class CSeccManage: public CThreadBase
{
public:
    CSeccManage();
    virtual ~CSeccManage();
    void InitSeccDevInfo();
    void UninitSeccDevInfo();
private:    
    virtual void ThreadProcFun();
private:
    CSlacManage     *m_SlacMange;   
    CUdpServer      *m_UdpServer; 
    CTcpIp6Server   *m_TcpIp6Server;
    C15118Protocol  *m_15118Protocol;
};
#endif
