// -----------------------------------------
// dVdt test routine
// ----------------------------------------

// Header
#include "Logic.h"
//
// Includes
#include "SysConfig.h"
//
#include "ZbBoard.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "CellMux.h"
#include "Controller.h"
#include "SCCITypes.h"


// Variables
//
static volatile Boolean WaitingForStart = FALSE;
static volatile LogicState LOGIC_State = LS_None;
static Int16U InternalFaultReason, ErrorCodeEx = 0;
static Int64U Timeout;


// Forward functions
//
static Boolean LOGIC_TestSequence();


// Functions
//
void LOGIC_BeginTest(Int64U TimerTicks)
{
	Timeout = TimerTicks + TEST_PREPARE_TIMEOUT_MS;
	LOGIC_State = LS_Measure;
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(LOGIC_Update, "ramfuncs");
#endif
void LOGIC_Update(Int64U TimerTicks)
{
	static Int16U i, targetCell;
	static Boolean result, voltageOK;

	switch (LOGIC_State)
	{
		case LS_Measure:
			{
				voltageOK = TRUE;

				for(i = 0; i < MAX_CELLS_COUNT; ++i)
					if (((1 << i) & CELLMUX_CellMask()) != 0)
						if(!(voltageOK &= DataTable[REG_VOLTAGE_OK_1 + i] ? TRUE : FALSE))
							targetCell = i;
				DataTable[REG_VOLTAGE_OK] = voltageOK ? 1 : 0;

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
			}
			break;

		case LS_Pause:
			if (TimerTicks > Timeout)
			{
				CONTROL_NotifyEndTest(result, FAULT_NONE, WARNING_NONE);
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
	ZbGPIO_SwitchLEDExt(FALSE);
}
// ----------------------------------------

Int16U LOGIC_GetFaultReason(pInt16U pErrorCodeEx)
{
	if(pErrorCodeEx)
		*pErrorCodeEx = ErrorCodeEx;

	return InternalFaultReason;
}
// ----------------------------------------

static Boolean LOGIC_TestSequence()
{
	Boolean result;

	ZbGPIO_RelayLine(TRUE);
	DELAY_US(RELAY_SWITCH_DELAY_L_US);

	DINT;

	GpioDataRegs.GPASET.bit.GPIO0 = 1;
	GpioDataRegs.GPASET.bit.GPIO18 = 1;		// RESULT_OUT is used for sync output
	DELAY_US((DataTable[REG_DESIRED_VOLTAGE] / DataTable[REG_VOLTAGE_RATE]) + PRE_PROBE_TIME_US);
	result = ZbGPIO_ReadDetector();
	ZbGPIO_SwitchStartPulse(FALSE);
	ZbGPIO_SwitchResultOut(FALSE);			// RESULT_OUT is used for sync output

	EINT;

	ZbGPIO_RelayLine(FALSE);
	DELAY_US(RELAY_SWITCH_DELAY_US);

	return result;
}
// ----------------------------------------

// No more.
