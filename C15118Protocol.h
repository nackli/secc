#pragma once
#ifndef ___15118_PROTOCOL___H__
#define ___15118_PROTOCOL___H__
#include "CThreadBase.h"
#include <stdint.h>
class C15118Protocol : public CThreadBase
{
public:
    C15118Protocol();
    virtual ~C15118Protocol();
    void SetSocketHandle(int iSocketHandle);
private: 
    virtual void ThreadProcFun(); 
    bool ReadEvccData(uint8_t *szDataBuf, uint16_t iBufSize);  
    bool OnTodo15118Info(int iType, iso1EXIDocument *pExiDoc);   
private:
    int m_iSocketHandle;
    int m_iCurStatus;
};
#endif 