﻿// ----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H

// Include
#include "stdinc.h"

// Global parameters
//--------------------------------------------------------
// Password to unlock non-volatile area for write
#define ENABLE_LOCKING					FALSE
#define UNLOCK_PWD_1					1
#define UNLOCK_PWD_2					1
#define UNLOCK_PWD_3					1
#define UNLOCK_PWD_4					1
//
#define EP_COUNT						1
#define VALUES_x_SIZE					500

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
#define TEST_APPLY_FIXED_MS             9000         // Apply parameters fixed delay (in ms)
#define TEST_PREPARE_TIMEOUT_MS			5000		// (in ms)
#define TEST_PAUSE_LONG_MS				1000		// Long pause in case of triggered device (in ms)
#define TEST_PAUSE_SHORT_MS				500			// Short pause between measurements (in ms)

// Voltage rate ranges
#define VRATE_RANGE_DEF					0			// Default rate range (500 - 2500 V/us)
#define VRATE_RANGE_LOWER1				1			// Lower range 1
#define VRATE_RANGE_LOWER2				2			// Lower range 2

#define MEANWELL_SWITCH_DELAY_US		500000L		// (in us)
#define RELAY_SWITCH_DELAY_US			5000		// (in us)
#define RELAY_SWITCH_DELAY_L_US			25000		// (in us)
#define INFO_UPDATE_DELAY_US			500			// (in us)
//
#define PRE_PROBE_TIME_US				250			// (in us)
#define PRE_PROBE_TIME_DIAG_US			250			// (in us)

// Correction
#define CORR_RATE_TUNE_LOW				TRUE		// Pull-up rate at lower rates
#define CORR_RATE_BY_FS_VOLTAGE			FALSE		// Tune rate by rate depending on full scale voltage

#define INVERT_SYNC						FALSE		// Invert SYNC pin

#define TEST_RESULT_NULL                0           //
#define TEST_RESULT_OK                  1           //
#define TEST_RESULT_FAIL                2           //

#define CSU_VOLTAGE_OFFSET              100         // Voltage offset for CSU

#define EXT_LED_SWITCH_ON_TIME          500         // (in ms)

#define MIN_RATE_FOR_DUO_MODE           6600        // (В/мкс)*10

//--------------------------------------------------------

#endif // __GLOBAL_H
