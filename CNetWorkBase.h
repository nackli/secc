#pragma once
#ifndef __NET_WORK_BASE__H__
#define __NET_WORK_BASE__H__
#include "CThreadBase.h"
#include "SystemType.h"
class CNetWorkBase: public CThreadBase
{
public:
    CNetWorkBase();
    virtual ~CNetWorkBase();  
    void InitNetWorkServerInfo(int iPort,bool bInet6 = true,bool bTcpConnect = true);
    void CloseNetWorkServer();
    int GetSocketHandle();
    bool SendGroupInfo();
    bool GetLocalIP(unsigned char ip_addr[46],const char *szNetName,bool bNetIp6);
private:
    int m_iSockHandle;
};
#endif