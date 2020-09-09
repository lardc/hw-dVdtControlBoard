// -----------------------------------------
// Board-specific GPIO functions
// ----------------------------------------

#ifndef __ZBGPIO_H
#define __ZBGPIO_H

// Include
#include "stdinc.h"
#include "ZwDSP.h"

// Functions
//
// Init GPIO
void ZbGPIO_Init();
// Switch LED 1
void ZbGPIO_SwitchLED1(Boolean Set);
// Switch LED 2
void ZbGPIO_SwitchLED2(Boolean Set);
// External LED
void ZbGPIO_SwitchLEDExt(Boolean Set);
// Toggle LED 1
void ZbGPIO_ToggleLED1();
// Toggle LED 2
void ZbGPIO_ToggleLED2();
// Set Meanwell mode
void ZbGPIO_SwitchMeanwell(Boolean Enable);
// Set RESULT output
void ZbGPIO_SwitchResultOut(Boolean Set);
// Enable external sync
void ZbGPIO_SwitchSyncEn(Boolean Enable);
// Commutate interface to active cell
void ZbGPIO_SetActiveCell(Int16U CellNumber);
// Switch relays
void ZbGPIO_RelayLine(Boolean Out);

// Inline functions
//
Boolean inline ZbGPIO_ReadSync()
{
	return ZwGPIO_ReadPin(PIN_SYNC_IN);
}
//
Boolean inline ZbGPIO_ReadDetector()
{
	return !ZwGPIO_ReadPin(PIN_DETECTOR);
}
//
void inline ZbGPIO_SwitchStartPulse(Boolean Set)
{
	ZwGPIO_WritePin(PIN_START_PULSE, Set);
}
//
void inline ZbGPIO_SwitchDebugPin(Boolean Set)
{
	ZwGPIO_WritePin(PIN_DEBUG, Set);
}
//
void inline ZbGPIO_ToggleDebugPin()
{
	ZwGPIO_TogglePin(PIN_DEBUG);
}

#endif // __ZBGPIO_H
