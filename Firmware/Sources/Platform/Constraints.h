// ----------------------------------------
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
#define X_D_DEF0					10
#define X_D_DEF1					100
#define X_D_DEF2					1000
#define X_D_DEF3					10000

// in V
#define DESIRED_VOLTAGE_MIN			100
#define DESIRED_VOLTAGE_MAX			2500
#define DESIRED_VOLTAGE_DEF			500

// Global voltage rate constraints rates (in V/us x10)
#define VOLTAGE_RATE_MIN			10
#define VOLTAGE_RATE_MAX			30000
#define VOLTAGE_RATE_DEF			5000

// Voltage rate setpoint constraints (in V/us x10)
#define VRATE_CELL_MIN				10
#define VRATE_CELL_MAX				20000
#define VRATE_CELL_DEF				100

// in mV
#define GATEV_CELL_MIN				1000
#define GATEV_CELL_MAX				5000
#define GATEV_CELL_DEF				3000

// in %
#define RATE_CORR_MIN				0
#define RATE_CORR_MAX				50
#define RATE_CORR_DEF				0
#define FAN_DUTY_MAX				100
#define FAN_DUTY_DEF				20

// Cell rate ranges (in V/us x10)
#define CELL_RATE_MIN				20
#define CELL_RATE_MAX				10000
#define CELL_RATE_DEF_MAX			5500
#define CELL_RATE_DEF_MIN			1000

// Unit rates (in V/us x10)
#define UNIT_RATE_MIN				10
#define UNIT_RATE_MAX				50000
#define UNIT_RATE_MIN_DEF			5000
#define UNIT_RATE_MAX_DEF			25000
#define UNIT_RATE_R1_DEF			500
#define UNIT_RATE_R2_DEF			5000

// Single cell number
#define CELL_NUM_MIN				1
#define CELL_NUM_MAX				6
#define CELL_NUM_DEF				1

// Single cell voltage limit
#define SINGLE_CELL_VLIM_MIN		20
#define SINGLE_CELL_VLIM_MAX		500
#define SINGLE_CELL_VLIM_DEF		300

// Cell vaoltage ranges
#define CELL_V_MIN					20		// in V
#define CELL_V_MAX					1000	// in V
#define CELL_V_DEF_MAX				900		// in V
#define CELL_V_DEF_MIN				50		// in V
#define FAN_TIME_MIN				10		// in s
#define FAN_TIME_MAX				600		// in s
#define FAN_TIME_DEF				30		// in s

// Variables
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];

#endif // __CONSTRAINTS_H
