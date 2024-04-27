#ifndef __SYS_DEVICE_CTRL__
#define __SYS_DEVICE_CTRL__
#include "SysCtrlDefine.h"
bool OnSetPwmWidth(int iGunId, int iPercent);
bool OnSetLedBrightness(bool bStatus);
float OnGetCpLineVoltage(int iGunId);
void OnInitDevInfo();
void HardwareResetPlc();
#endif