#ifndef ____TIMER_MANAGE__
#define ____TIMER_MANAGE__
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
typedef union sigval SIGVAL_UNION;
typedef void (*SIGNAL_FUN) (SIGVAL_UNION);
typedef void *	HANDLE;
bool  OnStopTimer(HANDLE );
HANDLE OnSetTimer(unsigned long uMSec,SIGNAL_FUN fun,void *pParam);
#endif

