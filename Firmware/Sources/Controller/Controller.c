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
//
#pragma DATA_SECTION(CONTROL_Values_1, "data_mem");
Int16U CONTROL_Values_1[VALUES_x_SIZE];
volatile Int16U CONTROL_Values_1_Counter = 0;
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
static Boolean CONTROL_ApplySettings(Int16U VRate, Boolean PerfomRateCorrection);
static void CONTROL_StartTest(Int16U VRate, Boolean PerfomRateCorrection);
void CONTROL_PrepareStart(pInt16U UserError, Int16U Rate_x10, Boolean UseCustomSettings);

// Functions
//
void CONTROL_Init(Boolean BadClockDetected)
{
	// Variables for endpoint configuration
	Int16U EPIndexes[EP_COUNT] = {EP16_Data_V};
	Int16U EPSized[EP_COUNT] = {VALUES_x_SIZE};
	pInt16U EPCounters[EP_COUNT] = {(pInt16U)&CONTROL_Values_1_Counter};
	pInt16U EPDatas[EP_COUNT] = {CONTROL_Values_1};
	// Data-table EPROM service configuration
	EPROMServiceConfig EPROMService = {&ZbMemory_WriteValuesEPROM, &ZbMemory_ReadValuesEPROM};
	
	// Init data table
	DT_Init(EPROMService, BadClockDetected);
	DT_SaveFirmwareInfo(DEVICE_CAN_ADDRESS, 0);
	// Fill state variables with default values
	CONTROL_FillWPPartDefault();
	
	// Device profile initialization
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitEPService(EPIndexes, EPSized, EPCounters, EPDatas);
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
	// Init interface to cells
	CELLMUX_Init();
}
// ----------------------------------------

void CONTROL_Idle()
{
	// Process external interface requests
	DEVPROFILE_ProcessRequests();
	
	// Update CAN bus status
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

Boolean CONTROL_ApplySettings(Int16U VRate, Boolean PerfomRateCorrection)
{
	Int16U cellVoltage, cellVRate, KRate;
	
	/*
	 * Описание принципа корректировки скорости нарастания:
	 * 1. Наилучшее отношение ожидаемой и реальной скорости нарастания получается на максимальном напряжении (4500В)
	 * и на средней скорости нарастания (1600В/мкс);
	 * 2. При сохранении значения напряжения и увеличении скорости нарастания возрастает ошибка скорости нарастания.
	 * Максимальная ошибка ~20-30% на максимальной скорости нарастания (2500В/мкс) (или на минимальной - в зависимости от схемы ячеек);
	 * 3. При сохранении значения скорости нарастания и уменьшении напряжения возрастает ошибка скорости нарастания.
	 * Максимальная ошибка ~20-30% на минимальном напряжении (500В), причём эффект наблюдается на напряжениях ниже 1000-1500В;
	 * 4. Т.о. требуется одновременно вносить две корректировки: по напряжению и по скорости нарастания.
	 */

	// Perfom rate correction
	if(PerfomRateCorrection)
	{
		if(DataTable[REG_UNIT_USE_RANGE1] && (VRate < SP_GetRange1MaxRate()))
		{
			VRate = (Int32U)VRate * DataTable[REG_CORR_RANGE1] / 1000;
		}
		else if(DataTable[REG_UNIT_USE_RANGE2] && (VRate < SP_GetRange2MaxRate()))
		{
			VRate = (Int32U)VRate * DataTable[REG_CORR_RANGE2] / 1000;
		}
		else
		{
			// by rate (and full range voltage)
			if(CORR_RATE_BY_FS_VOLTAGE)
			{
				KRate = ((Int32U)DataTable[REG_CORR_RATE_BY_RATE] * (DESIRED_VOLTAGE_MAX - DataTable[REG_DESIRED_VOLTAGE]))
						/ (DESIRED_VOLTAGE_MAX - DESIRED_VOLTAGE_MIN);
			}
			else
				KRate = DataTable[REG_CORR_RATE_BY_RATE];

			// by rate
			Int16U VRateMax = DataTable[REG_UNIT_RATE_MAX];
			Int16U VRateMin = DataTable[REG_UNIT_RATE_MIN];
			if(CORR_RATE_TUNE_LOW)
				VRate = ((100 + (((Int32U)(VRateMax - VRate) * KRate) / (VRateMax - VRateMin))) * VRate) / 100;
			else
				VRate = ((100 + (((Int32U)(VRate - VRateMin) * KRate) / (VRateMax - VRateMin))) * VRate) / 100;

			// by voltage (lower zone)
			if(DataTable[REG_CORR_RATE_VPOINT] > DataTable[REG_DESIRED_VOLTAGE])
			{
				VRate = (((((Int32U)(DataTable[REG_CORR_RATE_VPOINT] - DataTable[REG_DESIRED_VOLTAGE])
						* DataTable[REG_CORR_RATE_BY_VOLTAGE]) / (DataTable[REG_CORR_RATE_VPOINT] - DESIRED_VOLTAGE_MIN))
						+ 100) * VRate) / 100;
			}

			// global correction
			VRate = (Int32U)VRate * DataTable[REG_RATE_GLOBAL_K_N] / DataTable[REG_RATE_GLOBAL_K_D];
		}
	}
	
	// Check if settings differ
	cellVoltage = ((Int32U)DataTable[REG_DESIRED_VOLTAGE] * DataTable[REG_V_FINE_N] / DataTable[REG_V_FINE_D]
			+ (Int16S)DataTable[REG_V_OFFSET]) / CELLMUX_CellCount();
	cellVRate = VRate / CELLMUX_CellCount();

	if(cellVoltage != cellVoltageCopy || cellVRate != cellVRateCopy)
	{
		if(CELLMUX_SetCellsState(cellVoltage, cellVRate))
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

void CONTROL_NotifyEndTest(Boolean Result, Int16U FaultReason, Int16U Warning)
{
	DataTable[REG_TEST_RESULT] = Result ? 1 : 0;
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
	Int16U i;
	
	// Set states
	DataTable[REG_DEV_STATE] = DS_None;
	DataTable[REG_FAULT_REASON] = FAULT_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	
	for(i = REG_VOLTAGE_OK; i <= REG_CELL_STATE_6; ++i)
		DataTable[i] = 0;
}
// ----------------------------------------

static void CONTROL_SwitchToFault(Int16U FaultReason, Int16U ErrorCodeEx)
{
	ZbGPIO_SwitchMeanwell(FALSE);
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

static void CONTROL_StartTest(Int16U VRate, Boolean PerfomRateCorrection)
{
	ZbGPIO_SwitchLEDExt(TRUE);
	ZbGPIO_SwitchLED2(TRUE);
	
	if(!CONTROL_ApplySettings(VRate, PerfomRateCorrection))
	{
		CONTROL_SwitchToFaultEx();
		return;
	}
	
	CONTROL_SetDeviceState(DS_InProcess);
	LOGIC_BeginTest(CONTROL_TimeCounter);
	CycleActive = TRUE;
}
// ----------------------------------------

static Boolean CONTROL_ValidateSettings(Int16U Rate_x10)
{
	Int16U CellVoltage = DataTable[REG_DESIRED_VOLTAGE] / CELLMUX_CellCount();
	Int16U CellRate = Rate_x10 / CELLMUX_CellCount();
	
	if(Rate_x10 < DataTable[REG_UNIT_RATE_MIN] || Rate_x10 > DataTable[REG_UNIT_RATE_MAX])
		return FALSE;

	if(CellRate < DataTable[REG_CELL_MIN_RATE] || CellRate > DataTable[REG_CELL_MAX_RATE])
		return FALSE;
	
	if(CellVoltage < DataTable[REG_CELL_MIN_VOLTAGE] || CellVoltage > DataTable[REG_CELL_MAX_VOLTAGE])
		return FALSE;
	
	return TRUE;
}
// ----------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U UserError)
{
	switch(ActionID)
	{
		case ACT_ENABLE_POWER:
			{
				if(CONTROL_State == DS_None)
				{
					cellVoltageCopy = cellVRateCopy = 0;
					ZbGPIO_SwitchMeanwell(TRUE);
					DELAY_US(MEANWELL_SWITCH_DELAY_US);
					
					if(!CELLMUX_SetCellPowerState(TRUE))
						CONTROL_SwitchToFaultEx();
					else
						CONTROL_SetDeviceState(DS_Ready);
				}
				else
					*UserError = ERR_DEVICE_NOT_READY;
			}
			break;
		case ACT_DISABLE_POWER:
			{
				if(CONTROL_State == DS_Ready || CONTROL_State == DS_InProcess)
				{
					if(!CELLMUX_SetCellPowerState(FALSE))
						CONTROL_SwitchToFaultEx();
					else
					{
						CONTROL_SetDeviceState(DS_None);
						CONTROL_FillWPPartDefault();
					}
					
					DELAY_US(MEANWELL_SWITCH_DELAY_US);
					ZbGPIO_SwitchMeanwell(FALSE);
				}
			}
			break;
		case ACT_APPLY_SETTINGS:
			{
				if(CONTROL_State == DS_Ready)
				{
					if(CONTROL_ValidateSettings(DataTable[REG_VOLTAGE_RATE]))
					{
						if(!CONTROL_ApplySettings(DataTable[REG_VOLTAGE_RATE], DataTable[REG_TUNE_CUSTOM_SETTING]))
							CONTROL_SwitchToFaultEx();
					}
					else
						*UserError = ERR_OUT_OF_RANGE;
				}
			}
			break;

		case ACT_START_TEST_CUSTOM:
			CONTROL_PrepareStart(UserError, DataTable[REG_VOLTAGE_RATE], DataTable[REG_TUNE_CUSTOM_SETTING]);
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
			{
				if(CONTROL_State == DS_Fault)
				{
					CONTROL_SetDeviceState(DS_None);
					CONTROL_FillWPPartDefault();
				}
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

void CONTROL_PrepareStart(pInt16U UserError, Int16U Rate_x10, Boolean UseCustomSettings)
{
	if(CONTROL_State == DS_Ready)
	{
		if(CONTROL_ValidateSettings(Rate_x10))
		{
			CONTROL_HandleFanLogic(TRUE);
			CONTROL_StartTest(Rate_x10, UseCustomSettings);
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
