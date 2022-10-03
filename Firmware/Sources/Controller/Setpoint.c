// ----------------------------------------
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
#define SETPOINT_RANGES			3

// Forward functions
Boolean SP_Generate(Int16U CellIndex, Int16U VRate_x10, pInt16U RateRange, pInt16U GateV);

// Functions
Boolean SP_Generate(Int16U CellIndex, Int16U VRate_x10, pInt16U RateRange, pInt16U GateV)
{
	Boolean Result = FALSE;
	Int16U i, RangeStartRegs[SETPOINT_RANGES] = {REG_CELL1_LOW_GATEV1, REG_CELL1_MID_GATEV1, REG_CELL1_HIGH_GATEV1};

	// Поиск активных диапазонов с подходящей скоростью нарастания
	for(i = 0; i <= SETPOINT_RANGES; i++)
	{
		// Проверка при активном диапазоне
		if(DataTable[i + REG_UNIT_USE_RANGE_LOW])
		{
			// Определение граничных скоростей нарастания в диапазоне
			Int16U CellRateStartIndex = RangeStartRegs[i] + CellIndex * SETPOINT_ARRAY_SIZE * 2 + 1;
			Int16U CellRateEndIndex = CellRateStartIndex + (SETPOINT_ARRAY_SIZE - 1) * 2;

			if(DataTable[CellRateStartIndex] <= VRate_x10 && VRate_x10 <= DataTable[CellRateEndIndex])
			{
				Result = TRUE;
				*RateRange = i;
				break;
			}
		}
	}
	
	// Если нет попадания в активный диапазон - ошибка
	if(!Result)
		return FALSE;
	
	Int16U BaseDTGateVAddr = RangeStartRegs[i] + CellIndex * SETPOINT_ARRAY_SIZE * 2;
	Int16U BaseDTVRateAddr = BaseDTGateVAddr + 1;
	
	// Проверка на прямое совпадение
	for(i = 0; i < SETPOINT_ARRAY_SIZE; i++)
	{
		if(DataTable[BaseDTVRateAddr + i * 2] == VRate_x10)
		{
			*GateV = DataTable[BaseDTGateVAddr + i * 2];
			return TRUE;
		}
	}
	
	// Интерполяция значений
	for(i = SETPOINT_ARRAY_SIZE - 1; i > 0; i--)
	{
		if(VRate_x10 > DataTable[BaseDTVRateAddr + (i - 1) * 2])
		{
			Int32U Base = DataTable[BaseDTGateVAddr + (i - 1) * 2];
			Int32U N = DataTable[BaseDTGateVAddr + i * 2] - DataTable[BaseDTGateVAddr + (i - 1) * 2];
			Int32U D = DataTable[BaseDTVRateAddr + i * 2] - DataTable[BaseDTVRateAddr + (i - 1) * 2];
			Int32U K = VRate_x10 - DataTable[BaseDTVRateAddr + (i - 1) * 2];
			
			*GateV = Base + N * K / D;
			return TRUE;
		}
		else
			continue;
	}
	
	return FALSE;
}
// ----------------------------------------

Boolean SP_GetSetpointArray(Int16U VRate_x10, pInt16U RateRangeArray, pInt16U GateVArray)
{
	DataTable[REG_DIAG_CALC_FAILED_CELL] = 0;

	Int16U i, RateRange, GateV;
	for(i = 0; i < MAX_CELLS_COUNT; i++)
	{
		if((1 << i) & DataTable[REG_CELL_MASK])
		{
			if(SP_Generate(i, VRate_x10, &RateRange, &GateV))
			{
				if(RateRangeArray)
					RateRangeArray[i] = RateRange;

				if(GateVArray)
					GateVArray[i] = GateV;

				DataTable[REG_DIAG_GATEV_CELL1 + i] = GateV;
				DataTable[REG_DIAG_RANGE_CELL1 + i] = RateRange;
			}
			else
			{
				DataTable[REG_DIAG_CALC_FAILED_CELL] = i + 1;
				return FALSE;
			}
		}
	}

	return TRUE;
}
// ----------------------------------------
