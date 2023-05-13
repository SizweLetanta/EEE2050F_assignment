//********************************************************************
//*                    EEE2046F C template                           *
//*==================================================================*
//* WRITTEN BY: Sizwe Letanta   	                 		         *
//* DATE CREATED: 11/05/2023                                         *
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
#define DELAY 0.001 // The time between ticks
#define FREQUENCY 48e6 // Frequency of the system clock
#define MAX_CLK_COUNT 65536.0 // 2^16

#define FALSE 0
#define TRUE 1
typedef u_int8_t flag_t;

//====================================================================
// GLOBAL VARIABLES
//====================================================================
int tim = 0;
char buff[16]; // To store the time displayed on the screen

flag_t startFlag = FALSE, lapFlag = FALSE, stopFlag = FALSE, resetFlag = TRUE; // flags

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void initTIM14(void);
void TIM14_IRQHandler(void);
void initGPIO(void);

//====================================================================
// MAIN FUNCTION
//====================================================================
int main(void) {
	init_LCD();
	initTIM14();
	while (1) {

	}

}  // End of main

//====================================================================
// FUNCTION DEFINITIONS
//====================================================================
void initTIM14(void) {

	// Calculate the prescaler and ARR values
	double cyc_needed = DELAY * FREQUENCY;
	int presc = (int)(cyc_needed / MAX_CLK_COUNT);
	int arr = floor(cyc_needed / presc) - 1;

	// Initialise the timer
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
	TIM14->PSC |= presc;
	TIM14->ARR |= arr;
	TIM14->DIER |= TIM_DIER_UIE;
	TIM14->CR1 |= TIM_CR1_CEN;

	NVIC_EnableIRQ(TIM14_IRQn);
}

void initGPIO(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  // Connects Clock Signal
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 |
					  GPIO_MODER_MODER2 | GPIO_MODER_MODER3);  // we use the ~ because we want 0's instead of 1's

	// Enables Pull Up resistors
	GPIOA->PUPDR |= 0x00000055;
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1 | GPIO_PUPDR_PUPDR2_1 | GPIO_PUPDR_PUPDR3_1);

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// Connects Clock Signal

	// Configures LEDs to output modes
	GPIOB->MODER |= (GPIO_MODER_MODER0_0
					|GPIO_MODER_MODER1_0
					|GPIO_MODER_MODER2_0
					|GPIO_MODER_MODER3_0);

	GPIOB->ODR = 0x00000000;  // Ensures LEDs are off
}

void TIM14_IRQHandler(void) {
	++tim;
	TIM14->SR &= ~TIM_SR_UIF;
	if (!(tim % 21)) {
		get_time(tim, buff);
		lcd_command(CLEAR);
		lcd_putstring("Hello");
		lcd_command(LINE_TWO);
		lcd_putstring(buff);
	}
}
//********************************************************************
// END OF PROGRAM
//********************************************************************
