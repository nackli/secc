#pragma once
#ifndef __UDP_SERVER__H__
#define __UDP_SERVER__H__
#include "CNetWorkBase.h"
class CUdpServer: public CNetWorkBase
{
public:
    CUdpServer();
    virtual ~CUdpServer();  
    void InitSeccUdpInfo();
private:      
    virtual void ThreadProcFun();   
};
#endif