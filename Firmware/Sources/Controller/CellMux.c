// -----------------------------------------
// dVdt cell commutator
// ----------------------------------------

// Header
#include "CellMux.h"
//
// Includes
#include "SysConfig.h"
//
#include "ZbBoard.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "SCCIMaster.h"
#include "Constraints.h"
#include "Controller.h"
#include "Setpoint.h"

// Constants
#define CELL_MAPPING_SOURCE_START		10
#define CELL_MAPPING_DEST_START			3
#define CELL_MAPPING_COUNT				1

// Variables
//
static Int16U InternalFaultReason = FAULT_NONE, ErrorCodeEx = ERR_NO_ERROR;
static SCCIM_Interface DEVICE_UART_Interface;
static SCCI_IOConfig UART_IOConfig;
static volatile Boolean ThreadLocker = FALSE;

// Functions
//
void CELLMUX_Init()
{
	// Init interface
	UART_IOConfig.IO_SendArray16 = &ZwSCIb_SendArray16;
	UART_IOConfig.IO_ReceiveArray16 = &ZwSCIb_ReceiveArray16;
	UART_IOConfig.IO_GetBytesToReceive = &ZwSCIb_GetBytesToReceive;
	UART_IOConfig.IO_ReceiveByte = &ZwSCIb_ReceiveChar;

	// Init interface driver
	SCCIM_Init(&DEVICE_UART_Interface, &UART_IOConfig, SCCI_TIMEOUT_TICKS_MS, &CONTROL_TimeCounter);
}
// ----------------------------------------

Int16U CELLMUX_CellCount()
{
	Int16U i, Counter = 0;

	for (i = 0; i < MAX_CELLS_COUNT; ++i)
		if (DataTable[REG_CELL_MASK] & (1 << i)) ++Counter;

	return Counter;
}
// ----------------------------------------

Int16U CELLMUX_CellMask()
{
	return DataTable[REG_CELL_MASK];
}
// ----------------------------------------

Boolean CELLMUX_SetCellPowerState(Boolean Enable)
{
	Int16U i;

	ThreadLocker = TRUE;

	for(i = 0; i < MAX_CELLS_COUNT; ++i)
		if (((1 << i) & CELLMUX_CellMask()) != 0)
		{
			ZbGPIO_SetActiveCell(i);

			if((ErrorCodeEx = SCCIM_Call(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID,
										 Enable ? CELL_ACT_ENABLE_POWER : CELL_ACT_DISABLE_POWER)) == ERR_NO_ERROR)
				continue;

			InternalFaultReason = FAULT_LINK_CELL_1 + i;

			ThreadLocker = FALSE;
			return FALSE;
		}

	ThreadLocker = FALSE;
	return TRUE;
}
// ----------------------------------------

#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(CELLMUX_ReadStates, "ramfuncs");
#endif
Boolean CELLMUX_ReadStates()
{
	Int16U i;

	if(ThreadLocker)
		return TRUE;

	ThreadLocker = TRUE;

	for(i = 0; i < MAX_CELLS_COUNT; ++i)
		if (((1 << i) & CELLMUX_CellMask()) != 0)
		{
			ZbGPIO_SetActiveCell(i);

			if((ErrorCodeEx = SCCIM_Read16(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID,
										   CELL_REG_VOLTAGE_OK, (pInt16U)&DataTable[REG_VOLTAGE_OK_1 + i])) == ERR_NO_ERROR)
				if((ErrorCodeEx = SCCIM_Read16(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID,
											   CELL_REG_ACTUAL_VOLTAGE, (pInt16U)&DataTable[REG_ACTUAL_VOLTAGE_1 + i])) == ERR_NO_ERROR)
					if((ErrorCodeEx = SCCIM_Read16(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID,
												   CELL_REG_DEV_STATE, (pInt16U)&DataTable[REG_CELL_STATE_1 + i])) == ERR_NO_ERROR)
						continue;

			InternalFaultReason = FAULT_LINK_CELL_1 + i;

			ThreadLocker = FALSE;
			return FALSE;
		}

	ThreadLocker = FALSE;
	return TRUE;
}
// ----------------------------------------

Boolean CELLMUX_SetCellsState(Int16U CellVoltage, Int16U CellVRate, Int16U Fine)
{
	Int16U i, GateV;

	ThreadLocker = TRUE;

	for(i = 0; i < MAX_CELLS_COUNT; ++i)
		if (((1 << i) & CELLMUX_CellMask()) != 0)
		{
			ZbGPIO_SetActiveCell(i);

			GateV = SP_Generate(i, CellVRate);
			DataTable[REG_DIAG_GATEV_CELL1 + i] = GateV;

			if((ErrorCodeEx = SCCIM_Write16(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID,
											CELL_REG_DESIRED_GATE_V, GateV)) == ERR_NO_ERROR)
				if((ErrorCodeEx = SCCIM_Write16(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID,
												CELL_REG_DESIRED_VOLTAGE, CellVoltage)) == ERR_NO_ERROR)
					if((ErrorCodeEx = SCCIM_Write16(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID,
													CELL_REG_VOLTAGE_FINE_N, Fine)) == ERR_NO_ERROR)
						if((ErrorCodeEx = SCCIM_Call(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID,
													 CELL_ACT_APPLY_PARAMS)) == ERR_NO_ERROR)
						continue;

			InternalFaultReason = FAULT_LINK_CELL_1 + i;

			ThreadLocker = FALSE;
			return FALSE;
		}

	ThreadLocker = FALSE;
	return TRUE;
}
// ----------------------------------------

Int16U CELLMUX_GetFaultReason(pInt16U pErrorCodeEx)
{
	if(pErrorCodeEx)
		*pErrorCodeEx = ErrorCodeEx;

	return InternalFaultReason;
}
// ----------------------------------------

void CELLMUX_ReadCellRegister(Int16U CellIndex, Int16U Register, pInt16U Output)
{
	ThreadLocker = TRUE;

	ZbGPIO_SetActiveCell(CellIndex - 1);
	SCCIM_Read16(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID, Register, Output);

	ThreadLocker = FALSE;
}
// ----------------------------------------

void CELLMUX_WriteCellRegister(Int16U CellIndex, Int16U Register, Int16U Value)
{
	ThreadLocker = TRUE;

	ZbGPIO_SetActiveCell(CellIndex - 1);
    SCCIM_Write16(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID, Register, Value);

	ThreadLocker = FALSE;
}
// ----------------------------------------

void CELLMUX_CallCellAction(Int16U CellIndex, Int16U ActionID)
{
	ThreadLocker = TRUE;

	ZbGPIO_SetActiveCell(CellIndex - 1);
    SCCIM_Call(&DEVICE_UART_Interface, SCCI_CELL_NODE_ID, ActionID);

	ThreadLocker = FALSE;
}
// ----------------------------------------

// No more.
