// -----------------------------------------
// Constraints for tunable parameters
// ----------------------------------------

// Header
#include "Constraints.h"
#include "DeviceObjectDictionary.h"

#define NO		0	// equal to FALSE
#define YES		1	// equal to TRUE

// Constants
//
const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE] =
                                       {
                                    		   {0, INT16U_MAX, X_D_DEF2},												// 0
                                    		   {1, X_D_DEF3, X_D_DEF2},													// 1
                                    		   {0, INT16U_MAX, 0},														// 2
                                    		   {RATE_CORR_MIN, RATE_CORR_MAX, RATE_CORR_DEF},							// 3
                                    		   {DESIRED_VOLTAGE_MIN, DESIRED_VOLTAGE_MAX, DESIRED_VOLTAGE_MIN},			// 4
											   {RATE_CORR_MIN, RATE_CORR_MAX, RATE_CORR_DEF},							// 5
											   {0, INT16U_MAX, X_D_DEF2},												// 6
											   {1, X_D_DEF3, X_D_DEF2},													// 7
                                    		   {0, 0x3F, 0},															// 8
                                    		   {CELL_V_MIN, CELL_V_MAX, CELL_V_DEF_MIN},								// 9
                                    		   {CELL_V_MIN, CELL_V_MAX, CELL_V_DEF_MAX},								// 10
											   {CELL_RATE_MIN, CELL_RATE_MAX, CELL_RATE_DEF_MIN},						// 11
											   {CELL_RATE_MIN, CELL_RATE_MAX, CELL_RATE_DEF_MAX},						// 12
											   {FAN_TIME_MIN, FAN_TIME_MAX, FAN_TIME_DEF},								// 13
											   {FAN_TIME_MIN, FAN_TIME_MAX, FAN_TIME_DEF},								// 14
											   {0, INT16U_MAX, 0},														// 15
											   {0, 0, 0},																// 16
											   {0, 0, 0},																// 17
											   {0, 0, 0},																// 18
											   {0, 0, 0},																// 19
											   {0, 0, 0},																// 20
											   {0, 0, 0},																// 21
											   {0, 0, 0},																// 22
											   {0, 0, 0},																// 23
											   {0, 0, 0},																// 24
											   {0, 0, 0},																// 25
											   {0, 0, 0},																// 26
											   {0, 0, 0},																// 27
											   {0, 0, 0},																// 28
											   {0, 0, 0},																// 29
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 30
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 31
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 32
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 33
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 34
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 35
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 36
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 37
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 38
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 39
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 40
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 41
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 42
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 43
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 44
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 45
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 46
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 47
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 48
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 49
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 50
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 51
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 52
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 53
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 54
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 55
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 56
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 57
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 58
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 59
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 60
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 61
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 62
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 63
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 64
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 65
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 66
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 67
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 68
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 69
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 70
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 71
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 72
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 73
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 74
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 75
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 76
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 77
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 78
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 79
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 80
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 81
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 82
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 83
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 84
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 85
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 86
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 87
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 88
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 89
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 90
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 91
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 92
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 93
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 94
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 95
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 96
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 97
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 98
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 99
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 100
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 101
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 102
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 103
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 104
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 105
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 106
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 107
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 108
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 109
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 110
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 111
                                    		   {GATEV_CELL_MIN, GATEV_CELL_MAX, GATEV_CELL_DEF},						// 112
                                    		   {VRATE_CELL_MIN, VRATE_CELL_MAX, VRATE_CELL_DEF},						// 113
											   {0, 0, 0},																// 114
											   {0, 0, 0},																// 115
											   {0, 0, 0},																// 116
											   {0, 0, 0},																// 117
											   {0, 0, 0},																// 118
											   {0, 0, 0},																// 119
                                    		   {0, 0, 0},																// 120
                                    		   {0, 0, 0},																// 121
                                    		   {0, 0, 0},																// 122
                                    		   {0, 0, 0},																// 123
                                    		   {0, 0, 0},																// 124
                                    		   {0, 0, 0},																// 125
                                    		   {0, 0, 0},																// 126
                                    		   {0, 0, 0}																// 127
                                       };

const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START] =
                                      {
											   {DESIRED_VOLTAGE_MIN, DESIRED_VOLTAGE_MAX, DESIRED_VOLTAGE_DEF},			// 128
											   {VOLTAGE_RATE_MIN, VOLTAGE_RATE_MAX, VOLTAGE_RATE_DEF},					// 129
											   {NO, YES, YES},															// 130
											   {0, 0, 0},																// 131
											   {0, 0, 0},																// 132
											   {0, 0, 0},																// 133
											   {0, 0, 0},																// 134
											   {0, 0, 0},																// 135
											   {0, 0, 0},																// 136
											   {0, 0, 0},																// 137
											   {0, 0, 0},																// 138
											   {0, 0, 0},																// 139
                                    		   {0, 0, 0},																// 140
                                    		   {0, 0, 0},																// 141
                                    		   {0, 0, 0},																// 142
                                    		   {0, 0, 0},																// 143
                                    		   {0, 0, 0},																// 144
                                    		   {0, 0, 0},																// 145
                                    		   {0, 0, 0},																// 146
                                    		   {0, 0, 0},																// 147
											   {0, 0, 0},																// 148
											   {0, 0, 0},																// 149
											   {0, 0, 0},																// 150
											   {0, 0, 0},																// 151
											   {0, 0, 0},																// 152
											   {0, 0, 0},																// 153
											   {0, 0, 0},																// 154
											   {0, 0, 0},																// 155
											   {0, 0, 0},																// 156
											   {0, 0, 0},																// 157
											   {0, 0, 0},																// 158
											   {0, 0, 0},																// 159
											   {0, 0, 0},																// 160
											   {0, 0, 0},																// 161
											   {0, 0, 0},																// 162
											   {0, 0, 0},																// 163
											   {0, 0, 0},																// 164
											   {0, 0, 0},																// 165
											   {0, 0, 0},																// 166
											   {0, 0, 0},																// 167
											   {0, 0, 0},																// 168
											   {0, 0, 0},																// 169
											   {0, 0, 0},																// 170
											   {0, 0, 0},																// 171
											   {0, 0, 0},																// 172
											   {0, 0, 0},																// 173
											   {0, 0, 0},																// 174
											   {0, 0, 0},																// 175
											   {0, 0, 0},																// 176
											   {0, 0, 0},																// 177
											   {0, 0, 0},																// 178
											   {0, 0, 0},																// 179
											   {0, 0, 0},																// 180
											   {0, 0, 0},																// 181
											   {0, 0, 0},																// 182
											   {0, 0, 0},																// 183
											   {0, 0, 0},																// 184
											   {1, MAX_CELLS_COUNT, 0},													// 185
											   {0, INT16U_MAX, 0},														// 186
											   {0, INT16U_MAX, 0},														// 187
											   {0, 0, 0},																// 188
											   {0, 0, 0},																// 189
											   {0, 0, 0},																// 190
											   {0, 0, 0}																// 191
                                      };

// No more
