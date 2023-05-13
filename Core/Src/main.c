//********************************************************************
//*                    EEE2046F C template                           *
//*==================================================================*
//* WRITTEN BY: Jesse Arendse   	                 		               *
//* DATE CREATED: 07/04/2023                                         *
//* MODIFIED:                                                        *
//*==================================================================*
//* PROGRAMMED IN: Visual Studio Code                                *
//* TARGET:        STM32F0                                           *
//*==================================================================*
//* DESCRIPTION:                                                     *
//*                                                                  *
//********************************************************************
// INCLUDE FILES
//====================================================================
#include <lcd_stm32f0.c>
#include "math.h"
#include "stm32f0xx.h"
//====================================================================
// GLOBAL CONSTANTS
//====================================================================

//====================================================================
// GLOBAL VARIABLES
//====================================================================

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void init_timers(void);
void TIM14_IRQHandler(void);
int get_msec(int tim);
int get_secs(int tim);
int get_mins(int tim);
int get_hrs(int tim);
//====================================================================
// MAIN FUNCTION
//====================================================================
int main(void) {
	init_LCD();
	lcd_command(CLEAR);
	lcd_putstring("Hello");
	init_timers();
	double tim = 0.0f;
	char buff[10];
	while (1) {
		while (!(TIM14->SR & TIM_SR_UIF)) {
		};
		TIM14->SR &= ~TIM_SR_UIF;
		tim += 0.5f;  
		if (!(((int) tim )% 100)) {
			sprintf(buff, "%f", tim);
			lcd_command(CLEAR);
			lcd_putstring(buff);
		}
		// delay(10000);
	}

}  // End of main

//====================================================================
// FUNCTION DEFINITIONS
//====================================================================
void init_timers(void) {
	float cyc_needed = 48;
	int presc = ceil(cyc_needed / (65536));
	int arr = floor(cyc_needed / presc) - 1;
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
	TIM14->PSC |= presc;
	TIM14->ARR |= arr;
	TIM14->DIER |= (1);
	TIM14->CR1 |= TIM_CR1_CEN;
}

int get_ms(int tim) {
	return (tim / 100) % 100;
}

int get_secs(int tim) {
	return (tim / 100000) % 60;
}

int get_mins(int tim) {
	return (tim / 6000000) % 60;
}

void get_time(int tim, char* buffer) {
	sprintf(buffer, "%02d:%02d.%02d", get_mins(tim), get_secs(tim), get_ms(tim));
}

void TIM14_IRQHandler(void) {
}
//********************************************************************
// END OF PROGRAM
//********************************************************************
