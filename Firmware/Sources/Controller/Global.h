// ----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H

// Include
#include "stdinc.h"

// Global parameters
//--------------------------------------------------------
#define EP_COUNT						0

// Cell ID mask
#define CELL1_MASK						BIT0
#define CELL2_MASK						BIT1
#define CELL3_MASK						BIT2
#define CELL4_MASK						BIT3
#define CELL5_MASK						BIT4
#define CELL6_MASK						BIT5

// Miscellaneous
#define MAX_CELLS_COUNT					6			// maximum cells 6
#define SCCI_CELL_NODE_ID				0			// cells have NodeID 0
#define	SCCI_TIMEOUT_TICKS_MS			500			// (in ms)

// Задержки в процессе тестирования (в мс)
#define TEST_PREPARE_TIMEOUT_MS			5000		// Таймаут настройки ячеек
#define TEST_PAUSE_LONG_MS				1000		// Пауза при отпирании прибора
#define TEST_PAUSE_SHORT_MS				500			// Пауза, если прибор остаётся запертым

// Voltage rate ranges
#define VRATE_RANGE_DEF					0			// Default rate range (500 - 2500 V/us)
#define VRATE_RANGE_LOWER1				1			// Lower range 1
#define VRATE_RANGE_LOWER2				2			// Lower range 2

#define RELAY_SWITCH_DELAY_US			5000		// (in us)
#define RELAY_SWITCH_DELAY_L_US			25000		// (in us)

#define PRE_PROBE_TIME_US				100			// (in us)

#define EXT_LED_SWITCH_ON_TIME			500		 	// (in ms)
//--------------------------------------------------------

#endif // __GLOBAL_H
