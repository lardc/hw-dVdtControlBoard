// ----------------------------------------
// Gate voltage setpoint generator
// ----------------------------------------

#ifndef __SETPOINT_H
#define __SETPOINT_H

// Include
#include "stdinc.h"

// Functions
//
// Return required gate voltage
Int16U SP_Generate(Int16U CellIndex, Int16U VRate, pInt16U RateRange);
Int16U SP_GetRange1MaxRate();
Int16U SP_GetRange2MaxRate();

#endif // __SETPOINT_H
