#include "CTcpIp6Server.h"
#include "LogManage.h"
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
#include "SeccErrorCode.h"
#include "CMsgManage.h"

#define ISO_151158_UDP_SDP_SERVER_PORT        15118
#define ISO_151158_TCP_PORT                   50001
#define ISO_151158_TLS_PORT                   50002


void CTcpIp6Server::InitTcpServerInfo()
{
    this->InitNetWorkServerInfo(ISO_151158_TCP_PORT);
}

CTcpIp6Server::CTcpIp6Server()
{
    //m_iCurStatus = EM_SECC_STATE_UNKOWN;
}

CTcpIp6Server::~CTcpIp6Server()
{
    //m_iCurStatus = EM_SECC_STATE_UNKOWN;
}

void CTcpIp6Server::ThreadProcFun()
{
    if (this->GetSocketHandle() < 0)
        return;

    struct sockaddr_in6 tagClientAddr = {0};
    socklen_t uClientAddrLen = sizeof(tagClientAddr);

    /* Do TCP handshake with client */
    int ClientSockfd = accept(GetSocketHandle(), (struct sockaddr*)&tagClientAddr, &uClientAddrLen);
    if (ClientSockfd == -1) 
    {
        LOGOUT_ERROR("Accept error:  %s", strerror(errno));
        return;
    }  
    char str_addr[64];
    inet_ntop(AF_INET6, &(tagClientAddr.sin6_addr), str_addr, sizeof(str_addr));
    LOGOUT_DEBUG("New TCP connection from: %s:%d ,%d...", str_addr,ntohs(tagClientAddr.sin6_port), ClientSockfd);
    LP_SEND_TCP_CONNECT pSndTcpConnnect = new SEND_TCP_CONNECT;
    pSndTcpConnnect->iCmdType = EM_MSG_TCP_NEW_CONNECT;
    pSndTcpConnnect->iSocketHandle = ClientSockfd;
    SendMsg(pSndTcpConnnect, sizeof(SEND_TCP_CONNECT));
}
