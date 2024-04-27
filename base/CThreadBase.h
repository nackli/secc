#pragma once
#ifndef ___THREAD_BASE___
#define ___THREAD_BASE___
#include <pthread.h>
class CThreadBase
{
public:
    CThreadBase();
    virtual ~CThreadBase();
    void StartThread(pthread_attr_t *pThreadAttr = NULL);
    bool IsRuning();
    bool ExitAndStopThread();
    virtual void ThreadProcFun() = 0;
private:
    unsigned long m_iThreadId;
    bool m_bThreadRuning;
};
#endif