#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h> 
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include "TimerManage.h"
#include "LogManage.h"


typedef void(*INIT_TIMER_FUN)(int iCmdId);
typedef struct TIMER_INFO_PARAM{
    INIT_TIMER_FUN fun;
    int iCmdId;
    void * hTimerHandle;
}TIMER_INFO_PARAM,*LP_TIMER_INFO_PARAM;

// static void SigTimerFun(SIGVAL_UNION sigParam)
// {
//     if(!sigParam.sival_ptr)
//         return;
//     LP_TIMER_INFO_PARAM lpTimerInfoParam = (LP_TIMER_INFO_PARAM)sigParam.sival_ptr;
//     lpTimerInfoParam->fun(lpTimerInfoParam->iCmdId);
// }

static void * OnCreateTimer(SIGNAL_FUN fun,void *pParam)
{
    timer_t TimerId = NULL;
    struct sigevent sevp = {0};  // argument to timer_create
    pthread_attr_t attr = {0};
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr,SCHED_OTHER);
    sevp.sigev_notify = SIGEV_THREAD;
    sevp.sigev_notify_attributes = NULL;
    sevp.sigev_value.sival_ptr = pParam;
    sevp.sigev_notify_function= fun; 
    if (timer_create(CLOCK_REALTIME, &sevp, &TimerId) == -1)
    {
        fprintf(stderr, "LeakTracer (timer_trackStartTime): timer_create failed   to create timer. " \
                        "Leak measurement will be for entire duration of the execution   period:%s \n", strerror(errno));
        return 0;
    }
    pthread_attr_destroy(&attr);
    return TimerId;
}

bool OnStopTimer(HANDLE hTimerId)
{
    bool fRet = false;
    if(hTimerId)
    {
        if(timer_delete(hTimerId) == 0)
        {   
            hTimerId = NULL;
            fRet = true;
        }      
    }
    return fRet;
}

HANDLE OnSetTimer(unsigned long uMSec,SIGNAL_FUN fun,void *pParam)
{
    if(!fun)
        return NULL;
    HANDLE hTimer = OnCreateTimer(fun,pParam);
    int iSecNum = uMSec / 1000;
    int iMSec = uMSec % 1000;
    struct itimerspec its = {0};
    its.it_interval.tv_sec = iSecNum;
    its.it_interval.tv_nsec = iMSec * 1000;
    its.it_value.tv_sec = iSecNum;  // First expiry after 1 sec
    its.it_value.tv_nsec = iMSec * 1000;

    if (timer_settime(hTimer, 0, &its, NULL) == -1)
    {
        fprintf(stderr, "LeakTracer (timer_trackStartTime): timer_settime failed  to set the timer. " \
                        "Leak measurement will be for entire duration of execution period:%s \n", strerror(errno));
  
        OnStopTimer(hTimer);
        return NULL;
    }    
    return hTimer;    
}