#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))

/*
* Function Name: setup()
* Input: none
* Output: none
* Description: Set crystal frequency and enable GPIO Peripherals  
* Example Call: setup();
*/
void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}

/*
* Function Name: ledPinConfig()
* Input: none
* Output: none
* Description: Set PORTF Pin 1, Pin 2, Pin 3 as output.
* Example Call: ledPinConfig();
*/
void ledPinConfig(void)
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|  GPIO_PIN_2 | GPIO_PIN_3);  // Pin-1,2,3 of PORT F set as output.
}

/*
* Function Name: switchPinConfig()
* Input: none
* Output: none
* Description: Set PORTF Pin 0 and Pin 4 as input. Note that Pin 0 is locked.
* Example Call: switchPinConfig();
*/
void switchPinConfig(void)
{
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;

	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4 | GPIO_PIN_0,GPIO_DIR_MODE_IN); // Set Pin-4,0 of PORT F as Input.
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4 | GPIO_PIN_0,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}

/* Some global variables to maintain the state of switches and LEDs. */
int32_t state1=1;
int32_t state2=1;
uint8_t ui8LED = 2;
int32_t sw2status = 0;

int main(void)
{
	setup();
	ledPinConfig();
	switchPinConfig();
	uint32_t ui32Period;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ui32Period = (SysCtlClockGet() / 100);
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);
	while(1) {}
}

/*
* Function Name: detectKeyPress
* Input : int
* Output : int
* Description : Returns when a key is pressed (after debouncing) using system's state.
*/
int detectKeyPress(int32_t sw)
{
	int switch0 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
	int switch1 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
	if (sw == 1){
		if (state1 == 1) {
			if(!switch0){
				state1=2;
			}
			return 0;
		}
		else if (state1 == 2) {
			if(!switch0){
				state1=3;
				return 1;
			}
			else{
				return 0;
			}
		}
		else if (state1 == 3) {
			if(!switch0){
				return 0;
			}
			else{
				state1=1;
				return 0;
			}
		}
	}
	else if (sw == 2) {
		if (state2 == 1) {
			if(!switch1){
				state2=2;
			}
			return 0;
		}
		else if (state2 == 2) {
			if(!switch1){
				state2=3;
				return 1;
			}
			else{
				return 0;
			}
		}
		else if (state2 == 3) {
			if(!switch1){
				return 0;
			}
			else{
				state2=1;
				return 0;
			}
		}
	}
	return 0;
}

/*
* Function Name: Timer0IntHandler
* Input : None
* Output : None
* Description : Interrupt handling function for Timer-0 timer interrupt.
*/
void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Read the current state of the GPIO pin for switch-1 and write back the opposite state.
	int keypress = detectKeyPress(1);
	if(keypress == 1){
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
		if (ui8LED == 8)
		{
			ui8LED = 2;
		}
		else
		{
			ui8LED = ui8LED*2;
		}
	}
	// Read the current state of the GPIO pin for switch-2 and increment sw2status counter.
	keypress = detectKeyPress(2);
	sw2status = sw2status + keypress;
}
