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
// Notify that CAN system fault occurs
void CONTROL_NotifyCANFault(ZwCAN_SysFlags Flag);
// Notify end of test
void CONTROL_NotifyEndTest(Boolean Result, Int16U FaultReason, Int16U Warning);
void CONTROL_HandleFanLogic(Boolean IsImpulse);
// External sync event
void CONTROL_ExtSyncEvent();
void CONTROL_ExtSyncFinish();

#endif // __CONTROLLER_H
