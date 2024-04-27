#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "SysDeviceCtrl.h"
#include "LogManage.h"
#include <math.h>
/********************************************************************************************************************************/
#define _ADC_RESOLUTION_    4095
#define VOLT_OFFSET         2980.0 
#define MAX_GUN_NUM         2
#define DIV                 (VOLT_OFFSET / 24.0)
#define CLOSE_FILE(x) if((x)) {fclose((x));(x) = NULL;}

/********************************************************************************************************************************/
typedef enum {
    kHWVersionUnkown = 0,           ///< 欧标
    kHWVersionEU = 1,           ///< 欧标
    kHWVersionSAE = 2,          ///< 美标
} HW_VERSION_T;

static int g_iVoltageStandar = 0;
static HW_VERSION_T g_emVerSionNum = kHWVersionUnkown;
/********************************************************************************************************************************/
static void ExecEchoChar(const char *szData, const char *szKey)
{
    if(!szData)
        return;
    char szCmdBuf[256] = {0};
    sprintf(szCmdBuf,"@echo %s > %s", szData, szKey);
    //LOG_PRINTF("%s\r\n",szCmdBuf);
    system(szCmdBuf); 
}

static void ExecEchoInt(int iValue,const char *szKey)
{
    if(!szKey)
        return;
    char szCmdBuf[256] = {0};
    sprintf(szCmdBuf,"echo %d > %s", iValue,szKey);
    //LOG_PRINTF("%s\r\n",szCmdBuf);
    system(szCmdBuf);
}

static void OnInitPWMInfo(int iMaxGunNum)
{

    for(int i = 0; i < iMaxGunNum; i++)
    {
        char szCmdBuf[128] = {0};
        sprintf(szCmdBuf,PWM_EXPORT,  i);  
        ExecEchoInt(0, szCmdBuf);

        memset(szCmdBuf, 0, sizeof(szCmdBuf));
        sprintf(szCmdBuf,PWM_PERIOD,  i);  
        ExecEchoInt(1000000, szCmdBuf); 

        OnSetPwmWidth(100, i);

        memset(szCmdBuf, 0, sizeof(szCmdBuf));
        sprintf(szCmdBuf,PWM_ENABLE,  i);  
        ExecEchoInt(1, szCmdBuf);            
    }
}

static bool OnGetFileLineData(const char *szDevName, char *szData,size_t iMaxSize)
{
    bool fRet = false;
    if(!szDevName || !szData)
        return fRet;
    FILE *fr = fopen(szDevName,"r");
    if(fr)
    {
        char *pReadBuf = NULL;
        size_t iReadSize = 0;
        if(getline (&pReadBuf, &iReadSize, fr) > 0)
        {
            size_t iCpySize = iMaxSize > iReadSize ? iReadSize:iMaxSize;
            memcpy(szData, pReadBuf, iCpySize);
            free(pReadBuf);
            pReadBuf = NULL;
            fRet = true;
        }
        CLOSE_FILE(fr); 
    }
    return fRet;
}

static void OnGetStandardVol(int iGunId)
{
    if(iGunId < 0)
        return;
     const char *pDevName = ADC_DEV_NAME1;
    if(iGunId == 1)
        pDevName = ADC_DEV_NAME2;
    char szValue[32]  = {0};
    /*控制PWM输出占空比为0，这样可以得到-12V时ADC采样值*/
    OnSetPwmWidth(iGunId, 0);
    usleep(50000);  
    if(OnGetFileLineData(pDevName, szValue,sizeof(szValue)))
    {
        /*读取-12V ADC实际值*/
        int iLowVol = atoi((char*)szValue);
        /*计算12V ADC理论值，VOLT_OFFSET为固定值*/      
        int iHightVol = iLowVol - VOLT_OFFSET;
        /*计算0V ADC理论值*/
        int ZeroVol = VOLT_OFFSET / 2 + iHightVol;  

        /*如果偏差大于10进行校准*/
        int iDiffValue = ZeroVol - g_iVoltageStandar;

        if (abs(iDiffValue) > 10)   /* 未校准 */
        {
            g_iVoltageStandar = ZeroVol;
            LOGOUT_INFO("update s_voltage_standard = %d", g_iVoltageStandar);
        }          
    }
}


static void OnInitAdcInfo()
{
    ExecEchoInt(5, ADC_EXPORT);
    ExecEchoChar("in", ADC_DIRECTION);
}

static void OnGetDevType()
{
    char szValue[32]  = {0};
    if(OnGetFileLineData(SYS_GPIO_VALUE, szValue,sizeof(szValue)))
    {
        if(atoi(szValue) == 1)
            g_emVerSionNum = kHWVersionSAE;
        else
            g_emVerSionNum = kHWVersionEU;
    }
}

/********************************************************************************************************************************/

bool OnSetLedBrightness(bool bStatus)
{
    int iBrightnessValue = 0;
    if(bStatus)
        iBrightnessValue = 255;
    ExecEchoInt(iBrightnessValue, LED_BRIGHTNESS);
    return true;
}

bool OnSetPwmWidth(int iGunId, int iPercent)
{
    bool fRet = false;
    if(iPercent < 0 || iPercent > 100)
        return false;
    char szCmdBuf[128] = {0};
    sprintf(szCmdBuf, PWM_DUTY_CYCLE,  iGunId);  
    ExecEchoInt(iPercent * 10000, szCmdBuf);
    fRet = true;
    return fRet;
}

void OnInitDevInfo()
{
    OnInitPWMInfo(2);
    OnGetDevType();
    OnInitAdcInfo();
    OnGetStandardVol(0);
}

float OnGetCpLineVoltage(int iGunId)
{
    float fVoltage = 0.0;
    if(iGunId < 0)
        return fVoltage;
    const char *pDevName = ADC_DEV_NAME1;
    if(iGunId == 1)
        pDevName = ADC_DEV_NAME2;
    char szValue[32]  = {0};
    if(OnGetFileLineData(pDevName, szValue,sizeof(szValue)))
    {
        fVoltage = atof(szValue);
        if (g_emVerSionNum == kHWVersionSAE) 
            fVoltage = (g_iVoltageStandar - fVoltage) / DIV;
        else
            fVoltage =  12.623 * fVoltage / _ADC_RESOLUTION_ + 0.425 ; 
    }
    return fVoltage;
}

void HardwareResetPlc()
{
    ExecEchoInt(131,ADC_EXPORT);
    ExecEchoChar("out",PLC_RESET_DIRECTION);
    system("/usr/local/bin/GodHand -a 0x20E02f0 -w -v 1b0b1");  
    ExecEchoInt(0,PLC_RESET_GPIO);
    usleep(500000);
    ExecEchoInt(1,PLC_RESET_GPIO);
}