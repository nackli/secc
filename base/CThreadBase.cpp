#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <cerrno>
#include "CThreadBase.h"
#include "LogManage.h"

CThreadBase::CThreadBase()
{
    m_iThreadId = 0;
    m_bThreadRuning = false;
}

CThreadBase::~CThreadBase()
{
    m_iThreadId = 0;
    m_bThreadRuning = false;
}

static void *OnThreadProcFun(void *pThreadParam)
{
    if(!pThreadParam)
        return NULL;
    CThreadBase *pSelfThis = (CThreadBase *)pThreadParam;
    while(pSelfThis->IsRuning())
    {
        pSelfThis->ThreadProcFun();    
    }
    return NULL;
}

void CThreadBase::StartThread(pthread_attr_t *pThreadAttr)
{
    m_bThreadRuning = true;
    int iRetNum = pthread_create(&m_iThreadId, pThreadAttr, OnThreadProcFun, this);
    if (iRetNum != 0)
        LOGOUT_ERROR("can't create thread: %s\n", strerror(iRetNum));         
}

bool CThreadBase::IsRuning()
{
    return m_bThreadRuning;
}

bool CThreadBase::ExitAndStopThread()
{
    m_bThreadRuning = false;
    if(m_iThreadId)
    {
        pthread_cancel(m_iThreadId);
        pthread_join(m_iThreadId,NULL);
    }
    return true;
} 