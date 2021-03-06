// -----------------------------------------
// dVdt test routine
// ----------------------------------------

#ifndef __LOGIC_H
#define __LOGIC_H

// Include
#include "stdinc.h"


// Constants
typedef enum __LogicState
{
	LS_None			= 0,
	LS_Measure		= 1,
	LS_Pause		= 2
} LogicState;


// Functions
//
// Execute test
void LOGIC_BeginTest(Int64U TimerTicks);
// Process
void LOGIC_Update(Int64U TimerTicks);
// Reset process
void LOGIC_Reset();
// Get fault information
Int16U LOGIC_GetFaultReason(pInt16U pErrorCodeEx);


#endif // __LOGIC_H
