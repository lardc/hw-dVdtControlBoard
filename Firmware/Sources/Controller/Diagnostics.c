// ----------------------------------------
// dVdt diagnostics routines
// ----------------------------------------

// Header
#include "Diagnostics.h"

// Include
#include "ZwDSP.h"
#include "ZbBoard.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "CellMux.h"
#include "Setpoint.h"
#include "Logic.h"

// Functions
Boolean DIAG_DispatchCommand(Int16U Command)
{
	switch(Command)
	{
		case ACT_DIAG_PULSE_FAN:
			ZbGPIO_SwitchFAN(TRUE);
			DELAY_US(1000000);
			ZbGPIO_SwitchFAN(FALSE);
			break;

		case ACT_DIAG_PULSE_START:
			ZbGPIO_SwitchOutRelay(TRUE);
			DataTable[REG_TEST_RESULT] = LOGIC_TestSequence();
			break;

		case ACT_DIAG_PULSE_SWITCH:
			ZbGPIO_SwitchOutRelay(TRUE);
			DELAY_US(1000000);
			ZbGPIO_SwitchOutRelay(FALSE);
			break;

		case ACT_DIAG_PULSE_LED:
			ZbGPIO_SwitchExtLed(TRUE);
			DELAY_US(1000000);
			ZbGPIO_SwitchExtLed(FALSE);
			break;

		case ACT_DIAG_PULSE_SYNC:
			ZbGPIO_SwitchResultOut(TRUE);
			DELAY_US(100000);
			ZbGPIO_SwitchResultOut(FALSE);
			break;

		case ACT_DIAG_READ_CELL_REG:
			DataTable[REG_DIAG_GENERAL_OUT] = 0;
			CELLMUX_ReadCellRegister(DataTable[REG_DIAG_TEST_CELL_ID], DataTable[REG_DIAG_TEST_PARAM_1], (pInt16U)&DataTable[REG_DIAG_GENERAL_OUT]);
			break;

		case ACT_DIAG_WRITE_CELL_REG:
			CELLMUX_WriteCellRegister(DataTable[REG_DIAG_TEST_CELL_ID], DataTable[REG_DIAG_TEST_PARAM_1], DataTable[REG_DIAG_TEST_PARAM_2]);
			break;

		case ACT_DIAG_CALL_CELL:
			CELLMUX_CallCellAction(DataTable[REG_DIAG_TEST_CELL_ID], DataTable[REG_DIAG_TEST_PARAM_1]);
			break;

		case ACT_DIAG_READ_DETECTOR:
		    DataTable[REG_TEST_RESULT] = ZbGPIO_ReadDetectorPin() ? OPRESULT_OK : OPRESULT_FAIL;
		    break;

		case ACT_DIAG_GENERATE_SETP:
			SP_GetSetpointArray(DataTable[REG_VOLTAGE_RATE] / CELLMUX_CellCount(), NULL, NULL);
			break;

		default:
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------
