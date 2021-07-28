// -----------------------------------------
// Constraints for tunable parameters
// ----------------------------------------

#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
//
#include "DataTable.h"
#include "Global.h"

// Types
//
typedef struct __TableItemConstraint
{
	Int16U Min;
	Int16U Max;
	Int16U Default;
} TableItemConstraint;


// Restrictions
//
#define X_D_DEF0					10
#define X_D_DEF1					100
#define X_D_DEF2					1000
#define X_D_DEF3					10000
//
#define DESIRED_VOLTAGE_MIN			500		// in V
#define DESIRED_VOLTAGE_MAX			4400	// in V
#define DESIRED_VOLTAGE_DEF			500		// in V
//
// Global voltage rate constraints
#define VOLTAGE_RATE_MIN			200		// in V/us
#define VOLTAGE_RATE_MAX			2500	// in V/us
#define VOLTAGE_RATE_DEF			500		// in V/us
//
// Voltage rate setpoint constraints
#define VRATE_CELL_MIN				5		// in V/us
#define VRATE_CELL_MAX				2000	// in V/us
#define VRATE_CELL_DEF				10		// in V/us
//
#define GATEV_CELL_MIN				2000	// in mV
#define GATEV_CELL_MAX				5000	// in mV
#define GATEV_CELL_DEF				3000	// in mV
//
#define RATE_CORR_MIN				0		// in %
#define RATE_CORR_MAX				90		// in %
#define RATE_CORR_DEF				0		// in %

#define FAN_DUTY_MAX				100		// in %
#define FAN_DUTY_DEF				20		// in %

// Cell rate ranges
#define CELL_RATE_MIN				20		// in V/us
#define CELL_RATE_MAX				1000	// in V/us
#define CELL_RATE_DEF_MAX			550		// in V/us
#define CELL_RATE_DEF_MIN			100		// in V/us

// Cell vaoltage ranges
#define CELL_V_MIN					20		// in V
#define CELL_V_MAX					1000	// in V
#define CELL_V_DEF_MAX				900		// in V
#define CELL_V_DEF_MIN				50		// in V

#define FAN_TIME_MIN				10		// in s
#define FAN_TIME_MAX				600		// in s
#define FAN_TIME_DEF				30		// in s

// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];


#endif // __CONSTRAINTS_H
