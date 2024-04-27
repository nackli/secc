/**
 * @file V2GTP.cpp
 * @author w
 * @date 2018/10/23
 * @brief
 *
 *    V2GTP消息头的编码和解码
 */



#include "V2GTP.h"



int DecodeV2GTP(unsigned char *dat, int len, V2GTP_HEAD_T *v2gtp)
{
    /**
        0x01:
        V2GTP version 1
        0x00, 0x02-0xFF:
        reserved by document
    */
    int payload_length;

    /* 版本检查，目前仅支持 版本1,第二字节是一字节取反 */
    if (dat[0] != V2GTP_VERSION || dat[1] != V2GTP_VERSION_INV)
        return ERR_V2G_VERSION;

	v2gtp->payload_type = dat[2];
	v2gtp->payload_type = (v2gtp->payload_type << 8 | dat[3]);

	payload_length = dat[4];
	payload_length = (payload_length << 8 | dat[5]);
	payload_length = (payload_length << 8 | dat[6]);
	payload_length = (payload_length << 8 | dat[7]);
    v2gtp->payload_length = payload_length;
    v2gtp->payload = &dat[8];

    if (payload_length == 0)
        return ERR_V2G_PAYLOAD_LENGTH;

    /* 20230207 add by gaopengjie */ 
    /* 负载长度与实际接收长度不同，可能出现粘包，需要后续解码处理 */
    if (payload_length != len - V2GTP_HEADER_LENGTH)
        return payload_length;

    return payload_length;
}



int EncodeV2GTP(const V2GTP_HEAD_T *v2gtp, unsigned char *dest, int max_dest_len)
{
    if (max_dest_len < v2gtp->payload_length + V2GTP_HEADER_LENGTH)
        return ERR_V2G_PAYLOAD_LENGTH;

    dest[0] = v2gtp->version;
    dest[1] = v2gtp->inverse_version;
    dest[2] = HiByte(v2gtp->payload_type);
    dest[3] = LowByte(v2gtp->payload_type);
    dest[4] = HHByte(v2gtp->payload_length);
    dest[5] = HLByte(v2gtp->payload_length);
    dest[6] = LHByte(v2gtp->payload_length);
    dest[7] = LLByte(v2gtp->payload_length);

    memcpy(&dest[8], v2gtp->payload, v2gtp->payload_length);
    return v2gtp->payload_length + V2GTP_HEADER_LENGTH;
}

