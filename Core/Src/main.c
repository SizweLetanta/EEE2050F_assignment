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
#define DELAY 0.01			   // The time between ticks
#define FREQUENCY 48e6		   // Frequency of the system clock
#define MAX_CLK_COUNT 65536.0  // 2^16

#define FALSE 0
#define TRUE 1
typedef uint8_t flag_t;

typedef struct {
	uint8_t minutes;
	uint8_t seconds;
	uint8_t hundredths;
} time_struct;

//====================================================================
// GLOBAL VARIABLES
//====================================================================
int tim = 0;
char buff[16];	// To store the time displayed on the screen

flag_t startFlag = FALSE, lapFlag = FALSE, stopFlag = FALSE, resetFlag = TRUE;	// flags

uint8_t minutes = 0, seconds = 0, hundredths = 0;

time_struct lapped_time = {0, 0, 0};
//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void initTIM14(void);
void TIM14_IRQHandler(void);
void initGPIO(void);
void display(void);
void checkPB(void);

//====================================================================
// MAIN FUNCTION
//====================================================================
int main(void) {
	init_LCD();
	initTIM14();
	initGPIO();
	while (1) {
		checkPB(); // Check the previous button press
		lcd_command(CLEAR); // clear previous display
		display();
		delay(10000); // Delay to make the time easier to read on the LCD
	}

}  // End of main

//====================================================================
// FUNCTION DEFINITIONS
//====================================================================
void initTIM14(void) {
	// Calculate the prescaler and ARR values
	double cyc_needed = DELAY * FREQUENCY;
	int presc = ceil(cyc_needed / MAX_CLK_COUNT);
	int arr = (int)(cyc_needed / presc) - 1;

	// Initialise the timer
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
	TIM14->PSC |= presc;
	TIM14->ARR |= arr;
	TIM14->DIER |= TIM_DIER_UIE;
	TIM14->CR1 |= TIM_CR1_CEN;
}

void initGPIO(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	// Connects Clock Signal
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 |
					  GPIO_MODER_MODER2 | GPIO_MODER_MODER3);  // we use the ~ because we want 0's instead of 1's

	// Enables Pull Up resistors
	GPIOA->PUPDR |= 0x00000055;
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1 | GPIO_PUPDR_PUPDR2_1 | GPIO_PUPDR_PUPDR3_1);

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// Connects Clock Signal

	// Configures LEDs to output modes
	GPIOB->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0);

	GPIOB->ODR = 0x00000000;  // Ensures LEDs are off
}

void TIM14_IRQHandler(void) {
	TIM14->SR &= ~TIM_SR_UIF;
	++hundredths;

	if (hundredths == 10) {
		hundredths = 0;
		++seconds;
	}
	if (seconds == 60) {
		seconds = 0;
		++minutes;
	}
}

void display(void) {
	if (!startFlag && !lapFlag && !stopFlag && resetFlag) {
		lcd_putstring("Stopwatch");
		lcd_command(LINE_TWO);
		lcd_putstring("Press SW0...");
		GPIOB->ODR = 1 << 3;
	}

	if (startFlag && !lapFlag && !stopFlag && !resetFlag) {
		lcd_putstring("Time");
		lcd_command(LINE_TWO);
		sprintf(buff, "%02d:%02d.%02d", minutes, seconds, hundredths * 10);
		lcd_putstring(buff);
		GPIOB->ODR = 1 << 0;
	}

	if (startFlag && lapFlag && !stopFlag && !resetFlag) {
		lcd_putstring("Time");
		lcd_command(LINE_TWO);
		sprintf(buff, "%02d:%02d.%02d", lapped_time.minutes, lapped_time.seconds, lapped_time.hundredths * 10);
		lcd_putstring(buff);
		GPIOB->ODR = 1 << 1;
	}

	if (startFlag && !lapFlag && stopFlag && !resetFlag) {
		lcd_putstring("Time");
		lcd_command(LINE_TWO);
		sprintf(buff, "%02d:%02d.%02d", minutes, seconds, hundredths * 10);
		lcd_putstring(buff);
		GPIOB->ODR = 1 << 2;
	}
}

void checkPB(void) {
	uint16_t button_pressed = GPIOA->IDR;

	if (!(button_pressed & GPIO_IDR_0)) {
		startFlag = TRUE;
		lapFlag = FALSE;
		stopFlag = FALSE;
		resetFlag = FALSE;

		NVIC_EnableIRQ(TIM14_IRQn);
	}

	if (!(button_pressed & GPIO_IDR_1)) {
		startFlag = TRUE;
		lapFlag = TRUE;
		stopFlag = FALSE;
		resetFlag = FALSE;

		// Store lapped time
		lapped_time.minutes = minutes;
		lapped_time.seconds = seconds;
		lapped_time.hundredths = hundredths;
	}

	if (!(button_pressed & GPIO_IDR_2)) {
		startFlag = TRUE;
		lapFlag = FALSE;
		stopFlag = TRUE;
		resetFlag = FALSE;

		NVIC_DisableIRQ(TIM14_IRQn);
	}

	if (!(button_pressed & GPIO_IDR_3)) {
		startFlag = FALSE;
		lapFlag = FALSE;
		stopFlag = FALSE;
		resetFlag = TRUE;

		NVIC_DisableIRQ(TIM14_IRQn);

		minutes = 0;
		seconds = 0;
		hundredths = 0;
	}
}
//********************************************************************
// END OF PROGRAM
//********************************************************************
