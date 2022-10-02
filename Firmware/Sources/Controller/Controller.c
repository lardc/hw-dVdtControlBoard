﻿// ----------------------------------------
// Controller logic
// ----------------------------------------

// Header
#include "Controller.h"
//
// Includes
#include "SysConfig.h"
//
#include "ZbBoard.h"
#include "ZbGPIO.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "SCCISlave.h"
#include "DeviceProfile.h"
#include "Logic.h"
#include "CellMux.h"
#include "Diagnostics.h"
#include "Constraints.h"
#include "Setpoint.h"

// Variables
//
volatile Int64U CONTROL_TimeCounter = 0;
volatile DeviceState CONTROL_State = DS_None;
static volatile Boolean CycleActive = FALSE;
static Int16U cellVoltageCopy = 0, cellVRateCopy = 0;
static Int16U CONTROL_RateRangeArray[MAX_CELLS_COUNT], CONTROL_GateVArray[MAX_CELLS_COUNT];
//
// Boot-loader flag
#pragma DATA_SECTION(CONTROL_BootLoaderRequest, "bl_flag");
volatile Int16U CONTROL_BootLoaderRequest = 0;

// Forward functions
//
static void CONTROL_SetDeviceState(DeviceState NewState);
static void CONTROL_FillWPPartDefault();
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError);
static void CONTROL_SwitchToFault(Int16U FaultReason, Int16U ErrorCodeEx);
static void CONTROL_SwitchToFaultEx();
static Boolean CONTROL_ApplySettings(Int16U VRate);
static void CONTROL_PrepareStart(pInt16U UserError, Int16U Rate_x10, Boolean StartTest);

// Functions
//
void CONTROL_Init(Boolean BadClockDetected)
{
	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = {&ZbMemory_WriteValuesEPROM, &ZbMemory_ReadValuesEPROM};
	
	// Init data table
	DT_Init(EPROMService, BadClockDetected);
	DT_SaveFirmwareInfo(DEVICE_CAN_ADDRESS, 0);

	// Fill state variables with default values
	CONTROL_SetDeviceState(DS_None);
	CONTROL_FillWPPartDefault();
	
	// Device profile initialization
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	// Reset control values
	DEVPROFILE_ResetControlSection();
	
	if(!BadClockDetected)
	{
		if(ZwSystem_GetDogAlarmFlag())
		{
			DataTable[REG_WARNING] = WARNING_WATCHDOG_RESET;
			ZwSystem_ClearDogAlarmFlag();
		}
	}
	else
	{
		DataTable[REG_DISABLE_REASON] = DISABLE_BAD_CLOCK;
		ZbGPIO_SwitchLED2(TRUE);
		CONTROL_SetDeviceState(DS_Disabled);
	}
}
// ----------------------------------------

void CONTROL_DelayedInit()
{
	CELLMUX_Init();
}
// ----------------------------------------

void CONTROL_Idle()
{
	DEVPROFILE_ProcessRequests();
	DEVPROFILE_UpdateCANDiagStatus();
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(CONTROL_Update, "ramfuncs");
#endif
void CONTROL_Update()
{
	if(CONTROL_State == DS_Ready || CONTROL_State == DS_InProcess)
	{
		// Update cell states
		if(!CELLMUX_ReadStates())
			CONTROL_SwitchToFaultEx();
		
		// Update measurement state
		if(CycleActive)
			LOGIC_Update(CONTROL_TimeCounter);
	}
}
// ----------------------------------------

Int16U CONTROL_CorrectVoltage()
{
	Int32S Voltage = (Int32S)DataTable[REG_DESIRED_VOLTAGE] * DataTable[REG_V_FINE_N] / DataTable[REG_V_FINE_D] +
			(Int16S)DataTable[REG_V_OFFSET] + (Int16S)DataTable[REG_CSU_V_OFFSET];

	return (Voltage > 0) ? Voltage : 0;
}
// ----------------------------------------

Int16U CONTROL_СorrectRate(Int16U VRate)
{
	return (Int32S)VRate * DataTable[REG_RATE_GLOBAL_K_N] / DataTable[REG_RATE_GLOBAL_K_D];
}
// ----------------------------------------

Boolean CONTROL_ApplySettings(Int16U VRate)
{
	Int16U cellCount = CELLMUX_CellCount();
	Int16U cellVoltage = CONTROL_CorrectVoltage() / cellCount;
	Int16U cellVRate = CONTROL_СorrectRate(VRate) / cellCount;
	
	if(cellVoltage != cellVoltageCopy || cellVRate != cellVRateCopy)
	{
		if(CELLMUX_SetCellsState(cellVoltage, cellVRate, CONTROL_RateRangeArray, CONTROL_GateVArray))
		{
			cellVoltageCopy = cellVoltage;
			cellVRateCopy = cellVRate;
		}
		else
			return FALSE;
	}
	
	return TRUE;
}
// ----------------------------------------

void CONTROL_EnableExternalSync(Boolean Enable)
{
	DataTable[REG_TEST_RESULT] = OPRESULT_NONE;
	ZwPWM_EnableTZInterruptsGlobal(FALSE);

	// Prepare timer
	ZwTimer_StopT1();
	ZwTimer_ReloadT1();

	// FAN Logic
	CONTROL_HandleFanLogic(TRUE);

	// Configure pins
	ZbGPIO_SwitchSyncEn(Enable);
	ZbGPIO_SwitchLED2(Enable);
	ZbGPIO_SwitchOutRelay(Enable);

	// Clear registers
	ZwPWM6_ClearTZ();
	ZwPWM6_ProcessTZInterrupt();

	ZwPWM_EnableTZInterruptsGlobal(Enable);
}
// ----------------------------------------

void CONTROL_ExtSyncEvent()
{
	CONTROL_SetDeviceState(DS_InProcess);
	CONTROL_HandleExtLed(TRUE);
	ZbGPIO_SwitchResultOut(TRUE);
	ZwTimer_StartT1();
}
// ----------------------------------------

void CONTROL_ExtSyncFinish()
{
	ZwTimer_StopT1();
	Boolean PinState = ZbGPIO_ReadDetectorPin();

	ZbGPIO_SwitchResultOut(FALSE);
	ZbGPIO_SwitchSyncEn(FALSE);
	ZbGPIO_SwitchOutRelay(FALSE);
	ZbGPIO_SwitchLED2(FALSE);

	CONTROL_NotifyEndTest(PinState, FAULT_NONE, WARNING_NONE);
}
// ----------------------------------------

void CONTROL_NotifyEndTest(Boolean Result, Int16U FaultReason, Int16U Warning)
{
	DataTable[REG_TEST_RESULT] = Result ? OPRESULT_OK : OPRESULT_FAIL;
	DataTable[REG_WARNING] = Warning;
	
	if(FaultReason != FAULT_NONE)
		CONTROL_SwitchToFault(FaultReason, 0);
	else
		CONTROL_SetDeviceState(DS_Ready);
	
	CycleActive = FALSE;
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
#pragma CODE_SECTION(CONTROL_NotifyCANFault, "ramfuncs");
#endif
void CONTROL_NotifyCANFault(ZwCAN_SysFlags Flag)
{
	DEVPROFILE_NotifyCANFault(Flag);
}
// ----------------------------------------

static void CONTROL_SetDeviceState(DeviceState NewState)
{
	// Set new state
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;
}
// ----------------------------------------

static void CONTROL_FillWPPartDefault()
{
	DataTable[REG_FAULT_REASON] = FAULT_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_TEST_RESULT] = OPRESULT_NONE;
	
	Int16U i;
	for(i = REG_VOLTAGE_OK; i <= REG_CELL_STATE_6; ++i)
		DataTable[i] = 0;
}
// ----------------------------------------

static void CONTROL_SwitchToFault(Int16U FaultReason, Int16U ErrorCodeEx)
{
	LOGIC_Reset();
	CONTROL_SetDeviceState(DS_Fault);
	DataTable[REG_FAULT_REASON] = FaultReason;
	DataTable[REG_FAULT_REASON_EX] = ErrorCodeEx;
}
// ----------------------------------------

static void CONTROL_SwitchToFaultEx()
{
	Int16U errorCodeEx = 0;
	Int16U fault = CELLMUX_GetFaultReason(&errorCodeEx);
	
	CONTROL_SwitchToFault(fault, errorCodeEx);
}
// ----------------------------------------

static Boolean CONTROL_ValidateVoltage()
{
	Int16U CellVoltage = CONTROL_CorrectVoltage() / CELLMUX_CellCount();
	return (DataTable[REG_CELL_MIN_VOLTAGE] <= CellVoltage && CellVoltage <= DataTable[REG_CELL_MAX_VOLTAGE]);
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_ENABLE_POWER:
			if(CONTROL_State == DS_None)
			{
				cellVoltageCopy = cellVRateCopy = 0;

				if(!CELLMUX_SetCellPowerState(TRUE))
					CONTROL_SwitchToFaultEx();
				else
					CONTROL_SetDeviceState(DS_Ready);
			}
			else if(CONTROL_State != DS_Ready)
				*UserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_DISABLE_POWER:
			if(CONTROL_State == DS_Ready || CONTROL_State == DS_InProcess)
			{
				if(!CELLMUX_SetCellPowerState(FALSE))
					CONTROL_SwitchToFaultEx();
				else
				{
					CONTROL_SetDeviceState(DS_None);
					CONTROL_FillWPPartDefault();
				}
			}
			break;

		case ACT_APPLY_SETTINGS:
			if(CONTROL_State == DS_Ready)
				CONTROL_PrepareStart(UserError, DataTable[REG_VOLTAGE_RATE], FALSE);
			break;

		case ACT_ENABLE_EXT_SYNC_START:
			if(CONTROL_State == DS_Ready)
				CONTROL_EnableExternalSync(TRUE);
			break;

		case ACT_DISABLE_EXT_SYNC_START:
			CONTROL_EnableExternalSync(FALSE);
			break;

		case ACT_START_TEST_CUSTOM:
			CONTROL_PrepareStart(UserError, DataTable[REG_VOLTAGE_RATE], TRUE);
			break;

		case ACT_START_TEST_500:
			CONTROL_PrepareStart(UserError, 500 * 10, TRUE);
			break;

		case ACT_START_TEST_1000:
			CONTROL_PrepareStart(UserError, 1000 * 10, TRUE);
			break;

		case ACT_START_TEST_1600:
			CONTROL_PrepareStart(UserError, 1600 * 10, TRUE);
			break;

		case ACT_START_TEST_2000:
			CONTROL_PrepareStart(UserError, 2000 * 10, TRUE);
			break;

		case ACT_START_TEST_2500:
			CONTROL_PrepareStart(UserError, 2500 * 10, TRUE);
			break;

		case ACT_CLR_FAULT:
			if(CONTROL_State == DS_Fault)
			{
				CONTROL_SetDeviceState(DS_None);
				CONTROL_FillWPPartDefault();
			}
			break;

		case ACT_CLR_WARNING:
			DataTable[REG_WARNING] = WARNING_NONE;
			break;

		case ACT_STOP:
			if(CONTROL_State == DS_InProcess)
			{
				LOGIC_Reset();
				ZbGPIO_SwitchStartPulse(FALSE);
				CONTROL_SetDeviceState(DS_None);
			}
			break;

		default:
			return DIAG_DispatchCommand(ActionID);
	}
	
	return TRUE;
}
// ----------------------------------------

void CONTROL_PrepareStart(pInt16U UserError, Int16U Rate_x10, Boolean StartTest)
{
	if(CONTROL_State == DS_Ready)
	{
		// Проверка уставки по напряжению и скорости нарастания
		if(CONTROL_ValidateVoltage() && SP_GetSetpointArray(Rate_x10, CONTROL_RateRangeArray, CONTROL_GateVArray))
		{
			CONTROL_FillWPPartDefault();

			// Применение настроеек к ячейкам
			if(CONTROL_ApplySettings(Rate_x10))
			{
				CONTROL_HandleFanLogic(StartTest);
				CONTROL_HandleExtLed(StartTest);
				ZbGPIO_SwitchLED2(StartTest);

				// Реле включается только при явном запросе
				if(StartTest)
					ZbGPIO_SwitchOutRelay(TRUE);

				if(StartTest)
					LOGIC_BeginTest(CONTROL_TimeCounter);
				else
					LOGIC_ApplyParameters(CONTROL_TimeCounter);

				CONTROL_SetDeviceState(DS_InProcess);
				CycleActive = TRUE;
			}
			else
			{
				CONTROL_SwitchToFaultEx();
				return;
			}
		}
		else
			*UserError = ERR_OUT_OF_RANGE;
	}
	else
		*UserError = ERR_OPERATION_BLOCKED;
}
// ----------------------------------------

void CONTROL_HandleFanLogic(Boolean IsImpulse)
{
	static Int32U IncrementCounter = 0;
	static Int64U FanOnTimeout = 0;
	
	// Idle counter increment
	if(!IsImpulse)
		IncrementCounter++;
	
	// Fan turn on
	if((IncrementCounter > ((Int32U)DataTable[REG_FAN_OPERATE_PERIOD] * 1000)) || IsImpulse)
	{
		IncrementCounter = 0;
		FanOnTimeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_FAN_OPERATE_MIN_TIME] * 1000;
		ZbGPIO_SwitchFAN(TRUE);
	}
	
	// Fan turn off
	if(FanOnTimeout && (CONTROL_TimeCounter > FanOnTimeout))
	{
		FanOnTimeout = 0;
		ZbGPIO_SwitchFAN(FALSE);
	}
}
// ----------------------------------------

void CONTROL_HandleExtLed(Boolean IsImpulse)
{
	static Int64U ExtLedTimeout = 0;

	if(IsImpulse)
	{
		ZbGPIO_SwitchExtLed(TRUE);
		ExtLedTimeout = CONTROL_TimeCounter + EXT_LED_SWITCH_ON_TIME;
	}
	else
	{
		if(CONTROL_TimeCounter >= ExtLedTimeout)
			ZbGPIO_SwitchExtLed(FALSE);
	}
}
// ----------------------------------------
