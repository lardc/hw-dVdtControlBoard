// ----------------------------------------
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
#include "SaveToFlash.h"
#include "FormatOutputJSON.h"
#include "JSONDescription.h"

// Variables
//
volatile Int64U CONTROL_TimeCounter = 0;
volatile DeviceState CONTROL_State = DS_None;
volatile Boolean CycleActive = FALSE, RequestSaveToFlash = FALSE;
static Int16U cellVoltageCopy = 0, cellVRate_x10Copy = 0;
static Int16U CONTROL_RateRangeArray[MAX_CELLS_COUNT], CONTROL_GateVArray[MAX_CELLS_COUNT];
//
Int16U CONTROL_ExtInfoData[VALUES_EXT_INFO_SIZE] = {0};
Int16U CONTROL_ExtInfoCounter = 0;
//
Int16U CONTROL_CorrectedRate = 0;
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
Boolean CONTROL_ApplySettings(Int16U CellVRate_x10, Int16U CellVoltage);
static void CONTROL_PrepareStart(pInt16U UserError, Int16U VRate_x10, Boolean StartTest, Int16U ActionID);
void CONTROL_InitStoragePointers();

// Functions
//
void CONTROL_Init(Boolean BadClockDetected)
{
	Int16U EPIndexes[EP_COUNT] = { EP16_ExtInfoData };
	Int16U EPSized[EP_COUNT] = { VALUES_EXT_INFO_SIZE };
	pInt16U EPCounters[EP_COUNT] = { &CONTROL_ExtInfoCounter };
	pInt16U EPDatas[EP_COUNT] = { CONTROL_ExtInfoData };

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
	DEVPROFILE_InitEPService(EPIndexes, EPSized, EPCounters, EPDatas);
	// Reset control values
	DEVPROFILE_ResetControlSection();

	CONTROL_InitStoragePointers();
	
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

	if (DataTable[REG_DIAG_ALLOW_WRITE] && RequestSaveToFlash)
	{
		STF_SaveDiagData();
		RequestSaveToFlash = FALSE;
	}
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
	Int32S Voltage = (Int32S)DataTable[REG_DESIRED_VOLTAGE];
	Int32S correctedVoltage = Voltage  * Voltage * (Int16S)DataTable[REG_V_P2] / 1000000 + Voltage * DataTable[REG_V_P1] / 1000 + (Int16S)DataTable[REG_V_P0] + (Int16S)DataTable[REG_CSU_V_OFFSET];

	return (correctedVoltage > 0) ? correctedVoltage : 0;
}
// ----------------------------------------

Int16U CONTROL_CorrectRate(Int16U Voltage, Int16U VRate_x10, Int16U ActionID)
{
	// Подстановка для скорости, заданной через регистр
	if(ActionID == ACT_APPLY_SETTINGS || ActionID == ACT_START_TEST_CUSTOM)
	{
		switch(VRate_x10)
		{
			case 20 * 10:		ActionID = ACT_START_TEST_20;	break;
			case 50 * 10:		ActionID = ACT_START_TEST_50;	break;
			case 100 * 10:		ActionID = ACT_START_TEST_100;	break;
			case 200 * 10:		ActionID = ACT_START_TEST_200;	break;
			case 320 * 10:		ActionID = ACT_START_TEST_320;	break;
			case 500 * 10:		ActionID = ACT_START_TEST_500;	break;
			case 1000 * 10:		ActionID = ACT_START_TEST_1000;	break;
			case 1600 * 10:		ActionID = ACT_START_TEST_1600;	break;
			case 2000 * 10:		ActionID = ACT_START_TEST_2000;	break;
			case 2500 * 10:		ActionID = ACT_START_TEST_2500;	break;
			default:
				break;
		}
	}

	Int16U offset = 0;
	switch (ActionID)
	{
		case ACT_START_TEST_20:		offset = REG_RATE_TUNE_20_P2; break;
		case ACT_START_TEST_50:		offset = REG_RATE_TUNE_50_P2; break;
		case ACT_START_TEST_100:	offset = REG_RATE_TUNE_100_P2; break;
		case ACT_START_TEST_200:	offset = REG_RATE_TUNE_200_P2; break;
		case ACT_START_TEST_320:	offset = REG_RATE_TUNE_320_P2; break;
		case ACT_START_TEST_500:	offset = REG_RATE_TUNE_500_P2; break;
		case ACT_START_TEST_1000:	offset = REG_RATE_TUNE_1000_P2; break;
		case ACT_START_TEST_1600:	offset = REG_RATE_TUNE_1600_P2; break;
		case ACT_START_TEST_2000:	offset = REG_RATE_TUNE_2000_P2; break;
		case ACT_START_TEST_2500:	offset = REG_RATE_TUNE_2500_P2; break;
		default: return VRate_x10;
	}

	Int16S P2 = DataTable[offset];
	Int16S P1 = DataTable[offset + 1];
	Int16S P0 = DataTable[offset + 2];

	Int32S Error = (Int32S)Voltage * Voltage / 1000 * P2 / 10000 + (Int32S)Voltage * P1 / 10000 + P0 / 10;

	Int16U correctedRate = VRate_x10 - VRate_x10 * Error / 100;

	return (correctedRate > 0) ? correctedRate : 0;
}
// ----------------------------------------

Boolean CONTROL_ApplySettings(Int16U CellVRate_x10, Int16U CellVoltage)
{
	if(CellVoltage != cellVoltageCopy || CellVRate_x10 != cellVRate_x10Copy)
	{
		if(CELLMUX_SetCellsState(CellVoltage, CONTROL_RateRangeArray, CONTROL_GateVArray))
		{
			cellVoltageCopy = CellVoltage;
			cellVRate_x10Copy = CellVRate_x10;
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
//	ZbGPIO_SwitchOutRelay(FALSE);
//	ZbGPIO_SwitchLED2(FALSE);
	LOGIC_Reset();

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

	RequestSaveToFlash = TRUE;
}
// ----------------------------------------

static void CONTROL_SwitchToFaultEx()
{
	Int16U errorCodeEx = 0;
	Int16U fault = CELLMUX_GetFaultReason(&errorCodeEx);
	
	CONTROL_SwitchToFault(fault, errorCodeEx);
}
// ----------------------------------------

void CONTROL_InitStoragePointers()
{
	STF_AssignPointer(0, (Int32U)&DataTable[REG_DEV_STATE]);
	STF_AssignPointer(1, (Int32U)&DataTable[REG_FAULT_REASON]);
	STF_AssignPointer(2, (Int32U)&DataTable[REG_DISABLE_REASON]);
	STF_AssignPointer(3, (Int32U)&DataTable[REG_WARNING]);
	STF_AssignPointer(4, (Int32U)&DataTable[REG_PROBLEM]);
	STF_AssignPointer(5, (Int32U)&DataTable[REG_TEST_RESULT]);
	STF_AssignPointer(6, (Int32U)&DataTable[REG_FAULT_REASON_EX]);
	STF_AssignPointer(7, (Int32U)&DataTable[REG_VOLTAGE_OK]);
	STF_AssignPointer(8, (Int32U)&DataTable[REG_VOLTAGE_OK_1]);
	STF_AssignPointer(9, (Int32U)&DataTable[REG_VOLTAGE_OK_2]);
	STF_AssignPointer(10, (Int32U)&DataTable[REG_VOLTAGE_OK_3]);
	STF_AssignPointer(11, (Int32U)&DataTable[REG_VOLTAGE_OK_4]);
	STF_AssignPointer(12, (Int32U)&DataTable[REG_VOLTAGE_OK_5]);
	STF_AssignPointer(13, (Int32U)&DataTable[REG_VOLTAGE_OK_6]);
	STF_AssignPointer(14, (Int32U)&DataTable[REG_ACTUAL_VOLTAGE_1]);
	STF_AssignPointer(15, (Int32U)&DataTable[REG_ACTUAL_VOLTAGE_2]);
	STF_AssignPointer(16, (Int32U)&DataTable[REG_ACTUAL_VOLTAGE_3]);
	STF_AssignPointer(17, (Int32U)&DataTable[REG_ACTUAL_VOLTAGE_4]);
	STF_AssignPointer(18, (Int32U)&DataTable[REG_ACTUAL_VOLTAGE_5]);
	STF_AssignPointer(19, (Int32U)&DataTable[REG_ACTUAL_VOLTAGE_6]);
	STF_AssignPointer(20, (Int32U)&DataTable[REG_CELL_STATE_1]);
	STF_AssignPointer(21, (Int32U)&DataTable[REG_CELL_STATE_2]);
	STF_AssignPointer(22, (Int32U)&DataTable[REG_CELL_STATE_3]);
	STF_AssignPointer(23, (Int32U)&DataTable[REG_CELL_STATE_4]);
	STF_AssignPointer(24, (Int32U)&DataTable[REG_CELL_STATE_5]);
	STF_AssignPointer(25, (Int32U)&DataTable[REG_CELL_STATE_6]);
}
// ----------------------------------------

void CONTROL_InitJSONPointers()
{
	Rate20_Active  = (DataTable[REG_ACTIVE_RATE_MASK] >> 0) & 0x1;
	Rate50_Active  = (DataTable[REG_ACTIVE_RATE_MASK] >> 1) & 0x1;
	Rate200_Active = (DataTable[REG_ACTIVE_RATE_MASK] >> 2) & 0x1;
	Rate320_Active = (DataTable[REG_ACTIVE_RATE_MASK] >> 3) & 0x1;
	Rate500_Active = (DataTable[REG_ACTIVE_RATE_MASK] >> 4) & 0x1;
	Rate1000_Active = (DataTable[REG_ACTIVE_RATE_MASK] >> 5) & 0x1;
	Rate1600_Active = (DataTable[REG_ACTIVE_RATE_MASK] >> 6) & 0x1;
	Rate2000_Active = (DataTable[REG_ACTIVE_RATE_MASK] >> 7) & 0x1;
	Rate2500_Active = (DataTable[REG_ACTIVE_RATE_MASK] >> 8) & 0x1;

	JSON_AssignPointer(0, (pInt16U)&Rate20_Active);
	JSON_AssignPointer(1, (pInt16U)&Rate50_Active);
	JSON_AssignPointer(2, (pInt16U)&Rate200_Active);
	JSON_AssignPointer(3, (pInt16U)&Rate320_Active);
	JSON_AssignPointer(4, (pInt16U)&Rate500_Active);
	JSON_AssignPointer(5, (pInt16U)&Rate1000_Active);
	JSON_AssignPointer(6, (pInt16U)&Rate1600_Active);
	JSON_AssignPointer(7, (pInt16U)&Rate2000_Active);
	JSON_AssignPointer(8, (pInt16U)&Rate2500_Active);
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_ENABLE_POWER:
			if(CONTROL_State == DS_None)
			{
				cellVoltageCopy = cellVRate_x10Copy = 0;

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
				CONTROL_PrepareStart(UserError, DataTable[REG_VOLTAGE_RATE], FALSE, ACT_APPLY_SETTINGS);
			break;

		case ACT_ENABLE_EXT_SYNC_START:
			if(CONTROL_State == DS_Ready)
				CONTROL_EnableExternalSync(TRUE);
			break;

		case ACT_DISABLE_EXT_SYNC_START:
			CONTROL_EnableExternalSync(FALSE);
			break;

		case ACT_START_TEST_CUSTOM:
			CONTROL_PrepareStart(UserError, DataTable[REG_VOLTAGE_RATE], TRUE, ACT_START_TEST_CUSTOM);
			break;

		case ACT_START_TEST_500:
			CONTROL_PrepareStart(UserError, 500 * 10, TRUE, ACT_START_TEST_500);
			break;

		case ACT_START_TEST_1000:
			CONTROL_PrepareStart(UserError, 1000 * 10, TRUE, ACT_START_TEST_1000);
			break;

		case ACT_START_TEST_1600:
			CONTROL_PrepareStart(UserError, 1600 * 10, TRUE, ACT_START_TEST_1600);
			break;

		case ACT_START_TEST_2000:
			CONTROL_PrepareStart(UserError, 2000 * 10, TRUE, ACT_START_TEST_2000);
			break;

		case ACT_START_TEST_2500:
			CONTROL_PrepareStart(UserError, 2500 * 10, TRUE, ACT_START_TEST_2500);
			break;

		case ACT_START_TEST_200:
			CONTROL_PrepareStart(UserError, 200 * 10, TRUE, ACT_START_TEST_200);
			break;

		case ACT_START_TEST_320:
			CONTROL_PrepareStart(UserError, 320 * 10, TRUE, ACT_START_TEST_320);
			break;

		case ACT_START_TEST_20:
			CONTROL_PrepareStart(UserError, 20 * 10, TRUE, ACT_START_TEST_20);
			break;

		case ACT_START_TEST_50:
			CONTROL_PrepareStart(UserError, 50 * 10, TRUE, ACT_START_TEST_50);
			break;

		case ACT_START_TEST_100:
			CONTROL_PrepareStart(UserError, 100 * 10, TRUE, ACT_START_TEST_100);
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

void CONTROL_PrepareStart(pInt16U UserError, Int16U VRate_x10, Boolean StartTest, Int16U ActionID)
{
	if(CONTROL_State == DS_Ready)
	{
		Int16U cellCount = CELLMUX_CellCount();
		Int16U cellVoltage = CONTROL_CorrectVoltage() / cellCount;
		CONTROL_CorrectedRate = CONTROL_CorrectRate(cellVoltage * cellCount, VRate_x10, ActionID) / cellCount;

		// Проверка уставки по напряжению и скорости нарастания
		if(DataTable[REG_CELL_MIN_VOLTAGE] <= cellVoltage && cellVoltage <= DataTable[REG_CELL_MAX_VOLTAGE] &&
				SP_GetSetpointArray(CONTROL_CorrectedRate, CONTROL_RateRangeArray, CONTROL_GateVArray))
		{
			CONTROL_FillWPPartDefault();

			// Применение настроеек к ячейкам
			if(CONTROL_ApplySettings(CONTROL_CorrectedRate, cellVoltage))
			{
				CONTROL_HandleFanLogic(StartTest);
				CONTROL_HandleExtLed(StartTest);
				ZbGPIO_SwitchLED2(StartTest);

				// Реле включается только при явном запросе
//				if(StartTest)
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
