#include "CUdpServer.h"
#include "LogManage.h"
#include "CTcpIp6Server.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "CMsgManage.h"
#define ISO_151158_UDP_SDP_SERVER_PORT        15118
#define ISO_151158_TCP_PORT                   50001
#define ISO_151158_TLS_PORT                   50002
#ifndef HiByte
#define HiByte(x)                   (((x) & 0xFF00 ) >> 8 )
#endif
#ifndef LowByte
#define LowByte(x)                  ((x) & 0xFF )
#endif

CUdpServer::CUdpServer()
{
}

CUdpServer::~CUdpServer()
{

}

void CUdpServer::InitSeccUdpInfo()
{
    SendMsgUint8(EM_MSG_SDP_BEGIN);
    InitNetWorkServerInfo(ISO_151158_UDP_SDP_SERVER_PORT, true, false);
    SendGroupInfo();
    
}
//CTcpIp6Server *g_pTcpIp = NULL;
void CUdpServer::ThreadProcFun()
{
    if (this->GetSocketHandle() < 0)
        return;

    fd_set tagReadSet = {0};
    LOGOUT_NOTICE("<!-----------------Begin Sdp Info Wait--------------------->");
    FD_ZERO (&tagReadSet);
    FD_SET (GetSocketHandle(), &tagReadSet);
    if (select(FD_SETSIZE, &tagReadSet, NULL, NULL,NULL) <= 0)
    {
        LOGOUT_ERROR("Select read Error: %s",  strerror(errno));
        return;
    }
    
    struct sockaddr_in6 tagFromAddr = {0};
    size_t uAddrLen = sizeof(tagFromAddr);
    uint8_t uRecvBuf[256] = {0};
    int iRecvLen = 0;
    if((iRecvLen = recvfrom(GetSocketHandle(), uRecvBuf, sizeof(uRecvBuf), 0, (struct sockaddr*)&tagFromAddr,&uAddrLen)) > 0)
    {        
        HLOGOUT_DEBUG(uRecvBuf, iRecvLen);
        // 比较粗的做法, 直接固定格式 0x0--0x8000保留，0x8001->exi 0x9000->请求标识，0x9001->res,0x9002-0x9fff保留，0xa000-0xffff厂家自定义
        const unsigned char arrSdpRequestMsg[] = {0x01, 0xfe, 0x90, 0x00, 0x00, 0x00, 0x00, 0x02};
        //  format and length check
        if (iRecvLen != 10 || memcmp((unsigned char *)arrSdpRequestMsg, uRecvBuf, 8) != 0)
            return;
        // SDP_response_message default tcp mode
        unsigned char arrResponseMessage[] = 
        {
          0x01, 0xfe, 0x90, 0x01, 0x00, 0x00, 0x00, 0x14,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 地址
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 地址
          0x3b, 0x0e, 0x10, 0x00                            // 0x3b 0x0e 端口号  15118 高位在前
        };

        if (uRecvBuf[9] != 0x00) 
        {
            // 0x00= TCP  0x01-0x0F = reserved  0x10 = reserved for UDP 0x11-0xFF = reserved
            LOGOUT_ERROR("Invalid protocol type 0x%x", uRecvBuf[9]);
            return;
        }
        LOGOUT_INFO("Udp Req Prototype =  0x%x", uRecvBuf[8]);
        // 0x00 = secured with TLS  0x01-0x0F = reserved  0x10 = No transport layer security  0x11-0xFF = reserved  TLS 连接   Security byte
        if (uRecvBuf[8] == 0x00) 
        {
            arrResponseMessage[24] = HiByte(ISO_151158_TCP_PORT);
            arrResponseMessage[25] = LowByte(ISO_151158_TCP_PORT);
            arrResponseMessage[26] = 0x10;
            arrResponseMessage[27] = 0x00;//传输协议，0x00 = TCP 0x01-0x0F = 保留 0x10 = 为 UDP 保留 0x11-0xFF = 保留
        }
        else if (uRecvBuf[8] == 0x10)
        {
            arrResponseMessage[24] = HiByte(ISO_151158_TCP_PORT);
            arrResponseMessage[25] = LowByte(ISO_151158_TCP_PORT);
            arrResponseMessage[26] = 0x10;
            arrResponseMessage[27] = 0x00;
        } else {
            LOGOUT_ERROR("Invalid protocol secured type %d", uRecvBuf[8]);
            return;
        }
        // Update locale ip
        GetLocalIP(&arrResponseMessage[8], "eth1",true);
        if(sendto (GetSocketHandle(), arrResponseMessage, sizeof(arrResponseMessage), 0,(struct sockaddr*) &tagFromAddr, sizeof(tagFromAddr)) >= 0)
        {
            SendMsgUint8(EM_MSG_SDP_SUCCESS);
            // LOGOUT_NOTICE("<!-----------------Finsh Sdp Info--------------------->");
            // CTcpIp6Server *g_pTcpIp = new CTcpIp6Server;
            // g_pTcpIp->InitTcpServerInfo();
            // g_pTcpIp->StartThread();
            // LOGOUT_NOTICE("<!-----------------create Tcp Link Ok--------------------->");
        }
    }
}
