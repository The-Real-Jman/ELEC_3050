/*==============================================================*/
/* Jonathan Aldridge and Landen Batte                           */
/* ELEC 3040/3050 - Lab 2                                       */
/* Toggle LED1 while button pressed, with short delay inserted  */
/*==============================================================*/

#include "STM32L1xx.h"

// function definitions
void delay();
void count(unsigned char dir);
void pin_setup();

// global variables
unsigned char counter;

/*------------------------------------------------*/
/* Initialize GPIO pins used in the program */
/*------------------------------------------------*/

void pin_setup () {
	/* Configure PA0 as input pin to read push button */
	RCC->AHBENR |= 0x01; /* Enable GPIOA clock (bit 0) */
	GPIOA->MODER &= ~(0x0000003c); /* General purpose input mode */
	/* Configure PC8,PC9 as output pins to drive LEDs */
	RCC->AHBENR |= 0x04; /* Enable GPIOC clock (bit 2) */
	GPIOC->MODER &= ~(0x000000FF); /* Clear PC3-PC0 mode bits */
	GPIOC->MODER |= (0x00000055); /* General purpose output mode*/
}

int main (void) {
	
	pin_setup();
	
	unsigned char direction = 0; // initial direction
	unsigned char enable = 0; // enable/disable counter
	counter = 0;
	
	while (1) {
		direction = GPIOA->IDR & 0x00000004;
		enable = GPIOA->IDR & 0x00000002;
		if (enable != 0) {
			count(direction);
		}
		delay();
	} // repeat forever
}

/*------------------------------------------------*/
/* Delay function - generates ~0.5 seconds of */
/* delay. */
/*------------------------------------------------*/

void delay () {
	int i,j,n;
	for (i = 0; i < 10; i++) { // outer loop
		for (j = 0; j < 20000; j++) {	// inner loop
			n = j; // dummy operation for single-step test
		} // do nothing
	}
}

/*------------------------------------------------*/
/* Counts through the program. */
/*------------------------------------------------*/

void count (unsigned char dir) {
	if (!dir) {
			counter = (counter + 1) % 10;
	}
	else {
		counter = (counter + (10 - 1)) % 10;
	}
	GPIOC->BSRR |= (~counter & 0x0F) << 16;	// clear bits
	GPIOC->BSRR |= (counter & 0x0F); // write bits
}

/*------------------------------------------------*/
/* Main program */
/*------------------------------------------------*/

