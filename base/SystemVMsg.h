#ifndef __SYSTEM_V_MSG__
#define __SYSTEM_V_MSG__
int OnInitSysVMsg(int iKeyId);
bool OnSndSysVMsg(int iMsgId, void *pParam,unsigned int iSize);
bool OnPostSysVMsg(int iMsgId, void *pParam,unsigned int iSize);
int ReadSysVMsg(int iMsgId, void *pParam, unsigned int iSize);
bool OnRemoveSysVMsg(int iMsgId);
#endif