#ifndef __MSG_MANAGE__H__
#define __MSG_MANAGE__H__
#include <stdint.h>
typedef enum EM_MSG_ID
{
    EM_MSG_INIT = 0,
    EM_MSG_SLAC_MATCH,
    EM_MSG_PLUG_IN,
    EM_MSG_PLUG_CHAGE,
    EM_MSG_PLUG_OUT,
    EM_MSG_PLUS_UNKOWN,
    EM_MSG_SDP_BEGIN,
    EM_MSG_SDP_SUCCESS,
    EM_MSG_TCP_NEW_CONNECT,
    
    EM_MSG_MAX
}EM_MSG_ID;

int InitMsg(int iKeyId);
bool SendMsg(void *pParam,unsigned int iSize);
bool PostMsg(void *pParam,unsigned int iSize);
bool ReadMsg(void *pParam, unsigned int iSize);
bool SendMsgUint8(uint8_t iMsgFlag);
bool RemoveMsg();
#endif