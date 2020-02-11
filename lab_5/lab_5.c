/*==============================================================*/
/* Jonathan Aldridge and Landen Batte                           */
/* ELEC 3040/3050 - Lab 5                                       */
/* Count in two different directions, with buttons to control.  */
/*==============================================================*/

#include "STM32L1xx.h"

// function definitions
void delay();
void small_delay();
void count(unsigned char counter);
void pin_setup();
void interrupt_setup();
void EXTI1_IRQHandler();
void update_leds(unsigned char counter);

// global variables
struct{
	unsigned char row;
	unsigned char column;
	unsigned char event;
	unsigned char row1[4];
	unsigned char row2[4];
	unsigned char row3[4];
	unsigned char row4[4];
	const unsigned char* keys[];
} typedef matrix_keypad;

matrix_keypad keypad1 = {
	.row = ~0,
	.column = ~0,
	.event = 0,
	.row1 = {1, 2, 3, 0xA},
	.row2 = {4, 5, 6, 0xB},
	.row3 = {7, 8, 9, 0xC},
	.row4 = {0xE, 0, 0xF, 0xD},
	.keys = {keypad1.row1, keypad1.row2, keypad1.row3, keypad1.row4},
};

int bounce = 0;

/*------------------------------------------------*/
/* Initialize GPIO pins used in the program 			*/
/*------------------------------------------------*/

void pin_setup () {
	/* Configure PA0 as input pin to read push button */
	RCC->AHBENR |= 0x01; /* Enable GPIOA clock (bit 0) */
	GPIOA->MODER &= ~(0x0000000C); /* General purpose input mode */
		
	RCC->AHBENR |= 0x02;
	GPIOB->MODER &= ~(0x0000FFFF);
	
	GPIOB->PUPDR &= ~(0x0000000FF);
	GPIOB->PUPDR |= (0x000000055);
	
	GPIOB->PUPDR &= ~(0x00000FF00);
	GPIOB->PUPDR |= (0x000005500);
	
	/* Configure PC8,PC9 as output pins to drive LEDs */
	RCC->AHBENR |= 0x04; /* Enable GPIOC clock (bit 2) */
	GPIOC->MODER &= ~(0x000000FF); /* Clear PC9 - PC0 mode bits */
	GPIOC->MODER |= (0x00000055); /* General purpose output mode*/

}

/*------------------------------------------------*/
/* Initialize interrupt pins used in the program  */
/*------------------------------------------------*/

void interrupt_setup() {
	
	SYSCFG->EXTICR[0] &= ~(0x00F0); // Clear PA1
	SYSCFG->EXTICR[0] |= 0x0010; // Set PA1
	
	EXTI->FTSR |= 0x0002; // Set EXTI0 & EXTI1 to rising-edge
	
	EXTI->IMR |= 0x0002; // Enable EXTI0 & EXTI1
	
	EXTI->PR |= 0x0002; // clear pending status

	NVIC_EnableIRQ(EXTI1_IRQn); // enable EXTI1 interrupt
	
	NVIC_ClearPendingIRQ(EXTI1_IRQn); // clear pending flag for EXTI1
}



/*------------------------------------------------*/
/* Delay function - generates ~0.5 seconds of 		*/
/* delay. 																				*/
/*------------------------------------------------*/

void delay () {
	int i,j,n;
	for (i = 0; i < 20; i++) { // outer loop
		for (j = 0; j < 20000; j++) {	// inner loop
			n = j; // dummy operation for single-step test
		} // do nothing
	}
}

// Small delay method to attempt to fix some button debouncing
void small_delay() {
	int i;
	for (i = 0; i < 10; i++) {
		asm("nop");
	}
}


/*------------------------------------------------*/
/* Counts through the program. 										*/
/*------------------------------------------------*/

void count (unsigned char counter) {
		counter = (counter + 1) % 10; // rolls counter from 9 to 0
}

/*------------------------------------------------*/
/* Function to update leds in waveforms.					*/
/*------------------------------------------------*/

void update_leds(unsigned char counter) {
	GPIOC->BSRR |= 0x0F << 16;	// clear bits
	GPIOC->BSRR |= counter; // write bits
}

/*------------------------------------------------*/
/* External interrupt 1 function.									*/
/*------------------------------------------------*/

void EXTI1_IRQHandler() {
	EXTI->PR |= 0x0002;
	
	const int COLUMN_MASK[] = {(GPIOB->BSRR & 0x10), GPIOB->BSRR & 0x20), GPIOB->BSRR & 0x40), GPIOB->BSRR & 0x80)};
	const int ROW_MASK[] = {(GPIOB->IDR & 0x01), GPIOB->IDR & 0x02), GPIOB->IDR & 0x04), GPIOB->IDR & 0x08)};
	
	for (keypad1.column = 0; keypad1.column < 4; keypad1.column++){
		GPIOB->BSRR |= 0x000000F0;
		GPIOB-> &= COLUMN_MASK[keypad1.column];
		
		for (keypad1.row = 0; keypad1.row < 4; keypad1.row++){
			small_delay();
			unsigned short temp = (GPIOB->IDR & ROW_MASK[keypad1.row]);
			
			if(!temp) {
				keypad1.event = 4;
				update_leds(keypad1.keys[keypad1.row][keypad1.column]);
				GPIOB->BSRR |= 0x00F00000;
				NVIC_ClearPendingIRQ(EXTI1_IRQn);
				return;
			}
		}
	}
	
	GPIOB->BSRR |= 0x00F00000;
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
}

/*------------------------------------------------*/
/* Main program 																	*/
/*------------------------------------------------*/

int main (void) {
	
	pin_setup(); // Set up pins
	interrupt_setup(); // Set up interrupts

	unsigned char counter = 0;
	GPIOB->BSRR |= 0x00F00000;
	
	__enable_irq(); // enable interrupts
	
	while (1) {
		delay(); // delay function
		count(counter); // call function to increment counter
		if (keypad1.event) {
			keypad1.event--;
		}
		else {
			update_leds(counter); // update leds in waveforms
		}
	} // repeat forever
}