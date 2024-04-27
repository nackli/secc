/**
 * @file V2GTP.h
 * @author w
 * @date 2018/10/23
 * @brief
 *
 *    V2GTP消息头的编码和解码
 */


#ifndef __CC_V2GTP_H__
#define __CC_V2GTP_H__



#include "global.h"



#define MAX_V2G_PACKET_SIZE         4096*2


/* generic V2GTP header length */
#define V2GTP_HEADER_LENGTH 8

/* define V2GTP Version */
#define V2GTP_VERSION       0x01
#define V2GTP_VERSION_INV   0xFE

/* define V2GTP payload types*/
#define V2GTP_EXI_TYPE      0x8001

#define V2GTP_SDP_TYPE      0x8001



typedef struct
{
    unsigned char   version;
    unsigned char   inverse_version;
    unsigned short  payload_type;
    int             payload_length;

    unsigned char   *payload;   // 指向数据域
} V2GTP_HEAD_T;


#define FILL_V2GTP(head) \
    do{\
        head.version = V2GTP_VERSION;\
        head.inverse_version = V2GTP_VERSION_INV; \
        head.payload_type = V2GTP_EXI_TYPE; \
        head.payload_length = 0; \
        head.payload = NULL;\
    }while(0);



int DecodeV2GTP(unsigned char *p, int len, V2GTP_HEAD_T *v2gtp);

int EncodeV2GTP(const V2GTP_HEAD_T *v2gtp, unsigned char *dest, int max_dest_size);



#endif

