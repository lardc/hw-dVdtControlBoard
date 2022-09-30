﻿// ----------------------------------------
// Gate voltage setpoint generator
// ----------------------------------------

// Header
#include "Setpoint.h"

// Includes
#include "DeviceObjectDictionary.h"
#include "DataTable.h"
#include "Global.h"
#include "CellMux.h"
#include "Controller.h"

// Definitions
#define SETPOINT_ARRAY_SIZE		7

// Forward functions
Int16U SP_GetDistanceToRange(Int16U CellIndex, Int16U VRate, Int16U RangeBaseRateRegister, Boolean RangeIsActive);
Int16U SP_FindActiveRange(Int16U CellIndex, Int16U VRate);
Int16U SP_GetRangeXMaxRate(Int16U RangeBaseRateRegister);

// Functions
Int16U SP_Generate(Int16U CellIndex, Int16U VRate, pInt16U RateRange)
{
	Int16U i, GateVStartReg, VRateStartReg;
	
	*RateRange = SP_FindActiveRange(CellIndex, VRate);
	switch(*RateRange)
	{
		case VRATE_RANGE_LOWER1:
			GateVStartReg = REG_CELL1_R1_GATEV1;
			VRateStartReg = REG_CELL1_R1_VRATE1;
			break;
			
		case VRATE_RANGE_LOWER2:
			GateVStartReg = REG_CELL1_R2_GATEV1;
			VRateStartReg = REG_CELL1_R2_VRATE1;
			break;
			
		default:
			GateVStartReg = REG_CELL1_GATEV1;
			VRateStartReg = REG_CELL1_VRATE1;
			break;
	}
	
	Int16U BaseDTGateVAddr = GateVStartReg + CellIndex * SETPOINT_ARRAY_SIZE * 2;
	Int16U BaseDTVRateAddr = VRateStartReg + CellIndex * SETPOINT_ARRAY_SIZE * 2;
	
	// For incorrect cell
	if((DataTable[REG_CELL_MASK] & (1 << CellIndex)) == 0)
		return 0;
	
	// For voltage rate out of range
	if(VRate < DataTable[BaseDTVRateAddr])
		return DataTable[BaseDTGateVAddr];
	
	// Find direct matching
	for(i = 0; i < SETPOINT_ARRAY_SIZE; i++)
		if(DataTable[BaseDTVRateAddr + i * 2] == VRate)
			return DataTable[BaseDTGateVAddr + i * 2];
	
	// Interpolate and extrapolate
	for(i = SETPOINT_ARRAY_SIZE - 1; i > 0; i--)
	{
		if(VRate > DataTable[BaseDTVRateAddr + (i - 1) * 2])
		{
			Int32U Base = DataTable[BaseDTGateVAddr + (i - 1) * 2];
			Int32U N = DataTable[BaseDTGateVAddr + i * 2] - DataTable[BaseDTGateVAddr + (i - 1) * 2];
			Int32U D = DataTable[BaseDTVRateAddr + i * 2] - DataTable[BaseDTVRateAddr + (i - 1) * 2];
			Int32U K = VRate - DataTable[BaseDTVRateAddr + (i - 1) * 2];
			
			return Base + N * K / D;
		}
		else
			continue;
	}
	
	return 0;
}
// ----------------------------------------

Int16U SP_GetDistanceToRange(Int16U CellIndex, Int16U VRate, Int16U RangeBaseRateRegister, Boolean RangeIsActive)
{
	if(RangeIsActive)
	{
		Int16U RegRateStart = RangeBaseRateRegister + CellIndex * SETPOINT_ARRAY_SIZE * 2;
		Int16U RegRateEnd = RegRateStart + (SETPOINT_ARRAY_SIZE - 1) * 2;

		Int16U RateStart = DataTable[RegRateStart];
		Int16U RateEnd = DataTable[RegRateEnd];

		if((RateStart <= VRate) && (VRate <= RateEnd))
			return 0;
		else
		{
			Int16U DistStart = ABS((Int32S)VRate - RateStart);
			Int16U DistEnd = ABS((Int32S)VRate - RateEnd);

			return MIN(DistStart, DistEnd);
		}
	}
	else
		return INT16U_MAX;
}
// ----------------------------------------

Int16U SP_FindActiveRange(Int16U CellIndex, Int16U VRate)
{
	Int16U DistR1, DistR2, DistDef;

	// Поиск попадания в границы диапазона
	if((DistR1 = SP_GetDistanceToRange(CellIndex, VRate, REG_CELL1_R1_VRATE1,
			DataTable[REG_UNIT_USE_RANGE_LOW])) == 0)
		return VRATE_RANGE_LOWER1;

	if((DistR2 = SP_GetDistanceToRange(CellIndex, VRate, REG_CELL1_R2_VRATE1,
			DataTable[REG_UNIT_USE_RANGE_MID])) == 0)
		return VRATE_RANGE_LOWER2;

	if((DistDef = SP_GetDistanceToRange(CellIndex, VRate, REG_CELL1_VRATE1, TRUE)) == 0)
		return VRATE_RANGE_DEF;

	// Если значение не попало в диапазон, то определяем самый ближний диапазон
	Int16U ClosestDist = MIN(MIN(DistR1, DistR2), DistDef);
	if(ClosestDist == DistR1)
		return VRATE_RANGE_LOWER1;
	else if(ClosestDist == DistR2)
		return VRATE_RANGE_LOWER2;
	else
		return VRATE_RANGE_DEF;
}
// ----------------------------------------

Int16U SP_GetRangeXMaxRate(Int16U RangeBaseRateRegister)
{
	Int32U i, MaxRate = INT32U_MAX;

	for(i = 0; i < MAX_CELLS_COUNT; ++i)
		if((1 << i) & DataTable[REG_CELL_MASK])
			MaxRate = MIN(MaxRate, DataTable[RangeBaseRateRegister + i * SETPOINT_ARRAY_SIZE * 2]);

	return MaxRate;
}
// ----------------------------------------

Int16U SP_GetRange1MaxRate()
{
	return SP_GetRangeXMaxRate(REG_CELL1_R1_VRATE1 + ((SETPOINT_ARRAY_SIZE * 2)-2));
}
// ----------------------------------------

Int16U SP_GetRange2MaxRate()
{
	return SP_GetRangeXMaxRate(REG_CELL1_R2_VRATE1 + ((SETPOINT_ARRAY_SIZE * 2)-2));
}
// ----------------------------------------
