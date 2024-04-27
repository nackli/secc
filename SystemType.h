#ifndef __SYS_TYPE_DEFINE__
#define __SYS_TYPE_DEFINE__
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
#define MAX_LISTEN_NUM      10
typedef struct SEND_TCP_CONNECT
{
    uint8_t iCmdType;
    int iSocketHandle;
}SEND_TCP_CONNECT, *LP_SEND_TCP_CONNECT;
#endif


