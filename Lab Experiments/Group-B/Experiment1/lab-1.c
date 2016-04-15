/*

* Author: Texas Instruments 
* Editted by: Saurav Shandilya, Vishwanathan Iyer 
	      ERTS Lab, CSE Department, IIT Bombay
* Description: This code will familiarize you with using GPIO on TMS4C123GXL microcontroller.

* Filename: lab-1.c
* Functions: setup(), ledPinConfig(), switchPinConfig(), main()
* Global Variables: sw2Status, led
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))

/*
 ------ Global Variable Declaration
*/

int sw2Status = 0;
uint8_t led;

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
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  // Pin-1,2,3 of PORT F set as output.
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
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0 -- leave this unchanged
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;

	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input.
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);

	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_DIR_MODE_IN); // Set Pin-0 of PORT F as Input.
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}

int main(void)
{
	// Perform the initial setup and pin configuration.
	setup();
	ledPinConfig();
	switchPinConfig();

	bool pressed;
	bool pressed2;
	led = 2;

	// Keep checking periodically if switch-1 and switch-2 are pressed.
	// And correspondingly change the LED in use and increment the 'sw2status' counter.
	while(1) {
		SysCtlDelay(5000);
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) {
			// Switch 1 is pressed.
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, led);
			if (!pressed) {
				led = led*2;
				if (led == 16) led = 2;
				pressed = true;
			}
		}
		else {
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
			pressed = false;
		}
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) {
			// Switch 2 is pressed.
			if (!pressed2) {
				sw2Status = sw2Status + 1;
				pressed2 = true;
			}
		}
		else {
			pressed2 = false;
		}
	}
}
