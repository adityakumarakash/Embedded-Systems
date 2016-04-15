#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"


#define PWM_FREQUENCY 55

/* Global variables for storing the speed of LED colour change and relative intensities. */
uint32_t speed = 1;
uint8_t rg = 1;
uint8_t gb = 0;
uint8_t br = 0;

/* Global variables related to PWM and RGB intensities. */
volatile uint32_t ui32Load;
volatile uint32_t ui32PWMClock;
volatile uint32_t ui8Adjust1;
volatile uint32_t ui8Adjust2;
volatile uint32_t ui8Adjust3;

/* Mode of operation of the circuit. */
uint32_t mode = 0;

/* State variables for knowing the switch combination pressed. */
uint32_t sw1_presscount = 0, sw2_presscount = 0;
uint32_t sw1_longpresscount = 0, sw2_longpresscount = 0;
uint8_t sw1_prevpressed = 0, sw2_prevpressed = 0;

/* Function to increase intensity of Red LED */
void incR(){
	ui8Adjust1++;
	if (ui8Adjust1 > 254) ui8Adjust1 = 254;
}

/* Function to decrease intensity of Red LED */
void decR() {
	ui8Adjust1--;
	if (ui8Adjust1 < 10) ui8Adjust1 = 10;
}

/* Function to increase intensity of Green LED */
void incG(){
	ui8Adjust2++;
	if (ui8Adjust2 > 254) ui8Adjust2 = 254;
}

/* Function to decrease intensity of Green LED */
void decG() {
	ui8Adjust2--;
	if (ui8Adjust2 < 10) ui8Adjust2 = 10;
}

/* Function to increase intensity of Blue LED */
void incB(){
	ui8Adjust3++;
	if (ui8Adjust3 > 254) ui8Adjust3 = 254;
}

/* Function to decrease intensity of Blue LED */
void decB() {
	ui8Adjust3--;
	if (ui8Adjust3 < 10) ui8Adjust3 = 10;
}

int main(void)
{
	// Initial RGB intensities.
	ui8Adjust1 = 254;
	ui8Adjust2 = 10;
	ui8Adjust3 = 10;

	// System Clock and Peripheral setup.
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// PWM configuration.
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);


	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, ui32Load);

	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);

	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);


	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust1 * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_1);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjust2 * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjust3 * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

	// Code to continually change the LED intensities based on the current state.
	// The current state is determined by the previous state and the key combination pressed.
	while(1)
	{
		if (mode == 0) {
			if (rg) {
				ui8Adjust1 = ui8Adjust1 - speed/10;
				ui8Adjust2 = ui8Adjust2 + speed/10;
			} else if (gb) {
				ui8Adjust2 = ui8Adjust2 - speed/10;
				ui8Adjust3 = ui8Adjust3 + speed/10;
			} else if (br) {
				ui8Adjust3 = ui8Adjust3 - speed/10;
				ui8Adjust1 = ui8Adjust1 + speed/10;
			} else {}
			if (ui8Adjust1 < 10)
			{
				ui8Adjust1 = 10;
			}
			if (ui8Adjust2 < 10)
			{
				ui8Adjust2 = 10;
			}
			if (ui8Adjust3 < 10)
			{
				ui8Adjust3 = 10;
			}
			if (ui8Adjust1 > 254)
			{
				ui8Adjust1 = 254;
				br = 0;
				rg = 1;
			}
			if (ui8Adjust2 > 254)
			{
				ui8Adjust2 = 254;
				rg = 0;
				gb = 1;
			}
			if (ui8Adjust3 > 254)
			{
				ui8Adjust3 = 254;
				gb = 0;
				br = 1;
			}
		}

		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
		{
			if (mode == 0){
				speed++;
				if (speed > 100) speed = 100;
			}
			sw2_longpresscount++;
			if (sw2_longpresscount <= 2 || sw2_longpresscount > 30){
				if (mode == 1) incR();
				if (mode == 2) incG();
				if (mode == 3) incB();
			}
		}
		else {
			if (sw1_presscount == 1 && sw2_longpresscount > 20) mode = 1;
			sw2_presscount = 0;
			sw1_presscount = 0;
			sw2_longpresscount = 0;
			sw1_longpresscount = 0;
		}
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
		{
			if (mode == 0){
				speed--;
				if (speed < 1) speed = 1;
			}
			sw1_longpresscount++;
			if (sw1_longpresscount > 20) mode = 3;
			if (mode == 1) decR();
			if (mode == 2) decG();
			if (mode == 3) decB();
		}
		else {
			if (sw1_longpresscount <= 20 && sw1_longpresscount > 0)sw1_presscount++;
			sw1_longpresscount = 0;
			if (sw1_presscount == 2) mode = 2;
		}




		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust1 * ui32Load / 1000);
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjust3 * ui32Load / 1000);
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjust2 * ui32Load / 1000);

		SysCtlDelay(200000);
	}

}

