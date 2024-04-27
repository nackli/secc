#include <iostream>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <cstdlib>
#include "PosixMsg.h"
#include "LogManage.h"

#define FILE_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#define FREE_MEM(x) if((x)){free((x));(x) = NULL;}
int OnInitPosixMsg(const char *szMsgName,PROCMSG callBackFun,int iMsgSize)
{
    mqd_t iMsgId = -1;
    if(!szMsgName)
        return iMsgId;
    mq_attr tagAttr;
    tagAttr.mq_msgsize = 1024;
    tagAttr.mq_maxmsg = 50;
    
    iMsgId = mq_open(szMsgName, O_RDWR | O_CREAT | O_EXCL, 0666, &tagAttr);
    if(iMsgId == -1)
    {
        if (errno == EEXIST)
        {
            mq_unlink(szMsgName);
            iMsgId = mq_open(szMsgName, O_RDWR | O_CREAT, 0666, &tagAttr);
        }
        if(errno == EINVAL)
            iMsgId = mq_open(szMsgName, O_RDWR | O_CREAT, 0666, NULL);    
        else
            LOGOUT_ERROR("open mg error 1 = %d",FILE_MODE);       
    }
    else
        LOGOUT_ERROR("open mg error 0");   
    
    return iMsgId;
}

static bool SendPosixMsg(int iMsgId,const char *szData,int iSize,bool bBlock)
{
    bool fRet = false;
    if(!szData || iMsgId < 0 || !iSize)
        return fRet;
    // mq_attr mqAttr = {0};
    // if(mq_getattr(iMsgId, &mqAttr) < 0)
    //     return fRet;
    // if(!bBlock)
    //     mqAttr.mq_flags = O_NONBLOCK;
    // else
    //     mqAttr.mq_flags = 0;

    // mq_setattr(iMsgId,&mqAttr,NULL);

    //char *pSendData = (char *)malloc(iSize);
    //memcpy(pSendData,szData,iSize);
    if(0 == mq_send(iMsgId, szData, iSize, 0))
        fRet = true;
    return fRet;
}

int ReadPosixMsg(int iMsgId,char *szData,unsigned int iMaxSize)
{
    int fRet = -1;
    if(!szData || iMsgId < 0 || !iMaxSize)
        return fRet;  
    mq_attr mqAttr = {0};
    mq_getattr(iMsgId, &mqAttr);
    // mq_attr mqAttr = {0};
     if(mq_getattr(iMsgId, &mqAttr) < 0)
         return fRet;  
    
    char *pRecvData = (char *)malloc(mqAttr.mq_msgsize);
    memset(pRecvData, 0, mqAttr.mq_msgsize);
    if(!pRecvData)
    {
        LOGOUT_ERROR("Mem alloc fail");
        return fRet;
    }
        
    LOGOUT_DEBUG("mq_getattr = %ld,%ld ",mqAttr.mq_maxmsg,mqAttr.mq_msgsize);
    int iRecvSize = mq_receive(iMsgId, pRecvData, mqAttr.mq_msgsize, NULL); 
    if(iRecvSize <=0 )
    {
        LOGOUT_ERROR("Read msg queue fail");
        return fRet;
    }

    int iCpyDataSize = 0;
    iCpyDataSize =  iMaxSize > (unsigned int)iRecvSize ? iRecvSize : iMaxSize;
    memcpy(szData,pRecvData,iCpyDataSize);
    FREE_MEM(pRecvData);
    fRet = iCpyDataSize;
    return fRet;  
}

bool OnClosePosixMsg(int iMsgId)
{
    bool fRet = false;
    if(iMsgId < 0)
        return fRet;
    if(mq_close(iMsgId) == 0)
    {
        fRet = true;    
    }
        
    return fRet;
}

bool OnSendPosixMsg(int iMsgId,const char *szData,int iSize)
{
    return SendPosixMsg(iMsgId, szData, iSize, true);
}

bool OnDelPosixMsg(const char *szMsgName)
{
     bool fRet = false;
    if(!szMsgName)
        return fRet;
    if(mq_unlink(szMsgName) == 0)
        fRet = true;    
    return fRet;   
}