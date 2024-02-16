﻿// ----------------------------------------
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
extern volatile Boolean CycleActive;
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
// Rate calculator
Int16U CONTROL_СorrectRate(Int16U cellVoltage, Int16U VRate);
// Enable external sync
void CONTROL_EnableExternalSync(Boolean Enable);
// Start test from external sync
void CONTROL_ExtSyncEvent();
// Off external sync
void CONTROL_ExtSyncFinish();
// Out voltage calculator
Int16U CONTROL_CorrectVoltage();
// Ext LED
void CONTROL_HandleExtLed(Boolean IsImpulse);

#endif // __CONTROLLER_H
