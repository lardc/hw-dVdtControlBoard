// -----------------------------------------
// Gate voltage setpoint generator
// ----------------------------------------

// Header
#include "Setpoint.h"
//
// Includes
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "Global.h"
#include "CellMux.h"

// Definitions
#define SETPOINT_ARRAY_SIZE		7

// Functions
//
Int16U SP_Generate(Int16U CellIndex, Int16U VRate)
{
	Int16U i, result;
	Int16U BaseDTGateVAddr = REG_CELL1_GATEV1 + CellIndex * SETPOINT_ARRAY_SIZE * 2;
	Int16U BaseDTVRateAddr = REG_CELL1_VRATE1 + CellIndex * SETPOINT_ARRAY_SIZE * 2;

	// For incorrect cell
	if (CELLMUX_CellMask() & (1 << CellIndex) == 0)
		return 0;

	// For voltage rate out of range
	if (VRate < DataTable[BaseDTVRateAddr])
		return DataTable[BaseDTGateVAddr];

	// Find direct matching
	for (i = 0; i < SETPOINT_ARRAY_SIZE; i++)
		if (DataTable[BaseDTVRateAddr + i * 2] == VRate)
			return DataTable[BaseDTGateVAddr + i * 2];

	// Interpolate and extrapolate
	for (i = SETPOINT_ARRAY_SIZE - 1; i > 0; i--)
	{
		if (VRate > DataTable[BaseDTVRateAddr + (i - 1) * 2])
		{
			result = DataTable[BaseDTGateVAddr + (i - 1) * 2] +
					(DataTable[BaseDTGateVAddr + i * 2] - DataTable[BaseDTGateVAddr + (i - 1) * 2]) /
					(DataTable[BaseDTVRateAddr + i * 2] - DataTable[BaseDTVRateAddr + (i - 1) * 2]) *
					(VRate - DataTable[BaseDTVRateAddr + (i - 1) * 2]);

			return result;
		}
		else
			continue;
	}

	return 0;
}
// ----------------------------------------

// No more.
