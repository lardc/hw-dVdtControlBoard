// -----------------------------------------
// Board parameters
// ----------------------------------------

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

// Include
#include <ZwBase.h>

// Program build mode
//
#define BOOT_FROM_FLASH						// normal mode
#define RAM_CACHE_SPI_X						// cache SPI-A(BCD) functions

// Board options
#define DSP28_2809							// on-board CPU
#define OSC_FRQ				(20MHz)			// on-board oscillator
#define CPU_FRQ_MHZ			100				// CPU frequency = 100MHz
#define CPU_FRQ				(CPU_FRQ_MHZ * 1000000L)
#define SYS_HSP_FREQ		(CPU_FRQ / 2) 	// High-speed bus frequency
#define SYS_LSP_FREQ		(CPU_FRQ / 4) 	// Low-speed bus frequency
//
#define ZW_PWM_FREQUENCY	2000
#define ZW_PWM_DUTY_BASE	(CPU_FRQ / ZW_PWM_FREQUENCY)

// Peripheral options
#define HWUSE_SPI_B
#define HWUSE_SCI_A
#define HWUSE_SCI_B

// IO placement
//
#define SPI_B_QSEL			GPAQSEL1
#define SPI_B_MUX			GPAMUX1
#define SPI_B_SIMO			GPIO12
#define SPI_B_SOMI			GPIO13
#define SPI_B_CLK			GPIO14
#define SPI_B_CS			GPIO15
//
#define SPI_B_QSEL_V		GPAQSEL2
#define SPI_B_MUX_V			GPAMUX2
#define SPI_B_SOMI_V		GPIO25
//
#define SCI_A_QSEL			GPAQSEL2
#define SCI_A_MUX			GPAMUX2
#define SCI_A_TX			GPIO29
#define SCI_A_RX			GPIO28
#define SCI_A_MUX_SELECTOR	1
//
#define SCI_B_QSEL			GPAQSEL2
#define SCI_B_MUX			GPAMUX2
#define SCI_B_TX			GPIO22
#define SCI_B_RX			GPIO23
#define SCI_B_MUX_SELECTOR	3
//
#define PIN_START_PULSE		0
#define PIN_SYNC_EN			1
#define PIN_SYNC_IN			17
#define PIN_LED_1			8
#define PIN_LED_2			9
#define PIN_DEBUG			10
#define PIN_WD_RST			2
#define PIN_OPT_V1V2		6
#define PIN_OPT_V3V4		5
#define PIN_OPT_V5V6		4
#define PIN_OPT_V7V8		3
#define PIN_MEANWELL_EN		7
#define PIN_DETECTOR		16
#define PIN_RESULT_OUT		18
#define PIN_CELL_IND_1		13
#define PIN_CELL_IND_2		26
#define PIN_CELL_IND_3		32
//
#define PIN_EMPTY_1			11
#define PIN_EMPTY_2			19
#define PIN_EMPTY_3			20
#define PIN_EMPTY_4			21
#define PIN_EMPTY_5			24
#define PIN_EMPTY_6			27
#define PIN_EMPTY_7			33

#endif // __BOARD_CONFIG_H
