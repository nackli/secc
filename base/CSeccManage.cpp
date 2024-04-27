#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "CSeccManage.h"
#include "SysDeviceCtrl.h"
#include "CMsgManage.h"
#include "C15118Protocol.h"

#include "CUdpServer.h"
#include "LogManage.h"
#include "CSlacManage.h"
#include "CTcpIp6Server.h"
#define FREE_MEM(x) if((x)){free((x));(x) = NULL;}
#define DEL_MEM(x) if((x)){delete((x));(x) = NULL;}
#define DEL_MEMS(x) if((x)){delete([](x));(x) = NULL;}
CSeccManage::CSeccManage()
{
    m_SlacMange = NULL;
    m_UdpServer = NULL;
    m_TcpIp6Server = NULL;
}

CSeccManage::~CSeccManage()
{
    DEL_MEM(m_SlacMange);
    DEL_MEM(m_UdpServer);
    DEL_MEM(m_TcpIp6Server);
}

void CSeccManage::ThreadProcFun()
{
    char szCmdInfo[128] = {0};    
    bool fRet = ReadMsg(szCmdInfo,sizeof(szCmdInfo));
    if(fRet <= 0)
        return;
    uint8_t uCmdType = *szCmdInfo;
    switch (uCmdType)
    {
        case EM_MSG_SLAC_MATCH:
            m_UdpServer->StartThread();
            m_SlacMange->ExitAndStopThread();
        break;
        case EM_MSG_PLUG_IN:
            OnSetPwmWidth(0, 5);
            m_SlacMange->StartThread();
            m_SlacMange->OnSendSetKeyReq();
       // m_UdpServer->StartThread();
        //m_SlacMange->ExitAndStopThread();
        break;   
        case EM_MSG_PLUG_OUT:
            OnSetPwmWidth(0, 100);
            m_UdpServer->ExitAndStopThread();
            m_SlacMange->ExitAndStopThread();
        break;   
        case EM_MSG_SDP_SUCCESS:
            //m_SlacMange->ExitAndStopThread();
            m_TcpIp6Server->StartThread();
        break;
        case EM_MSG_TCP_NEW_CONNECT:
            {
                LP_SEND_TCP_CONNECT lpSendTcpConnnet = (LP_SEND_TCP_CONNECT)szCmdInfo;
                m_15118Protocol->SetSocketHandle(lpSendTcpConnnet->iSocketHandle);
                m_15118Protocol->StartThread();
            }
            break;
    default:
        break;
    }
}

void CSeccManage::InitSeccDevInfo()
{
    InitMsg(1234);
    OnInitDevInfo();
    OnSetPwmWidth(0, 100);
    HardwareResetPlc();
   
    m_SlacMange = new CSlacManage;
    m_SlacMange->InitSlacInfo();
    m_UdpServer = new CUdpServer;
    m_UdpServer->InitSeccUdpInfo();
    m_TcpIp6Server = new CTcpIp6Server;
    m_TcpIp6Server->InitTcpServerInfo();

    m_15118Protocol = new C15118Protocol();
}

void CSeccManage::UninitSeccDevInfo()
{
    OnSetPwmWidth(0, 100);
    RemoveMsg();
    m_SlacMange->UnInitSlacInfo();
    m_UdpServer->CloseNetWorkServer();
    m_SlacMange->ExitAndStopThread();
    m_UdpServer->ExitAndStopThread();   
}

