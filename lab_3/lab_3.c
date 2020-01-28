/*==============================================================*/
/* Jonathan Aldridge and Landen Batte                           */
/* ELEC 3040/3050 - Lab 3                                       */
/* Count in two different directions, with switches to control. */
/*==============================================================*/

#include "STM32L1xx.h"

// function definitions
void delay();
void count(unsigned char dir);
void pin_setup();

// global variables
unsigned char counter1;
unsigned char counter2;

/*------------------------------------------------*/
/* Initialize GPIO pins used in the program */
/*------------------------------------------------*/

void pin_setup () {
	/* Configure PA0 as input pin to read push button */
	RCC->AHBENR |= 0x01; /* Enable GPIOA clock (bit 0) */
	GPIOA->MODER &= ~(0x0000003c); /* General purpose input mode */
	/* Configure PC8,PC9 as output pins to drive LEDs */
	RCC->AHBENR |= 0x04; /* Enable GPIOC clock (bit 2) */
	GPIOC->MODER &= ~(0x0000FFFF); /* Clear PC3-PC0 & PC7-PC4 mode bits */
	GPIOC->MODER |= (0x00005555); /* General purpose output mode*/
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
			counter1 = (counter1 + 1) % 10; // rolls counter from 9 to 0
			counter2 = (counter2 + (10 - 1)) % 10; // rolls counter from 0 to 9
	}
	else {
		counter1 = (counter1 + (10 - 1)) % 10; // rolls counter from 9 to 0
		counter2 = (counter2 + 1) % 10; // rolls counter from 0 to 9
	}
	GPIOC->BSRR |= (~counter1 & 0x0F) << 16;	// clear bits
	GPIOC->BSRR |= (counter1 & 0x0F); // write bits
	
	GPIOC->BSRR |= 0xF0 << 16; // clear bits
	GPIOC->BSRR |= counter2 << 4; // write bits
}

/*------------------------------------------------*/
/* Main program */
/*------------------------------------------------*/

int main (void) {
	
	pin_setup();
	
	unsigned char direction = 0; // initial direction
	unsigned char enable = 0; // enable/disable counter
	counter1 = 0;
	counter2 = 0;
	
	while (1) {
		direction = GPIOA->IDR & 0x00000004;
		enable = GPIOA->IDR & 0x00000002;
		if (enable != 0) {
			count(direction);
		}
		delay();
	} // repeat forever
}