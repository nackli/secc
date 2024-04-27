#if 0
#include <errno.h>    /* for errno */
#include <signal.h>   /* for signal */
#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* for EXIT_FAILURE */
#include <string.h>   /* for strerror */
#include <sys/time.h> /* for setitimer */
#include <time.h>
#include <unistd.h>   /* for _exit pause */

#include <linux/sched.h>
#if 0
#include <linux/workqueue.h>

static struct workqueue_struct *wq = NULL;
static struct work_struct work;
static void work_handler(struct work_struct *data)
{
    printk(KERN_ALERT"work handler function");
}


static int __init test_init(void)
{
        wq = create_singlethread_workqueue("my_workqueue");
        if(!wq)
                goto err;
        INIT_WORK(&work, work_handler);
        queue_work(&work);
        return 0;
    err:
        return -1;
}

static void __exit test_exit(void)
{
        destroy_workqueue(wq);
}


#endif

#define TIMER_IT_VALE_INTERVAL 1000 //ms
typedef struct TIMER_INFO
{
    int iTimerId;
    unsigned long iTimerOut;
    int iIterVal;
    void (*handler)(void *argv);
    void *argv;
}TIMER_INFO,*LP_TIMER_INFO;
static TIMER_INFO g_TimerInfo[2000] = {0};

unsigned long  getCurrentMSec()
{
    struct timeval tm;
    gettimeofday(&tm, NULL);

    return (tm.tv_sec * 1000 + tm.tv_usec / 1000);   
}

static void OnTimerProc(LP_TIMER_INFO lpTimerInfo,int iSize)
{
    if(!lpTimerInfo)
        return;
    unsigned long lGetTickCount =  getCurrentMSec();
    for(int i = 0; i < iSize ;i++)
    {
        if(lpTimerInfo->iTimerId ==0)
            continue;
        if(lpTimerInfo->iTimerOut <= lGetTickCount)
        {
            lpTimerInfo->iTimerOut = lGetTickCount + lpTimerInfo->iIterVal;
            lpTimerInfo->handler(lpTimerInfo->argv);
        }            
        lpTimerInfo ++;
    }
}

void signalHandler(int signo)
{
    switch (signo)
    {
    case SIGALRM:
    case SIGVTALRM:
    case SIGPROF:
        OnTimerProc(g_TimerInfo,sizeof(g_TimerInfo) / sizeof(g_TimerInfo[0]));
        break;
    }
}

bool SetTimer(int iTimerId,int iTimerVale,void (*handler)(void *argv),void *argv )
{ 
    for(int i = 0; i < sizeof(g_TimerInfo) / sizeof(g_TimerInfo[0]);i++)
    {
        if(g_TimerInfo[i].iTimerId == 0)
        {
            struct timeval tm = {0};
            unsigned long lGetTickCount =  getCurrentMSec();
            g_TimerInfo[i].iTimerId = iTimerId;
            g_TimerInfo[i].iTimerOut = iTimerVale  + lGetTickCount;
            g_TimerInfo[i].handler = handler;
            g_TimerInfo[i].argv = argv;
            g_TimerInfo[i].iIterVal = iTimerVale;
            break;
        }
        else
            continue;
    }
    return false;
}

static bool OnInitUserTimer()
{
    static bool bInit = false;
    if(!bInit)
    {
        bInit = true;
        struct itimerval new_value = {0};

        /* 设置信号的处理方式 */
        signal(SIGALRM, signalHandler);
        /* 启动定时器的时间 */
        new_value.it_value.tv_sec = 0;
        new_value.it_value.tv_usec = TIMER_IT_VALE_INTERVAL;
        /* 定时器的间隔时间 */
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = TIMER_IT_VALE_INTERVAL;

        /* 以系统真实的时间来计算，将送出SIGALRM信号 */
        if (setitimer(ITIMER_REAL, &new_value, NULL) < 0)
        {
            /* 函数调用返回错误 */
            printf("Setitimer Failed : %s\n", strerror(errno));
            /* 标准退出 */
            return false;
        }
    }
    return true;
}

static void TestFun(void *argv)
{
    struct timeval tm;
    gettimeofday(&tm, NULL);
    printf("Get the TestFun 00000 = %ld.%03ld\n",
               tm.tv_sec, tm.tv_usec / 1000);
   
}

static void TestFun1(void *argv)
{
    struct timeval tm;
    gettimeofday(&tm, NULL);
    printf("Get the TestFun1 11111 = %ld.%03ld\n",
               tm.tv_sec, tm.tv_usec / 1000);

   // sleep(2);        
}


int main(int argc, char *argv[])
{
    /* 定义设置的时间结构体 */

    OnInitUserTimer();
    SetTimer(1023,5,TestFun,NULL);
    SetTimer(1025,2,TestFun1,NULL);
    /* 进入无限循环 */
    while (1)
    {
        /* 进程主动挂起，等待信号唤醒 */
        pause();
    }

    return 0;
}
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> 
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>   /* for _exit pause */
// #include "TimerManage.h"
// #include "PosixMsg.h"
#include "CMsgManage.h"
#include "SysDeviceCtrl.h"
#include "LogManage.h"
// #include "CSlacManage.h"
// #include "CUdpServer.h"
 #include "CSeccManage.h"

// void sig_handlerTimer1(int);
// void sig_handlerTimer2(SIGVAL_UNION);

// void sig_handlerTimer2(SIGVAL_UNION signum)
// {
//     int *iTemp = (int*) signum.sival_ptr;

//     LOGOUT_ERROR("Caught signal: %d\n",*iTemp);
// }

// void sig_handlerTimer3(SIGVAL_UNION signum)
// {
//     static int iIndex = 0;
//     //int *iTemp = (int*) signum.sival_ptr;
//     //while(1)
//     //printf("aaaaa\r\n");
//     char szBuf[128] = {0};
//     sprintf(szBuf,"Nack test Msg Queue = %f ,%d",OnGetCpLineVoltage(0),iIndex % 2);
//     //if(iIndex % 2 == 1)
//     OnSetLedBrightness(iIndex % 2);
//     SendMsg(szBuf,sizeof(szBuf));
//     // else
//     //     OnPostSysVMsg(*iTemp,&iIndex,1000,sizeof(iIndex));
//     iIndex ++;
// }

int main()
{
    //ExecEchoInt(10, PWM_EXPORT, 0);
    OnInitLogInfo();
    CSeccManage secc;
    secc.InitSeccDevInfo();
    pthread_attr_t attr = {0};
    pthread_attr_setstacksize(&attr, 8192);
    secc.StartThread(&attr);
    system("ipcs -q") ;  
    int iOldCpVol = 0;
    while(1)
    {
        //char szVolNamep[128] = {0};
        int iCpVol = (int)OnGetCpLineVoltage(0);
        uint8_t iCpStatusType = 0;
        if(abs(iOldCpVol - iCpVol) > 2)
        {
            LOGOUT_DEBUG("CP Vol = Cur : %d, Old : %d",iCpVol,iOldCpVol);
            if(iCpVol > 10)
                iCpStatusType = EM_MSG_PLUG_OUT;
            else if(iCpVol > 6 )
                iCpStatusType = EM_MSG_PLUG_IN;
            else if(iCpVol > 3)
                iCpStatusType = EM_MSG_PLUG_CHAGE;
            else if(iOldCpVol != iCpVol)
                iCpStatusType = EM_MSG_PLUS_UNKOWN;
            SendMsgUint8(iCpStatusType);
        }
        iOldCpVol = iCpVol;          
        usleep(500000);
    }      
    secc.UninitSeccDevInfo();
    secc.ExitAndStopThread();
    //OnInitAdcInfo(2);
    //InitMsg(12345);


    // CSlacManage SlacManage;
    // SlacManage.InitSlacInfo();
    // SlacManage.StartThread();
    // SlacManage.OnSendSetKeyReq();
    
    //OnGetStandardVol(0);
    // if(iMsgId < 0)
    // {
    //     //EEXIST
    //     LOGOUT_DEBUG("OnInitMsg = %d,%d",iMsgId,errno);
    //     pause();
    //     return 0;
    // }


   // char szBuf[128] = {0};
    //LOGOUT_DEBUG(szBuf,"Nack test Msg Queue = %d",0);
    //if(iIndex % 2 == 1)
    //LOGOUT_DEBUG("OnInitMsg = %d,%d",iMsgId,errno);

    //SendMsg(szBuf,sizeof(szBuf));
    //OnSetPwmWidth(0, 5);
    //OnSetPwmWidth(0, 100);
    //HANDLE hTimer1 = OnSetTimer(1000, sig_handlerTimer3,NULL);
    // CUdpServer *pUdpServer = NULL;

    // if(!pUdpServer)
    // {
    //     pUdpServer = new CUdpServer;
    //     pUdpServer->InitSeccUdpInfo();
    //     pUdpServer->StartThread();
    // }    
    // while(1)
    // {
    //     char szTest[512] = {0};
    //     //system("ipcs -q") ;
    //     int fRet = ReadMsg(szTest,sizeof(szTest));
    //     if(!strcasecmp("CM_SLAC_MATCH",szTest))
    //     {
    //         SlacManage.ExitAndStopThread();
    //     }
    // }
    // OnSetPwmWidth(0, 100);
    // RemoveMsg();
    // pUdpServer->ExitAndStopThread();
    // pUdpServer->CloseNetWorkServer();
    LOGOUT_DEBUG("proc exit");
    return 0;
}

#endif
