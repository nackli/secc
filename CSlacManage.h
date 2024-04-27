#ifndef __PLC_SLAC_MANAGE__
#define __PLC_SLAC_MANAGE__
#include "CThreadBase.h"
struct channel;
struct session;
class CSlacManage: public CThreadBase
{
public:
    CSlacManage();
    virtual ~CSlacManage();
    void InitSlacInfo();
    void OnSendSetKeyReq();
    void UnInitSlacInfo();
private: 
    virtual void ThreadProcFun();   
private:
    struct channel *m_pPlcChannel;
    struct session *m_pPlcSession;  
    unsigned char *m_pAAgData; 
};
#endif