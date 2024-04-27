#ifndef __POSIX_MSG__
#define __POSIX_MSG__
typedef void (*PROCMSG) (int,void *pParam);
int OnInitPosixMsg(const char *szMsgName,PROCMSG callBackFun,int iMsgSize);
bool OnSendPosixMsg(int iMsgId,const char *szData,int iSize);
int ReadPosixMsg(int iMsgId,char *szData,unsigned int iMaxSize);
bool OnClosePosixMsg(int iMsgId);
bool OnDelPosixMsg(const char *szMsgName);
#endif

