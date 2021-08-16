// ----------------------------------------
// Board-specific GPIO functions
// ----------------------------------------

// Header
#include "ZbGPIO.h"
//
#include "SysConfig.h"
#include "Global.h"


// Constants
//
static const Int16U EmptyPins[] = { PIN_EMPTY_1, PIN_EMPTY_2, PIN_EMPTY_3, PIN_EMPTY_4, PIN_EMPTY_5, PIN_EMPTY_6, PIN_EMPTY_7 };


// Functions
//
void ZbGPIO_Init()
{
    Int16U i;

    // Hold down empty DSC pins
    for(i = 0; i < sizeof(EmptyPins) / sizeof(Int16U); ++i)
    {
        ZwGPIO_WritePin(EmptyPins[i], FALSE);
        ZwGPIO_PinToOutput(EmptyPins[i]);
    }

    // Configure input pin
    ZwGPIO_PinToInput(PIN_SYNC_IN, FALSE, GPIO_NSAMPLE);
    ZwGPIO_PinToInput(PIN_DETECTOR, FALSE, GPIO_NSAMPLE);

    // Reset to default state
    ZwGPIO_WritePin(PIN_LED_1, FALSE);
    ZwGPIO_WritePin(PIN_LED_2, FALSE);
    ZwGPIO_WritePin(PIN_DEBUG, FALSE);
    ZwGPIO_WritePin(PIN_RESULT_OUT, FALSE);
    ZwGPIO_WritePin(PIN_START_PULSE, FALSE);
    ZwGPIO_WritePin(PIN_SYNC_EN, TRUE);
    ZwGPIO_WritePin(PIN_OPT_V1V2, FALSE);
    ZwGPIO_WritePin(PIN_OPT_V3V4, FALSE);
    ZwGPIO_WritePin(PIN_OPT_V5V6, FALSE);
    ZwGPIO_WritePin(PIN_OPT_V7V8, FALSE);
    ZwGPIO_WritePin(PIN_CELL_IND_1, FALSE);
    ZwGPIO_WritePin(PIN_CELL_IND_2, FALSE);
    ZwGPIO_WritePin(PIN_CELL_IND_3, FALSE);
    ZwGPIO_WritePin(PIN_MEANWELL_EN, FALSE);
    // Configure pins
    ZwGPIO_PinToOutput(PIN_LED_1);
    ZwGPIO_PinToOutput(PIN_LED_2);
    ZwGPIO_PinToOutput(PIN_DEBUG);
    ZwGPIO_PinToOutput(PIN_RESULT_OUT);
    ZwGPIO_PinToOutput(PIN_START_PULSE);
    ZwGPIO_PinToOutput(PIN_SYNC_EN);
    ZwGPIO_PinToOutput(PIN_OPT_V1V2);
    ZwGPIO_PinToOutput(PIN_OPT_V3V4);
    ZwGPIO_PinToOutput(PIN_OPT_V5V6);
    ZwGPIO_PinToOutput(PIN_OPT_V7V8);
    ZwGPIO_PinToOutput(PIN_CELL_IND_1);
    ZwGPIO_PinToOutput(PIN_CELL_IND_2);
    ZwGPIO_PinToOutput(PIN_CELL_IND_3);
    ZwGPIO_PinToOutput(PIN_MEANWELL_EN);
}
// ----------------------------------------

void ZbGPIO_SwitchFAN(Boolean Set)
{
    ZwGPIO_WritePin(PIN_OPT_V1V2, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchLED1(Boolean Set)
{
    ZwGPIO_WritePin(PIN_LED_1, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchLED2(Boolean Set)
{
    ZwGPIO_WritePin(PIN_LED_2, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchOutRelay(Boolean Set)
{
    ZwGPIO_WritePin(PIN_OPT_V7V8, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchExtLed(Boolean Set)
{
    ZwGPIO_WritePin(PIN_OPT_V5V6, Set);
}
// ----------------------------------------

void ZbGPIO_ToggleLED1()
{
    ZwGPIO_TogglePin(PIN_LED_1);
}
// ----------------------------------------

void ZbGPIO_ToggleLED2()
{
    ZwGPIO_TogglePin(PIN_LED_2);
}
// ----------------------------------------

void ZbGPIO_SwitchMeanwell(Boolean Enable)
{
    ZwGPIO_WritePin(PIN_MEANWELL_EN, Enable);
}
// ----------------------------------------

void ZbGPIO_SwitchResultOut(Boolean Set)
{
    ZwGPIO_WritePin(PIN_RESULT_OUT, Set);
}
// ----------------------------------------

void ZbGPIO_SwitchSyncEn(Boolean Enable)
{
    ZwGPIO_WritePin(PIN_SYNC_EN, !Enable);
}
// ----------------------------------------

void ZbGPIO_RelayLine(Boolean Out)
{
    ZwGPIO_WritePin(PIN_OPT_V3V4, Out);
    ZwGPIO_WritePin(PIN_OPT_V5V6, Out);
}
// ----------------------------------------

void ZbGPIO_SetActiveCell(Int16U CellNumber)
{
    ZwGPIO_WritePin(PIN_CELL_IND_1, CellNumber & 0x01);
    ZwGPIO_WritePin(PIN_CELL_IND_2, (CellNumber >> 1) & 0x01);
    ZwGPIO_WritePin(PIN_CELL_IND_3, (CellNumber >> 2) & 0x01);

    DELAY_US(1);
}
// ----------------------------------------

Boolean ZbGPIO_ReadDetectorPin()
{
    return (!ZwGPIO_ReadPin(PIN_DETECTOR));
}
// ----------------------------------------

void ZbGPIO_SwitchStartPulse(Boolean Set)
{
    ZwGPIO_WritePin(PIN_START_PULSE, Set);
}
// ----------------------------------------


// No more.
