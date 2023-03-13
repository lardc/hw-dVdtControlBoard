// ----------------------------------------
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
void ZbGPIO_SwitchExtLed(Boolean Set);
// Out Relay
void ZbGPIO_SwitchOutRelay(Boolean Set);
// Toggle LED 1
void ZbGPIO_ToggleLED1();
// Toggle LED 2
void ZbGPIO_ToggleLED2();
// Set RESULT output
void ZbGPIO_SwitchResultOut(Boolean Set);
// Enable external sync
void ZbGPIO_SwitchSyncEn(Boolean Enable);
// Commutate interface to active cell
void ZbGPIO_SetActiveCell(Int16U CellNumber);
// Switch fan
void ZbGPIO_SwitchFAN(Boolean Set);
//Read detector pin
Boolean ZbGPIO_ReadDetectorPin();
//Enable internal sync
void ZbGPIO_SwitchStartPulse(Boolean Set);
//Read SYNC_IN pin
Boolean ZbGPIO_ReadSyncIn();

#endif // __ZBGPIO_H
