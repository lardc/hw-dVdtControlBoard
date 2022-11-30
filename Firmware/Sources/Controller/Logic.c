// ----------------------------------------
// dVdt test routine
// ----------------------------------------

// Header
#include "Logic.h"

// Include
#include "SysConfig.h"
#include "ZbBoard.h"
#include "ZbGPIO.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "CellMux.h"
#include "Controller.h"
#include "SCCITypes.h"

// Variables
static volatile Boolean WaitingForStart = FALSE;
static volatile LogicState LOGIC_State = LS_None;
static Int16U InternalFaultReason, ErrorCodeEx = 0;
static Int64U Timeout;

// Functions
void LOGIC_BeginTest(Int64U TimerTicks)
{
	Timeout = TimerTicks + TEST_PREPARE_TIMEOUT_MS;
	LOGIC_State = LS_Measure;
}
// ----------------------------------------

void LOGIC_ApplyParameters(Int64U TimerTicks)
{
	Timeout = TimerTicks + TEST_PREPARE_TIMEOUT_MS;
	LOGIC_State = LS_Apply;
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(LOGIC_Update, "ramfuncs");
#endif
void LOGIC_Update(Int64U TimerTicks)
{
	static Int16U i, targetCell;
	static Boolean result, voltageOK;

	// Сбор данных о готовности напряжений ячеек
	if(LOGIC_State != LS_None)
	{
		voltageOK = TRUE;

		for(i = 0; i < MAX_CELLS_COUNT; ++i)
			if((1 << i) & DataTable[REG_CELL_MASK])
				if(!(voltageOK &= DataTable[REG_VOLTAGE_OK_1 + i]))
					targetCell = i;

		DataTable[REG_VOLTAGE_OK] = voltageOK ? 1 : 0;
	}

	switch(LOGIC_State)
	{
		case LS_Measure:
			if(voltageOK)
			{
				result = LOGIC_TestSequence();

				Timeout = (result) ? (TimerTicks + TEST_PAUSE_SHORT_MS) : (TimerTicks + TEST_PAUSE_LONG_MS);
				LOGIC_State = LS_Pause;
			}
			else if(TimerTicks > Timeout)
			{
				CONTROL_NotifyEndTest(FALSE, FAULT_NOT_READY_1 + targetCell, WARNING_NONE);
				LOGIC_Reset();
			}
			break;

		case LS_Pause:
			if(TimerTicks > Timeout)
			{
				CONTROL_NotifyEndTest(result, FAULT_NONE, WARNING_NONE);
				LOGIC_Reset();
			}
			break;

		case LS_Apply:
			if(voltageOK)
			{
				CONTROL_NotifyEndTest(FALSE, FAULT_NONE, WARNING_NONE);
				LOGIC_Reset();
			}
			else if(TimerTicks > Timeout)
			{
				CONTROL_NotifyEndTest(FALSE, FAULT_NOT_READY_1 + targetCell, WARNING_NONE);
				LOGIC_Reset();
			}
			break;

		default:
			break;
	}
}
// ----------------------------------------

void LOGIC_Reset()
{
	LOGIC_State = LS_None;
	ZbGPIO_SwitchLED2(FALSE);
	ZbGPIO_SwitchOutRelay(FALSE);
}
// ----------------------------------------

Int16U LOGIC_GetFaultReason(pInt16U pErrorCodeEx)
{
	if(pErrorCodeEx)
		*pErrorCodeEx = ErrorCodeEx;

	return InternalFaultReason;
}
// ----------------------------------------

Boolean LOGIC_TestSequence()
{
	Boolean Result;
	Int32U Delay = 10ul * DataTable[REG_DESIRED_VOLTAGE] / DataTable[REG_VOLTAGE_RATE] + PRE_PROBE_TIME_US;

	DINT;
	ZbGPIO_SwitchStartPulse(TRUE);
	ZbGPIO_SwitchResultOut(TRUE);			// RESULT_OUT is used for sync oscilloscope

	DELAY_US(Delay);

	ZbGPIO_SwitchStartPulse(FALSE);
	ZbGPIO_SwitchOutRelay(FALSE);
	ZbGPIO_SwitchLED2(FALSE);

	Result = ZbGPIO_ReadDetectorPin();

	ZbGPIO_SwitchResultOut(FALSE);			// RESULT_OUT is used for sync oscilloscope
	EINT;

	DELAY_US(RELAY_SWITCH_DELAY_US);

	return Result;
}
// ----------------------------------------
