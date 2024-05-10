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
#include <stdio.h>
#include <lcd_stm32f0.c>
#include "math.h"
#include "stm32f0xx.h"
//====================================================================
// GLOBAL CONSTANTS
//====================================================================
#define FALSE 0
#define TRUE 1
typedef uint8_t flag_t;

// Struct to store the lapped time
typedef struct {
	uint8_t minutes;
	uint8_t seconds;
	uint8_t hundredths;
} time_struct;

//====================================================================
// GLOBAL VARIABLES
//====================================================================
char buff[16];	// To store the time displayed on the screen

// Flags for different modes
flag_t startFlag = FALSE;
flag_t lapFlag = FALSE;
flag_t stopFlag = FALSE;
flag_t resetFlag = TRUE;

// Current time is stored in these variables
uint8_t minutes = 0;
uint8_t seconds = 0;
uint8_t hundredths = 0;

time_struct lapped_time = {0, 0, 0};

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================

/**
 * Function to initialise the TIM14 register
 */
void initTIM14(void);

/**
 * Function to handle the TIM14 interrupts by incrementing the time
 */
void TIM14_IRQHandler(void);

/**
 * Initialises the GPIO pins to allow for the mode LEDs and the different
 * buttons to function
 */
void initGPIO(void);

/**
 * function to display time and messages to the LCD and show the mode with the LEDs
 */
void display(void);

/**
 * Function to check which button has been clicked and set the flags accordinlgy
 */
void checkPB(void);

/**
 * Function to convert the time to binary coded decimal and convert the digits to
 * ASCII so that they can be displayed on the LCD
 */
void convert2BCDASCII(const uint8_t min, const uint8_t sec, const uint8_t hund, char* resultPtr);

//====================================================================
// MAIN FUNCTION
//====================================================================
int main(void) {
	initGPIO();	  // Initialise the GPIO pins
	initTIM14();  // Initialise the timer
	init_LCD();	  // Initialise the LCD
	while (1) {
		checkPB();			 // Check the previous button press
		lcd_command(CLEAR);	 // clear previous display
		// char val[4];
		
		// sprintf( buff,"%lu", GPIOA->IDR);
		display();
		// lcd_putstring(buff);
		delay(10000);  // Delay to make the time easier to read on the LCD
	}

}  // End of main

//====================================================================
// FUNCTION DEFINITIONS
//====================================================================
void initTIM14(void) {
	// Calculate the prescaler and ARR values
	int presc = 8;
	int arr = 59999;

	// Initialise the timer
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;  // Enable clock source register
	TIM14->PSC |= presc;				  // Set prescaler value
	TIM14->ARR |= arr;					  // Set the Auto reload register value
	TIM14->DIER |= TIM_DIER_UIE;		  // Enable tim14 interrupts
	TIM14->CR1 |= TIM_CR1_CEN;			  // Start the timer
}

void initGPIO(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;																   // Connects Clock Signal
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3);  // we use the ~ because we want 0's instead of 1's

	GPIOA->MODER |= GPIO_MODER_MODER7_1;
	GPIOA->OTYPER |= GPIO_OTYPER_OT_7;

	// Enables Pull Up resistors//0x00000055;
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0 | GPIO_PUPDR_PUPDR2_0 | GPIO_PUPDR_PUPDR3_0);

	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1 | GPIO_PUPDR_PUPDR2_1 | GPIO_PUPDR_PUPDR3_1);

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;	// Connects Clock Signal

	// Configures LEDs to output modes
	GPIOB->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0);

	GPIOB->ODR = 0x00000000;  // Ensures LEDs are off
}

void TIM14_IRQHandler(void) {
	TIM14->SR &= ~TIM_SR_UIF;  // Clear the flag
	hundredths += 10;		   // Increment the hundredths

	// calculate the seconds and minutes
	if (hundredths == 100) {
		hundredths = 0;
		++seconds;
	}
	if (seconds == 60) {
		seconds = 0;
		++minutes;
	}
}

void convert2BCDASCII(const uint8_t min, const uint8_t sec, const uint8_t hund, char* resultPtr) {
	uint8_t bc_min1 = min / 10;	 // Digit 1
	uint8_t bc_min0 = min % 10;	 // Digit 0

	uint8_t bc_sec1 = sec / 10;	 // Digit 1
	uint8_t bc_sec0 = sec % 10;	 // Digit 0

	uint8_t bc_hund1 = hund / 10;  // Digit 1
	uint8_t bc_hund0 = hund % 10;  // Digit 0

	// convert to asciii
	resultPtr[0] = bc_min1 + 48;
	resultPtr[1] = bc_min0 + 48;
	resultPtr[2] = ':';
	resultPtr[3] = bc_sec1 + 48;
	resultPtr[4] = bc_sec0 + 48;
	resultPtr[5] = '.';
	resultPtr[6] = bc_hund1 + 48;
	resultPtr[7] = bc_hund0 + 48;
	resultPtr[8] = '\0';
}

void display(void) {
	// Reset button pressed
	if (!startFlag && !lapFlag && !stopFlag && resetFlag) {
		lcd_putstring("Stopwatch");
		lcd_command(LINE_TWO);
		lcd_putstring("Press SW0...");
		GPIOB->ODR = 1 << 3;  // LED 3 on
	}

	// Start button pressed
	if (startFlag && !lapFlag && !stopFlag && !resetFlag) {
		lcd_putstring("Time");
		lcd_command(LINE_TWO);
		convert2BCDASCII(minutes, seconds, hundredths, buff);
		lcd_putstring(buff);
		GPIOB->ODR = 1 << 0;  // Set LED0
	}

	// Lap button pressed
	if (startFlag && lapFlag && !stopFlag && !resetFlag) {
		lcd_putstring("Time");
		lcd_command(LINE_TWO);
		convert2BCDASCII(lapped_time.minutes, lapped_time.seconds, lapped_time.hundredths, buff);  // Display the lapped time
		lcd_putstring(buff);
		GPIOB->ODR = 1 << 1;  // Set LED1
	}

	// Stop button pressed
	if (startFlag && !lapFlag && stopFlag && !resetFlag) {
		lcd_putstring("Time");
		lcd_command(LINE_TWO);
		convert2BCDASCII(minutes, seconds, hundredths, buff);
		lcd_putstring(buff);
		GPIOB->ODR = 1 << 2;  // Set LED2
	}
}

void checkPB(void) {
	uint16_t button_pressed = GPIOA->IDR;

	// Switch 0 pressed
	if (!(button_pressed & GPIO_IDR_0)) {
		startFlag = TRUE;
		lapFlag = FALSE;
		stopFlag = FALSE;
		resetFlag = FALSE;
		GPIOA->ODR |= GPIO_ODR_7;

		NVIC_EnableIRQ(TIM14_IRQn);
	}

	// Switch 1 pressed
	if (!(button_pressed & GPIO_IDR_1)) {
		startFlag = TRUE;
		lapFlag = TRUE;
		stopFlag = FALSE;
		resetFlag = FALSE;

		// Store lapped time
		lapped_time.minutes = minutes;
		lapped_time.seconds = seconds;
		lapped_time.hundredths = hundredths;

		GPIOA->ODR &= ~GPIO_ODR_7;
	}

	// Switch 2 pressed
	if (!(button_pressed & GPIO_IDR_2)) {
		startFlag = TRUE;
		lapFlag = FALSE;
		stopFlag = TRUE;
		resetFlag = FALSE;

		NVIC_DisableIRQ(TIM14_IRQn);  // Disable the counter
	}

	// Switch 3 pressed
	if (!(button_pressed & GPIO_IDR_3)) {
		startFlag = FALSE;
		lapFlag = FALSE;
		stopFlag = FALSE;
		resetFlag = TRUE;

		NVIC_DisableIRQ(TIM14_IRQn);  // Disable the counter

		// Reset the time
		minutes = 0;
		seconds = 0;
		hundredths = 0;
	}
}
//********************************************************************
// END OF PROGRAM
//********************************************************************
