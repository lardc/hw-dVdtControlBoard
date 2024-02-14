// -----------------------------------------
// Program entry point
// ----------------------------------------

// Include
#include <stdinc.h>
//
#include "ZwDSP.h"
#include "ZbBoard.h"
//
#include "SysConfig.h"
//
#include "Controller.h"


// FORWARD FUNCTIONS
// -----------------------------------------
Boolean InitializeCPU();
void InitializeTimers();
void InitializeCAN();
void InitializeSCI();
void InitializeSPI();
void InitializeSyncInput();
void InitializeBoard();
void InitializeController(Boolean GoodClock);
// -----------------------------------------


// FORWARD ISRs
// -----------------------------------------
// CPU Timer 0 ISR
ISRCALL Timer0_ISR();
// CPU Timer 1 ISR
ISRCALL Timer1_ISR();
// CPU Timer 2 ISR
ISRCALL Timer2_ISR();
// CAN Line 0 ISR
ISRCALL CAN0_ISR();
// SYNC Input ISR (PWM6 TZ)
ISRCALL TZ6_ISR();
// ILLEGAL ISR
ISRCALL IllegalInstruction_ISR();
// -----------------------------------------


// FUNCTIONS
// -----------------------------------------
void main()
{
	Boolean clockInitResult;

	// Boot process
	clockInitResult = InitializeCPU();

	// Only if good clocking was established
	if(clockInitResult)
	{
		InitializeTimers();
		InitializeCAN();
		InitializeSPI();
		InitializeBoard();
	}

	// Try initialize SCI in spite of result of clock initialization
	InitializeSCI();

	// Setup ISRs
	BEGIN_ISR_MAP
		ADD_ISR(TINT0, Timer0_ISR);
		ADD_ISR(TINT1_XINT13, Timer1_ISR);
		ADD_ISR(TINT2, Timer2_ISR);
		ADD_ISR(ECAN0INTA, CAN0_ISR);
		ADD_ISR(EPWM6_TZINT, TZ6_ISR);
	END_ISR_MAP

	// Initialize controller logic
	InitializeController(clockInitResult);

	// Only if good clocking was established
	if(clockInitResult)
	{
		// Enable interrupts
		EINT;
		ERTM;

		// Set watch-dog as WDRST
		ZwSystem_SelectDogFunc(FALSE);
		// Enable watch-dog
		ZwSystem_EnableDog(SYS_WD_PRESCALER);
		// Lock WD configuration
		ZwSystem_LockDog();

		// Start system ticks timer
		ZwTimer_StartT0();
	}

	DELAY_US(MSC_PON_DELAY_US);

	// Delayed initialization
	CONTROL_DelayedInit();

	// Start update cycle
	ZwTimer_StartT2();

	// Low-priority services
	while(TRUE)
		CONTROL_Idle();
}
// -----------------------------------------

Boolean InitializeCPU()
{
	Boolean clockInitResult;

	// Init clock and peripherals
	clockInitResult = ZwSystem_Init(CPU_PLL, CPU_CLKINDIV, SYS_LOSPCP, SYS_HISPCP, SYS_PUMOD);

	if(clockInitResult)
	{
		// Do default GPIO configuration
		ZwGPIO_Init(GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE, GPIO_TSAMPLE);

		// Initialize PIE and vectors
		ZwPIE_Init();
		ZwPIE_Prepare();
	}

	// Config flash
	ZW_FLASH_CODE_SHADOW;
	ZW_FLASH_OPTIMIZE(FLASH_FWAIT, FLASH_OTPWAIT);

	return clockInitResult;
}
// -----------------------------------------

void InitializeTimers()
{
	ZwTimer_InitT0();
	ZwTimer_SetT0(TIMER0_PERIOD);
	ZwTimer_EnableInterruptsT0(TRUE);

	ZwTimer_InitT1();
	ZwTimer_SetT1(PRE_PROBE_TIME_US);
	ZwTimer_EnableInterruptsT1(TRUE);

	ZwTimer_InitT2();
	ZwTimer_SetT2(TIMER2_PERIOD);
	ZwTimer_EnableInterruptsT2(TRUE);
}
// -----------------------------------------

void InitializeSyncInput()
{
	// Disable interrupts
	ZwPWM_EnableTZInterruptsGlobal(FALSE);

	// TZ GPIO
	ZwPWM_ConfigTZ6(TRUE, PQ_SysClock);

	// Dummy PWM config
	ZwPWM6_Init(PWMUp, CPU_FRQ, ZW_PWM_FREQUENCY, FALSE, FALSE, 0, BIT5, TRUE, TRUE, TRUE, FALSE, FALSE);

	// Configure TZ interrupts
	ZwPWM_ConfigTZIntCBC(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
	ZwPWM_ConfigTZIntOST(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE);

	// Enable interrupts
	ZwPWM_EnableTZInterrupts(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE);
}
// -----------------------------------------

void InitializeSCI()
{
	ZwSCIa_Init(SCIA_BR, SCIA_DB, SCIA_PARITY, SCIA_SB, FALSE);
	ZwSCIa_InitFIFO(16, 0);
	ZwSCIa_EnableInterrupts(FALSE, FALSE);

	ZwSCIb_Init(SCIB_BR, SCIB_DB, SCIB_PARITY, SCIB_SB, FALSE);
	ZwSCIb_InitFIFO(16, 0);
	ZwSCIb_EnableInterrupts(FALSE, FALSE);

	ZwSCI_EnableInterruptsGlobal(FALSE);
}
// -----------------------------------------

void InitializeSPI()
{
	// Common (ABCD)
	ZwSPI_EnableInterruptsGlobal(FALSE);
}
// -----------------------------------------

void InitializeCAN()
{
	// Init CAN
	ZwCANa_Init(CANA_BR, CANA_BRP, CANA_TSEG1, CANA_TSEG2, CANA_SJW);

	// Register system handler
	ZwCANa_RegisterSysEventHandler(&CONTROL_NotifyCANFault);

	// Allow interrupts for CAN
	ZwCANa_InitInterrupts(TRUE);
	ZwCANa_EnableInterrupts(TRUE);
}
// -----------------------------------------

void InitializeBoard()
{
	// Init watch-dog and on-board GPIO
	ZbWatchDog_Init();
	// Init DAC
	ZbMemory_Init();
	// Init on-board GPIO
	ZbGPIO_Init();
}
// -----------------------------------------

void InitializeController(Boolean GoodClock)
{
	InitializeSyncInput();

	// Init controllers and logic
	CONTROL_Init(!GoodClock);
}
// -----------------------------------------


// ISRs
// -----------------------------------------
#ifdef BOOT_FROM_FLASH
	#pragma CODE_SECTION(Timer0_ISR, "ramfuncs");
	#pragma CODE_SECTION(Timer2_ISR, "ramfuncs");
	#pragma CODE_SECTION(CAN0_ISR, "ramfuncs");
	#pragma CODE_SECTION(TZ6_ISR, "ramfuncs");
	#pragma CODE_SECTION(IllegalInstruction_ISR, "ramfuncs");
#endif
//
//#pragma INTERRUPT(Timer0_ISR, HPI);
#pragma INTERRUPT(TZ6_ISR, HPI);

// timer 0 ISR
ISRCALL Timer0_ISR(void)
{
	static Int16U LedCounter = 0;

	// Update time
	++CONTROL_TimeCounter;

	// Service watch-dogs
	if (CONTROL_BootLoaderRequest != BOOT_LOADER_REQUEST)
	{
		ZwSystem_ServiceDog();
		ZbWatchDog_Strobe();
	}

	CONTROL_HandleFanLogic(FALSE);
	CONTROL_HandleExtLed(FALSE);

	++LedCounter;
	if(LedCounter == DBG_COUNTER_PERIOD)
	{
		ZbGPIO_ToggleLED1();
		LedCounter = 0;
	}

	// allow other interrupts from group 1
	TIMER0_ISR_DONE;
}
// -----------------------------------------

// timer 1 ISR
ISRCALL Timer1_ISR(void)
{
	// Stop on-state voltage pulse
	CONTROL_ExtSyncFinish();
	// no PIE
	TIMER1_ISR_DONE;
}
// -----------------------------------------

// timer 2 ISR
ISRCALL Timer2_ISR(void)
{
	// Allow HPI interrupt
	EINT;

	// Invoke control routine
	CONTROL_Update();

	// no PIE
	TIMER2_ISR_DONE;
}
// -----------------------------------------

// Line 0 ISR
ISRCALL CAN0_ISR(void)
{
	// Allow HPI interrupt
	EINT;

	// handle CAN system events
	ZwCANa_DispatchSysEvent();
	// allow other interrupts from group 9
	CAN_ISR_DONE;
}
// -----------------------------------------

// EPWM6 TZ ISR
ISRCALL TZ6_ISR(void)
{
	CONTROL_ExtSyncEvent();
	// allow other interrupts from group 2
	PWM_TZ_ISR_DONE;
}
// -----------------------------------------

// ILLEGAL ISR
ISRCALL IllegalInstruction_ISR(void)
{
	GpioDataRegs.GPASET.bit.GPIO0 = 0;
	GpioDataRegs.GPASET.bit.GPIO1 = 0;
	GpioDataRegs.GPASET.bit.GPIO3 = 0;
	GpioDataRegs.GPASET.bit.GPIO4 = 0;
	GpioDataRegs.GPASET.bit.GPIO5 = 0;
	GpioDataRegs.GPASET.bit.GPIO6 = 0;
	GpioDataRegs.GPASET.bit.GPIO7 = 0;

	// Reset system using WD
	ZwSystem_ForceDog();
}
// -----------------------------------------

// No more.
