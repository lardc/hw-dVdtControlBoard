// -----------------------------------------
// dVdt diagnostics routines
// ----------------------------------------

// Header
#include "Diagnostics.h"
//
#include "ZwDSP.h"
#include "ZbBoard.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "CellMux.h"
#include "Setpoint.h"

// Functions
//
Boolean DIAG_DispatchCommand(Int16U Command)
{
	switch(Command)
	{
		case ACT_DIAG_SWITCH_ON:
			ZbGPIO_RelayLine(TRUE);
			break;
		case ACT_DIAG_SWITCH_OFF:
			ZbGPIO_RelayLine(FALSE);
			break;
		case ACT_DIAG_MW_ON:
			ZbGPIO_SwitchMeanwell(TRUE);
			break;
		case ACT_DIAG_MW_OFF:
			ZbGPIO_SwitchMeanwell(FALSE);
			break;
		case ACT_DIAG_PULSE_START:
			{
				ZbGPIO_SwitchLEDExt(TRUE);

				ZbGPIO_RelayLine(TRUE);
				DELAY_US(RELAY_SWITCH_DELAY_L_US);

				DINT;

				ZbGPIO_SwitchResultOut(TRUE);
				ZbGPIO_SwitchStartPulse(TRUE);
				DELAY_US(PRE_PROBE_TIME_DIAG_US);
				ZbGPIO_SwitchStartPulse(FALSE);
				ZbGPIO_SwitchResultOut(FALSE);

				EINT;

				DELAY_US(100);
				ZbGPIO_RelayLine(FALSE);
				DELAY_US(RELAY_SWITCH_DELAY_US);

				ZbGPIO_SwitchLEDExt(FALSE);
			}
			break;
		case ACT_DIAG_PULSE_SWITCH:
			ZbGPIO_RelayLine(TRUE);
			DELAY_US(100000);
			ZbGPIO_RelayLine(FALSE);
			break;
		case ACT_DIAG_PULSE_LED:
			ZbGPIO_SwitchLEDExt(TRUE);
			DELAY_US(100000);
			ZbGPIO_SwitchLEDExt(FALSE);
			break;
		case ACT_DIAG_PULSE_SYNC:
			ZbGPIO_SwitchResultOut(TRUE);
			DELAY_US(100000);
			ZbGPIO_SwitchResultOut(FALSE);
			break;
		case ACT_DIAG_DETECTOR:
			DataTable[REG_DIAG_DETECTOR] =  ZbGPIO_ReadDetector();
			break;
		case ACT_DIAG_GENERATE_SETP:
			{
				Int16U i, RatePerCell = DataTable[REG_VOLTAGE_RATE] / CELLMUX_CellCount(), dummy;
				for (i = 0; i < MAX_CELLS_COUNT; i++)
					if (CELLMUX_CellMask() & (1 << i))
						DataTable[REG_DIAG_GATEV_CELL1 + i] = SP_Generate(i, RatePerCell, &dummy);
			}
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
		default:
			return FALSE;
	}

	return TRUE;
}
// ----------------------------------------

// No more

