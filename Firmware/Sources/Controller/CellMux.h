// ----------------------------------------
// dVdt cell commutator
// ----------------------------------------

#ifndef __CELL_MUX_H
#define __CELL_MUX_H

// Include
#include "stdinc.h"
//
#include "ZwDSP.h"
#include "Global.h"

// Functions
//
// Initialize interface
void CELLMUX_Init();
// Return cell mask parameters
Int16U CELLMUX_CellCount();
Int16U CELLMUX_CellMask();
// Begin preparation and test
Boolean CELLMUX_SetCellPowerState(Boolean Enable);
// Read state of cells
Boolean CELLMUX_ReadStates();
// Apply settings to cells
Boolean CELLMUX_SetCellsState(Int16U CellVoltage, Int16U CellVRate, Int16U Fine);
// Get fault information
Int16U CELLMUX_GetFaultReason(pInt16U pErrorCodeEx);
// Read cell register
void CELLMUX_ReadCellRegister(Int16U CellIndex, Int16U Register, pInt16U Output);
// Write into register fo specified cell
void CELLMUX_WriteCellRegister(Int16U CellIndex, Int16U Register, Int16U Value);
// Call action in specified cell
void CELLMUX_CallCellAction(Int16U CellIndex, Int16U ActionID);

#endif // __CELL_MUX_H
