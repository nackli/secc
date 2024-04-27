#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SystemVMsg.h"
#include "CMsgManage.h"

static int g_iMsgHandle = 0;
int InitMsg(int iKeyId)
{
    g_iMsgHandle = OnInitSysVMsg(1234);
    return g_iMsgHandle;
}

bool SendMsg(void *pParam,unsigned int iSize)
{
    return OnSndSysVMsg(g_iMsgHandle, pParam, iSize);
}

bool SendMsgUint8(uint8_t iMsgFlag)
{
    return SendMsg(&iMsgFlag, sizeof(iMsgFlag));
}

bool PostMsg(void *pParam,unsigned int iSize)
{
    return OnPostSysVMsg(g_iMsgHandle, pParam, iSize);

}

bool ReadMsg(void *pParam, unsigned int iSize)
{
    return ReadSysVMsg(g_iMsgHandle, pParam, iSize);
}

bool RemoveMsg()
{
    return OnRemoveSysVMsg(g_iMsgHandle);
}