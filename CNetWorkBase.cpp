#include "CNetWorkBase.h"
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
#define ISO_151158_UDP_SDP_SERVER_PORT        15118
#define ISO_151158_TCP_PORT                   50001
#define ISO_151158_TLS_PORT                   50002
#ifndef HiByte
#define HiByte(x)                   (((x) & 0xFF00 ) >> 8 )
#endif
#ifndef LowByte
#define LowByte(x)                  ((x) & 0xFF )
#endif

static bool OnInitNetWorkServer(int &iSockHandle,int iPort,bool bInet6 = true,bool bTcpConnect = true)
{
    if(iSockHandle > 0)
    {
        LOGOUT_ERROR("Socket already exists");
        return true;
    }   
    struct sockaddr_in6 tagServerAddr = {0};
    int iNetType = AF_INET;
    int iProtocol = SOCK_DGRAM;
    if(bInet6)
        iNetType = AF_INET6;
    if(bTcpConnect)
        iProtocol = SOCK_STREAM;

	iSockHandle = socket(iNetType, iProtocol, 0);
	if(iSockHandle == -1) {
		LOGOUT_ERROR("Create socket error : %s", strerror(errno));
		return false;
	}
    LOGOUT_DEBUG("Net Info : %d, %d",  iNetType, iProtocol);
	tagServerAddr.sin6_family = iNetType;
	tagServerAddr.sin6_addr = in6addr_any;
	tagServerAddr.sin6_port = htons(iPort);


    int iFlag = 0;
    int fRet = -1;
    if(bTcpConnect)
    {
        iFlag = 0;
        fRet = setsockopt(iSockHandle, SOL_SOCKET, SO_SNDBUF , &iFlag, sizeof(iFlag));
        if(fRet == -1)
            LOGOUT_ERROR("Bind socket error : %s",  strerror(errno));

        fRet = setsockopt(iSockHandle, SOL_SOCKET, SO_RCVBUF , &iFlag, sizeof(iFlag));
        if(fRet == -1)
            LOGOUT_ERROR("Bind socket error : %s",  strerror(errno));    

        iFlag = 1;
        fRet = setsockopt(iSockHandle, SOL_SOCKET, SO_KEEPALIVE , &iFlag, sizeof(iFlag));
        if(fRet == -1)
            LOGOUT_ERROR("Bind socket error : %s",  strerror(errno));               
    }

    iFlag = 0;
    fRet = setsockopt(iSockHandle, SOL_SOCKET, SO_REUSEADDR , &iFlag, sizeof(iFlag));
    if(fRet == -1)
        LOGOUT_ERROR("Bind socket error : %s",  strerror(errno));

    // fRet = setsockopt(iSockHandle, SOL_SOCKET, SO_REUSEPORT , &iFlag, sizeof(iFlag));
    // if(fRet == -1)
    //     LOGOUT_ERROR("Bind socket error : %s",  strerror(errno));    
    
	fRet = bind(iSockHandle, (struct sockaddr*)&tagServerAddr, sizeof(tagServerAddr));
	if(fRet == -1) {
        LOGOUT_ERROR("Bind socket error : %s",  strerror(errno));
		close(iSockHandle);
        iSockHandle = -1;
        return false;
	}

    if(bTcpConnect)
    {
        /* Create listening queue (client requests) */
        fRet = listen(iSockHandle, MAX_LISTEN_NUM);
        if (fRet == -1) {
            LOGOUT_ERROR("Listen socket error. errno : %s", strerror(errno));
            close(iSockHandle);
            iSockHandle = -1;
            return false;
        }
    }
    return true;
}


void CNetWorkBase::InitNetWorkServerInfo(int iPort, bool bInet6, bool bTcpConnect)
{
    OnInitNetWorkServer(m_iSockHandle, iPort, bInet6, bTcpConnect);
    //StartThread();
}

void CNetWorkBase::CloseNetWorkServer()
{
    if(m_iSockHandle < 0)
        return;
    close(m_iSockHandle);
    m_iSockHandle = -1;

}

CNetWorkBase::CNetWorkBase()
{
    m_iSockHandle = -1;
}

CNetWorkBase::~CNetWorkBase()
{

}

int CNetWorkBase::GetSocketHandle()
{
    return m_iSockHandle;
}

bool CNetWorkBase::SendGroupInfo()
{
    if(m_iSockHandle < 0)
        return false;
    struct ipv6_mreq tagGroup = {0};
    tagGroup.ipv6mr_interface = 0;
    inet_pton(AF_INET6, "FF02::1", &tagGroup.ipv6mr_multiaddr);
    int fRet = setsockopt(m_iSockHandle, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &tagGroup, sizeof(tagGroup));
	if(fRet == -1) {
        LOGOUT_ERROR("Join multicast error s: %s\n",  strerror(errno));
		close(m_iSockHandle);
        m_iSockHandle = -1;
        return false;
	}
    return true;
}

bool CNetWorkBase::GetLocalIP(unsigned char ip_addr[16],const char *szNetName,bool bNetIp6)
{
    struct ifaddrs *ifa=NULL,*ifEntry=NULL;  
    int rc = 0;
    char addressBuffer[INET6_ADDRSTRLEN] = {0};
    int iAfNet = AF_INET;
    if(bNetIp6)
        iAfNet = AF_INET6;
    rc = getifaddrs(&ifa);
    if (rc==0)
    {
        for(ifEntry=ifa; ifEntry!=NULL; ifEntry=ifEntry->ifa_next)
        {
            if(ifEntry->ifa_addr->sa_data == NULL || ifEntry->ifa_addr == NULL) 
               continue;

            if(ifEntry->ifa_addr->sa_family==iAfNet)
            {
                void *addPtr = NULL; 
                if(AF_INET6 == iAfNet)
                    addPtr = &((struct sockaddr_in6 *)ifEntry->ifa_addr)->sin6_addr;
                else
                    addPtr = &((struct sockaddr_in *)ifEntry->ifa_addr)->sin_addr;

                if (strcmp(szNetName, ifEntry->ifa_name) == 0)
                {
                    memcpy(ip_addr, addPtr, 16);
                    const char *ip_str = inet_ntop(ifEntry->ifa_addr->sa_family, addPtr, addressBuffer, sizeof(addressBuffer));
                    LOGOUT_INFO("IPV ADDR : %s %s", ifEntry->ifa_name, ip_str);

                    addPtr = &((struct sockaddr_in6 *)ifEntry->ifa_netmask)->sin6_addr;
                    ip_str = inet_ntop(ifEntry->ifa_addr->sa_family, addPtr, addressBuffer, sizeof(addressBuffer));
                    LOGOUT_INFO("IPV MASK : %s %s", ifEntry->ifa_name, ip_str);

                    freeifaddrs(ifa);
                    ifa = NULL;
                    return true;
                }
            }
        }
    }
    freeifaddrs(ifa);
    ifa = NULL;
    return false;
}
