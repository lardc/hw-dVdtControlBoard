// Header
#include "ZbFan.h"
//
#include "BoardConfig.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"

// Definitions
//
#define FAN_SILENT_DUTY		10

// Functions
//
void ZbFan_Init()
{
	ZwPWM4_Init(PWMUp, CPU_FRQ, ZW_PWM_FREQUENCY, TRUE, FALSE, 0, 0, TRUE, TRUE, TRUE, FALSE, FALSE);
	ZbFan_SetSilent(TRUE);
	ZwPWM_Enable(TRUE);
}
// ----------------------------------------

void ZbFan_SetSilent(Boolean Enable)
{
	if (Enable)
		ZwPWM4_SetValueA(((ZW_PWM_DUTY_BASE >> 1) * FAN_SILENT_DUTY) / 100);
	else
		ZwPWM4_SetValueA(((ZW_PWM_DUTY_BASE >> 1) * DataTable[REG_COOLER_MAX_SPEED]) / 100);
}
// ----------------------------------------

// No more.
