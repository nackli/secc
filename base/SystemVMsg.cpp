#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <error.h>
#include <cerrno>
#include "LogManage.h"

int OnInitSysVMsg(int iKeyId)
{
    //IPC_PRIVATE ftok("msg.tmp", 0x01 )
    int iMsgId = msgget((key_t)iKeyId, 0666 | IPC_CREAT);
    if (iMsgId < 0 )
        LOGOUT_ERROR("get ipc_id error : %s" ,strerror(errno)) ;
    return iMsgId;
}


static bool SndSysVMsg(int iMsgId, void *pParam, unsigned int iSize,int iMsgFlag)
{
    bool fRet = false;
    if(iMsgId < 0 || !pParam || !iSize)
        return fRet;

    // LP_SYS_V_MSG pSysVMsg = new SYS_V_MSG;
    // pSysVMsg->iMsgType = iMsgType;
    // memcpy(pSysVMsg->szText, pParam, iSize);
    // SYS_V_MSG tagSysVMsg = {0};
    // tagSysVMsg.iMsgType = iMsgType;
    // memcpy(tagSysVMsg.szText, pParam, iSize);

    if(msgsnd( iMsgId, pParam, iSize , iMsgFlag )< 0)
        LOGOUT_ERROR("send msg fail : %s",strerror(errno));
    else
        fRet = true;
    return fRet;
}

bool OnSndSysVMsg(int iMsgId, void *pParam, unsigned int iSize)
{
    return SndSysVMsg(iMsgId, pParam,  iSize, 0);
}

bool OnPostSysVMsg(int iMsgId, void *pParam, unsigned int iSize)
{
    return SndSysVMsg(iMsgId, pParam,  iSize, IPC_NOWAIT);
}

int ReadSysVMsg(int iMsgId, void *pParam, unsigned int iSize)
{
    int fRet = -1;
    if(iMsgId < 0 || !pParam || !iSize)
        return fRet;
    fRet = msgrcv( iMsgId, pParam, iSize , 0, 0);
    if(fRet <= 0)
        LOGOUT_ERROR("read msg fail : %s",strerror(errno));      
    return fRet;
}

bool OnRemoveSysVMsg(int iMsgId)
{
    if( msgctl( iMsgId, IPC_RMID, 0) == -1)
        return false;
    char szCmdBuf[64] = {0};
    sprintf(szCmdBuf,"ipcrm -q %d",iMsgId);
    if(system(szCmdBuf) >=0)
        return true;
    return false;
}