// -----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
#include "stdinc.h"
//
#include "ZwDSP.h"
#include "Global.h"

// Constants
typedef enum __DeviceState
{
	DS_None = 0,
	DS_Fault = 1,
	DS_Disabled = 2,
	DS_Ready = 3,
	DS_InProcess = 4
} DeviceState;

// Variables
//
extern volatile Int64U CONTROL_TimeCounter;
extern volatile DeviceState CONTROL_State;
//
extern Int16U CONTROL_Values_1[VALUES_x_SIZE];
extern volatile Int16U CONTROL_Values_1_Counter;
extern volatile Int16U CONTROL_BootLoaderRequest;

// Functions
//
// Initialize controller
void CONTROL_Init(Boolean BadClockDetected);
// Initialize controller (delayed operations)
void CONTROL_DelayedInit();
// Update low-priority states
void CONTROL_Idle();
// Main control cycle
void CONTROL_Update();
// Enable work mode single cell or all cell
Boolean CONTROL_EnableSingleCellMode();
// Notify that CAN system fault occurs
void CONTROL_NotifyCANFault(ZwCAN_SysFlags Flag);
// Notify end of test
void CONTROL_NotifyEndTest(Boolean Result, Int16U FaultReason, Int16U Warning);
void CONTROL_HandleFanLogic(Boolean IsImpulse);
// Rate calculator
Int16U CONTROL_СalculationRateXMode(Int16U MaxRate, Int16U MinRate, Int16U VRate, Int16U RegCorrByRate, Int16U RegCorrRateVpoint, Int16U RegCorrRateByVoltage,
                              Int16U RegCorrRange1, Int16U RegCorrRange2, Int16U RegOffsetRange2, Int16U RegRateGlobalKN, Int16U RegRateGlobalKD, Int16U OffsetByVoltage, Boolean EnableTuneLow);
// Single mode rate calculator
Int16U CONTROL_СalculationRateSingleMode(Int16U VRate);
// Full mode rate calculator
Int16U CONTROL_СalculationRateFullMode(Int16U VRate);
// Enable external sync
void CONTROL_EnableExternalSync(Boolean Enable);
// Start test from external sync
void CONTROL_ExtSyncEvent();
// Off external sync
void CONTROL_ExtSyncFinish();
// Out voltage calculator
Int16U CONTROL_CalOutVoltage();
//Ext LED
void CONTROL_HandleExtLed(Boolean IsImpulse);

#endif // __CONTROLLER_H
