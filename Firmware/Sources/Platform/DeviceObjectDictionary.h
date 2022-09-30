// ----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// ACTIONS
//
#define ACT_ENABLE_POWER			1	// Enable flyback converter
#define ACT_DISABLE_POWER			2	// Disable flyback converter
#define ACT_CLR_FAULT				3	// Clear fault (try switch state from FAULT to NONE)
#define ACT_CLR_WARNING				4	// Clear warning
//
#define ACT_APPLY_SETTINGS			10	// Apply settings to cells
//
#define ACT_START_TEST_CUSTOM		100	// Start test custom voltage rate
#define ACT_START_TEST_500			101	// Start test 500V/us
#define ACT_START_TEST_1000			102	// Start test 1000V/us
#define ACT_START_TEST_1600			103	// Start test 1600V/us
#define ACT_START_TEST_2000			104	// Start test 2000V/us
#define ACT_START_TEST_2500			105	// Start test 2500V/us
#define ACT_ENABLE_EXT_SYNC_START	106	// Enable external sync
#define ACT_DISABLE_EXT_SYNC_START	107	// Disable external sync
//
#define ACT_STOP					109	// Stop test
//
#define ACT_DIAG_PULSE_FAN			113
#define ACT_DIAG_PULSE_START		114
#define ACT_DIAG_PULSE_SWITCH		115
#define ACT_DIAG_PULSE_LED			116
#define ACT_DIAG_PULSE_SYNC			117
#define ACT_DIAG_READ_DETECTOR		118
#define ACT_DIAG_GENERATE_SETP		119
#define ACT_DIAG_READ_CELL_REG		120
#define ACT_DIAG_WRITE_CELL_REG		121
#define ACT_DIAG_CALL_CELL			122
//
#define ACT_SAVE_TO_ROM				200	// Save parameters to EEPROM module
#define ACT_RESTORE_FROM_ROM		201	// Restore parameters from EEPROM module
#define ACT_RESET_TO_DEFAULT		202	// Reset parameters to default values (only in controller memory)
//
#define ACT_BOOT_LOADER_REQUEST		320	// Request reboot to bootloader
//
// ----------------------------------------

// REGISTERS
#define REG_V_FINE_N				0	// Voltage amplitude fine (N)
#define REG_V_FINE_D				1	// Voltage amplitude fine (D)
#define REG_V_OFFSET				2	// Voltage offset (in V)
#define REG_CSU_V_OFFSET			3	// CSU Voltage Offset V
// 4 - 5
#define REG_RATE_GLOBAL_K_N			6	// Global rate correction (N)
#define REG_RATE_GLOBAL_K_D			7	// Global rate correction (D)

#define REG_CELL_MASK				8	// Cell presence mask
#define REG_CELL_MIN_VOLTAGE		9	// Cell min voltage (in V)
#define REG_CELL_MAX_VOLTAGE		10	// Cell max voltage (in V)
#define REG_CELL_MIN_RATE			11	// Cell min rate (in V/us x10)
#define REG_CELL_MAX_RATE			12	// Cell max rate (in V/us x10)

#define REG_FAN_OPERATE_PERIOD		13	// Default fan turn on period (in s)
#define REG_FAN_OPERATE_MIN_TIME	14	// Minimum fan turn on time (in s)

#define REG_UNIT_RATE_MIN			15	// Unit min rate (in V/us x10)
#define REG_UNIT_RATE_MAX			16	// Unit max rate (in V/us x10)

#define REG_UNIT_USE_RANGE_LOW		17	// Use setpoint data for range low
#define REG_UNIT_USE_RANGE_MID		18	// Use setpoint data for range mid
#define REG_UNIT_USE_RANGE_HIGH		19	// Use setpoint data for range high

// 20 - 39
#define REG_CELL1_HIGH_GATEV1		40	// Gate voltage setpoint 1 for cell 1
#define REG_CELL1_HIGH_VRATE1		41	// Voltage rate setpoint 1 for cell 1
// 42 - 121								// Setpoints for 6 cells (7 per cell)
#define REG_CELL6_HIGH_GATEV7		122	// Gate voltage setpoint 7 for cell 6
#define REG_CELL6_HIGH_VRATE7		123	// Voltage rate setpoint 7 for cell 6
//
// Extended NV-registers
// Range 1
#define REG_CELL1_LOW_GATEV1		320	// Gate voltage setpoint 1 for cell 1
#define REG_CELL1_LOW_VRATE1		321	// Voltage rate setpoint 1 for cell 1
// 322 - 400						// Setpoints for 6 cells (7 per cell)
#define REG_CELL6_LOW_GATEV7		401	// Gate voltage setpoint 7 for cell 6
#define REG_CELL6_LOW_VRATE7		402	// Voltage rate setpoint 7 for cell 6
//
// Range 2
#define REG_CELL1_MID_GATEV1		410	// Gate voltage setpoint 1 for cell 1
#define REG_CELL1_MID_VRATE1		411	// Voltage rate setpoint 1 for cell 1
// 412 - 490						// Setpoints for 6 cells (7 per cell)
#define REG_CELL6_MID_GATEV7		491	// Gate voltage setpoint 7 for cell 6
#define REG_CELL6_MID_VRATE7		492	// Voltage rate setpoint 7 for cell 6
//
// ----------------------------------------
//
#define REG_DESIRED_VOLTAGE			128	// Desired plate voltage (in V)
#define REG_VOLTAGE_RATE			129	// dV/dt rate (in V/us x10)
#define REG_TUNE_CUSTOM_SETTING		130	// Apply voltage rate correction for custom setting
// 131 - 184
#define REG_DIAG_TEST_CELL_ID		185
#define REG_DIAG_TEST_PARAM_1		186
#define REG_DIAG_TEST_PARAM_2		187
//
// ----------------------------------------
//
#define REG_DEV_STATE				192	// Device state
#define REG_FAULT_REASON			193	// Fault reason in the case DeviceState -> FAULT
#define REG_DISABLE_REASON			194	// Disbale reason in the case DeviceState -> DISABLE
#define REG_WARNING					195	// Warning if present
#define REG_PROBLEM					196	// Problem if present
//
#define REG_TEST_RESULT				197	// Test result
#define REG_FAULT_REASON_EX			199 // External fault code
//
#define REG_VOLTAGE_OK				200	// Charged summary
#define REG_VOLTAGE_OK_1			201	// Charged flag 1
#define REG_VOLTAGE_OK_2			202	// Charged flag 2
#define REG_VOLTAGE_OK_3			203	// Charged flag 3
#define REG_VOLTAGE_OK_4			204	// Charged flag 4
#define REG_VOLTAGE_OK_5			205	// Charged flag 5
#define REG_VOLTAGE_OK_6			206	// Charged flag 6
#define REG_ACTUAL_VOLTAGE_1		207	// Actual capacitor level 1
#define REG_ACTUAL_VOLTAGE_2		208	// Actual capacitor level 2
#define REG_ACTUAL_VOLTAGE_3		209	// Actual capacitor level 3
#define REG_ACTUAL_VOLTAGE_4		210	// Actual capacitor level 4
#define REG_ACTUAL_VOLTAGE_5		211	// Actual capacitor level 5
#define REG_ACTUAL_VOLTAGE_6		212	// Actual capacitor level 6
#define REG_CELL_STATE_1			213	// Cell state 1
#define REG_CELL_STATE_2			214	// Cell state 2
#define REG_CELL_STATE_3			215	// Cell state 3
#define REG_CELL_STATE_4			216	// Cell state 4
#define REG_CELL_STATE_5			217	// Cell state 5
#define REG_CELL_STATE_6			218	// Cell state 6
//
#define REG_CAN_BUSOFF_COUNTER		220 // Counter of bus-off states
#define REG_CAN_STATUS_REG			221	// CAN status register (32 bit)
#define REG_CAN_STATUS_REG_32		222
#define REG_CAN_DIAG_TEC			223	// CAN TEC
#define REG_CAN_DIAG_REC			224	// CAN REC
//
#define REG_DIAG_GENERAL_OUT		225
#define REG_DIAG_OPT_INTERFACE_ERR	226	// Error code for diagnostic optical interface communication
//
#define REG_DIAG_GATEV_CELL1		230	// Cell 1 gate voltage
#define REG_DIAG_GATEV_CELL2		231	// Cell 2 gate voltage
#define REG_DIAG_GATEV_CELL3		232	// Cell 3 gate voltage
#define REG_DIAG_GATEV_CELL4		233	// Cell 4 gate voltage
#define REG_DIAG_GATEV_CELL5		234	// Cell 5 gate voltage
#define REG_DIAG_GATEV_CELL6		235	// Cell 6 gate voltage
#define REG_DIAG_RANGE_CELL1		236	// Cell 1 range
#define REG_DIAG_RANGE_CELL2		237	// Cell 2 range
#define REG_DIAG_RANGE_CELL3		238	// Cell 3 range
#define REG_DIAG_RANGE_CELL4		239	// Cell 4 range
#define REG_DIAG_RANGE_CELL5		240	// Cell 5 range
#define REG_DIAG_RANGE_CELL6		241	// Cell 6 range
#define REG_DIAG_CALC_FAILED_CELL	242	// Ячейка, для которой зафейлился расчёт
//
// ----------------------------------------
//
#define REG_FWINFO_SLAVE_NID		256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID		257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN			260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN		261	// Begining of the information string record

// CELL ACTIONS
//
#define CELL_ACT_ENABLE_POWER		1	// Enable flyback converter
#define CELL_ACT_DISABLE_POWER		2	// Disable flyback converter
#define CELL_ACT_APPLY_PARAMS		10	// Apply params

// CELL REGISTERS
//
#define CELL_REG_DESIRED_VOLTAGE	1	// Desired cell voltage
#define CELL_REG_DESIRED_GATE_V		2 	// Desired gate voltage
//
#define	CELL_REG_RATE_RANGE			4	// Rate range selector
//
#define CELL_REG_DEV_STATE			10	// Device state
//
#define CELL_REG_VOLTAGE_OK			14	// Charged flag
#define CELL_REG_ACTUAL_VOLTAGE		15	// Actual capacitor level
//
// ----------------------------------------

// ENDPOINTS
//
#define EP16_Data_V					1	// Data V

// FAULT CODES
//
#define FAULT_NONE					0	// No fault
#define FAULT_LINK_CELL_1			601	// Link to cell 1 faulted
#define FAULT_LINK_CELL_2			602	// Link to cell 2 faulted
#define FAULT_LINK_CELL_3			603	// Link to cell 3 faulted
#define FAULT_LINK_CELL_4			604	// Link to cell 4 faulted
#define FAULT_LINK_CELL_5			605	// Link to cell 5 faulted
#define FAULT_LINK_CELL_6			606	// Link to cell 6 faulted
#define FAULT_NOT_READY_1			611	// Cell 1 is not charged
#define FAULT_NOT_READY_2			612	// Cell 2 is not charged
#define FAULT_NOT_READY_3			613	// Cell 3 is not charged
#define FAULT_NOT_READY_4			614	// Cell 4 is not charged
#define FAULT_NOT_READY_5			615	// Cell 5 is not charged
#define FAULT_NOT_READY_6			616	// Cell 6 is not charged

// OPERATION RESULTS
//
#define OPRESULT_NONE				0	// No information or not finished
#define OPRESULT_OK					1	// Operation was successful
#define OPRESULT_FAIL				2	// Operation failed

// WARNING CODES
//
#define WARNING_NONE				0
#define WARNING_WATCHDOG_RESET		1001	// System has been reseted by WD

// DISABLE CODES
//
#define DISABLE_NONE				0
#define DISABLE_BAD_CLOCK			1001	// Problem with main oscillator

// USER ERROR CODES
//
#define ERR_NONE					0	// No error
#define ERR_CONFIGURATION_LOCKED	1	// Device is locked for writing
#define ERR_OPERATION_BLOCKED		2	// Operation can't be done due to current device state
#define ERR_DEVICE_NOT_READY		3	// Device isn't ready to switch state
#define ERR_WRONG_PWD				4	// Wrong password - unlock failed
#define ERR_OUT_OF_RANGE			5	// Configured value is out of range

#endif // __DEV_OBJ_DIC_H
