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
static void CONTROL_StartTest(Int16U VRate, Boolean PerfomRateCorrection, Boolean StartTest);
void CONTROL_PrepareStart(pInt16U UserError, Int16U Rate_x10, Boolean UseCustomSettings, Boolean StartTest);

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

Int16U CONTROL_CalOutVoltage()
{
    Int16U Voltage;

    Voltage = (Int32U)DataTable[REG_DESIRED_VOLTAGE] * DataTable[REG_V_FINE_N] / DataTable[REG_V_FINE_D]
              + (Int16S)DataTable[REG_V_OFFSET] + (Int16S)DataTable[REG_CSU_V_OFFSET];

    return Voltage;
}

Boolean CONTROL_EnableSingleCellMode()
{
    Int16U Voltage;

     Voltage = (Int32U)DataTable[REG_DESIRED_VOLTAGE];

     // Условие активации работы с одиночной ячейкой
     Boolean SingleCellMode = (Voltage <= DataTable[REG_SINGLE_CELL_V_LEVEL] && DataTable[REG_VOLTAGE_RATE] < MIN_RATE_FOR_DUO_MODE);

     return SingleCellMode;
}

Boolean CONTROL_EnableDuoCellMode()
{
    Int16U Voltage;

     Voltage = (Int32U)DataTable[REG_DESIRED_VOLTAGE];

     // Условие активации работы с двумя ячейками
     Boolean DuoCellMode = (Voltage <= DataTable[REG_DUO_CELL_V_LEVEL] && DataTable[REG_VOLTAGE_RATE] > MIN_RATE_FOR_DUO_MODE
    		 	 	 	 	 && DataTable[REG_VOLTAGE_RATE] < MAX_RATE_FOR_DUO_MODE);

     return DuoCellMode;
}

Int16U CONTROL_СalculationRateXMode(Int16U MaxRate, Int16U MinRate, Int16U VRate, Int16U RegCorrByRate, Int16U RegCorrRateVpoint, Int16U RegCorrRateByVoltage,
		Int16U RegCorrRange1, Int16U RegCorrRange2, Int16U RegOffsetRange2, Int16U RegRateGlobalKN, Int16U RegRateGlobalKD, Int16U OffsetByVoltage, Boolean EnableTuneLow)
{
	Int16U  KRate, CalRate;

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

	Int16U VRateMax = DataTable[MaxRate];
	Int16U VRateMin = DataTable[MinRate];

	if ((DataTable[REG_DESIRED_VOLTAGE] < 1500) && ((DataTable[REG_VOLTAGE_RATE] / 10) >= 1600))
		VRate = (Int32U)(VRate - (Int32U)VRate * (1500 - DataTable[REG_DESIRED_VOLTAGE])/(1500 - 500) / DataTable[REG_CORR_VOL_RATE1] * 10);

	if (DataTable[REG_UNIT_USE_RANGE1] && (VRate < SP_GetRange1MaxRate()))
	{
		VRate = (Int32U)VRate * DataTable[RegCorrRange1] /1000;

	}
	else if (DataTable[REG_UNIT_USE_RANGE2] && (VRate < SP_GetRange2MaxRate()))
	{
		VRate = (Int32U)VRate * DataTable[RegCorrRange2] /1000 + (Int16S)DataTable[RegOffsetRange2];
	}
	else
	{
		// by rate (and full range voltage)
		if(CORR_RATE_BY_FS_VOLTAGE)
		{
			KRate = ((Int32U)DataTable[RegCorrByRate] * (DESIRED_VOLTAGE_MAX - DataTable[REG_DESIRED_VOLTAGE]))
					/ (DESIRED_VOLTAGE_MAX - DESIRED_VOLTAGE_MIN);
		}
		else
			KRate = DataTable[RegCorrByRate];

		// by rate
		if (EnableTuneLow)
			VRate = ((100 + (((Int32U)(VRateMax - VRate) * KRate) / (VRateMax - VRateMin))) * VRate) / 100;
		else
			VRate = ((100 + (((Int32U)(VRate - VRateMin) * KRate) / (VRateMax - VRateMin))) * VRate) / 100;

		// by voltage (lower zone)
		if (DataTable[RegCorrRateVpoint] > DataTable[REG_DESIRED_VOLTAGE])
		{
			VRate = (((((Int32U)(DataTable[RegCorrRateVpoint] - DataTable[REG_DESIRED_VOLTAGE])
					* DataTable[RegCorrRateByVoltage]) / (DataTable[RegCorrRateVpoint] - DESIRED_VOLTAGE_MIN))
					+ 100) * VRate) / 100 + (Int16S)DataTable[OffsetByVoltage];
		}

		// global correction
		VRate = (Int32U)VRate * DataTable[RegRateGlobalKN] / DataTable[RegRateGlobalKD];

	}

	CalRate = VRate;

	return CalRate;
}
// ----------------------------------------

Int16U CONTROL_СalculationRateSingleMode(Int16U VRate)
{
    return CONTROL_СalculationRateXMode(REG_SINGLE_RATE_MAX, REG_SINGLE_RATE_MIN, VRate, REG_SINGLE_CORR_BY_RATE, REG_SINGLE_CORR_VPOINT, REG_SINGLE_CORR_BY_VOLTAGE,
                                  REG_SINGLE_CORR_RANGE1, REG_SINGLE_CORR_RANGE2, REG_SINGLE_OFFSET_RANGE2, REG_SINGLE_RATE_GLOBAL_K_N, REG_SINGLE_RATE_GLOBAL_K_D, REG_SINGLE_RATE_OFFSET, FALSE);
}

Int16U CONTROL_СalculationRateFullMode(Int16U VRate)
{
    return CONTROL_СalculationRateXMode(REG_UNIT_RATE_MAX, REG_UNIT_RATE_MIN, VRate, REG_CORR_RATE_BY_RATE, REG_CORR_RATE_VPOINT, REG_CORR_RATE_BY_VOLTAGE,
                                   REG_CORR_RANGE1, REG_CORR_RANGE2, REG_OFFSET_RANGE2, REG_RATE_GLOBAL_K_N, REG_RATE_GLOBAL_K_N, REG_RATE_OFFSET, FALSE);
}

Boolean CONTROL_ApplySettings(Int16U VRate, Boolean PerfomRateCorrection)
{
	Int16U Voltage, cellVoltage, cellVRate;

	Voltage = CONTROL_CalOutVoltage();
	
	// Perfom rate correction
	if(PerfomRateCorrection)
	{
	    if(CONTROL_EnableSingleCellMode())
	    {
	        cellVRate = CONTROL_СalculationRateSingleMode(VRate);
	    }
	    else
	    {
            cellVRate = CONTROL_СalculationRateFullMode(VRate);
	    }
	}
	

	if(CONTROL_EnableSingleCellMode())
	{
		cellVoltage = Voltage;
		cellVRate = cellVRate;

		DataTable[REG_DBG_DATA] = 1;
	}
	else if (CONTROL_EnableDuoCellMode())
	{
        cellVoltage = Voltage / 2;
        cellVRate = cellVRate / 2;

        DataTable[REG_DBG_DATA] = 2;
	}
	else
	{
		cellVoltage = Voltage / CELLMUX_CellCount();
		cellVRate = cellVRate / CELLMUX_CellCount();

		DataTable[REG_DBG_DATA] = 3;
	}

	if(cellVoltage != cellVoltageCopy || cellVRate != cellVRateCopy)
	{
		if(CELLMUX_SetCellsState(cellVoltage, cellVRate, CONTROL_EnableSingleCellMode(), CONTROL_EnableDuoCellMode()))
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
	DataTable[REG_TEST_FINISHED] = OPRESULT_NONE;
	DataTable[REG_TEST_RESULT] = TEST_RESULT_NULL;
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


void CONTROL_NotifyEndTest(Boolean Result, Int16U FaultReason, Int16U Warning)
{
	DataTable[REG_TEST_RESULT] = Result ? TEST_RESULT_OK : TEST_RESULT_FAIL;
	DataTable[REG_WARNING] = Warning;
	DataTable[REG_TEST_FINISHED] = (FaultReason == FAULT_NONE) ? OPRESULT_OK : OPRESULT_FAIL;
	
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
	DataTable[REG_TEST_FINISHED] = OPRESULT_NONE;
	DataTable[REG_TEST_RESULT] = TEST_RESULT_NULL;
	
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

static void CONTROL_StartTest(Int16U VRate, Boolean PerfomRateCorrection, Boolean StartTest)
{
	CONTROL_HandleExtLed(StartTest);
	ZbGPIO_SwitchOutRelay(StartTest);
	ZbGPIO_SwitchLED2(StartTest);

	DataTable[REG_TEST_FINISHED] = OPRESULT_NONE;
	DataTable[REG_TEST_RESULT] = TEST_RESULT_NULL;
	
	if(!CONTROL_ApplySettings(VRate, PerfomRateCorrection))
	{
		CONTROL_SwitchToFaultEx();
		return;
	}

	CONTROL_SetDeviceState(DS_InProcess);

	if (StartTest)
	{
	    LOGIC_BeginTest(CONTROL_TimeCounter);
	}
	else
	{
	    LOGIC_ApplyParameters(CONTROL_TimeCounter);
	}

	CycleActive = TRUE;
}
// ----------------------------------------

static Boolean CONTROL_ValidateSettings(Int16U Rate_x10)
{
    Int16U Voltage = CONTROL_CalOutVoltage();
	Int16U CellVoltage = Voltage / CELLMUX_CellCount();
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
				    CONTROL_PrepareStart(UserError, DataTable[REG_VOLTAGE_RATE], DataTable[REG_TUNE_CUSTOM_SETTING], FALSE);
				}
			}
			break;

        case ACT_ENABLE_EXT_SYNC_START:
            {
                if(CONTROL_State == DS_Ready)
                {
                    CONTROL_EnableExternalSync(TRUE);
                }
            }
            break;

        case ACT_DISABLE_EXT_SYNC_START:
            {
                CONTROL_EnableExternalSync(FALSE);
            }
            break;

		case ACT_START_TEST_100:
			CONTROL_PrepareStart(UserError, 100 * 10, TRUE, TRUE);
			break;

		case ACT_START_TEST_200:
			CONTROL_PrepareStart(UserError, 200 * 10, TRUE, TRUE);
			break;

		case ACT_START_TEST_320:
			CONTROL_PrepareStart(UserError, 320 * 10, TRUE, TRUE);
			break;

		case ACT_START_TEST_CUSTOM:
			CONTROL_PrepareStart(UserError, DataTable[REG_VOLTAGE_RATE], DataTable[REG_TUNE_CUSTOM_SETTING], TRUE);
			break;

		case ACT_START_TEST_500:
			CONTROL_PrepareStart(UserError, 500 * 10, TRUE, TRUE);
			break;

		case ACT_START_TEST_1000:
			CONTROL_PrepareStart(UserError, 1000 * 10, TRUE, TRUE);
			break;

		case ACT_START_TEST_1600:
			CONTROL_PrepareStart(UserError, 1600 * 10, TRUE, TRUE);
			break;

		case ACT_START_TEST_2000:
			CONTROL_PrepareStart(UserError, 2000 * 10, TRUE, TRUE);
			break;

		case ACT_START_TEST_2500:
			CONTROL_PrepareStart(UserError, 2500 * 10, TRUE, TRUE);
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

void CONTROL_PrepareStart(pInt16U UserError, Int16U Rate_x10, Boolean UseCustomSettings, Boolean StartTest)
{
	if(CONTROL_State == DS_Ready)
	{
		if(CONTROL_ValidateSettings(Rate_x10))
		{
			CONTROL_HandleFanLogic(StartTest);
			CONTROL_StartTest(Rate_x10, UseCustomSettings, StartTest);
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
