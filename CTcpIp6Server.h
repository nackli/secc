#pragma once
#ifndef __TCP_IP6_SERVER__H__
#define __TCP_IP6_SERVER__H__
#include "CNetWorkBase.h"
#include <stdint.h>
class CTcpIp6Server: public CNetWorkBase
{
public:
    CTcpIp6Server();
    virtual ~CTcpIp6Server();  
    void InitTcpServerInfo();
private:      
    virtual void ThreadProcFun();   
};
#endif