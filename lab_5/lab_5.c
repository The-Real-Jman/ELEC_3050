/*==============================================================*/
/* Jonathan Aldridge and Landen Batte                           */
/* ELEC 3040/3050 - Lab 5                                       */
/* Count in two different directions, with buttons to control.  */
/*==============================================================*/

#include "STM32L1xx.h"

// function definitions
void delay();
void small_delay();
// void count();
void pin_setup();
void interrupt_setup();
void EXTI1_IRQHandler();
void update_leds(unsigned char counter);

// global variables
struct{
	int row;
	int column;
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

/*------------------------------------------------*/
/* Initialize GPIO pins used in the program 			*/
/*------------------------------------------------*/

void pin_setup () {
	/* Configure PA0 as input pin to read push button */
	RCC->AHBENR |= 0x01; /* Enable GPIOA clock (bit 0) */
	GPIOA->MODER &= ~(0x0000000C); /* General purpose input mode */
		
	RCC->AHBENR |= 0x02;
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00005500);
	
	GPIOB->PUPDR &= ~(0x0000000FF);
	GPIOB->PUPDR |= (0x000000055);
	
	/* Configure PC8,PC9 as output pins to drive LEDs */
	RCC->AHBENR |= 0x04; /* Enable GPIOC clock (bit 2) */
	GPIOC->MODER &= ~(0x000000FF); /* Clear PC9 - PC0 mode bits */
	GPIOC->MODER |= (0x00000055); /* General purpose output mode*/

}

/*------------------------------------------------*/
/* Initialize interrupt pins used in the program  */
/*------------------------------------------------*/

void interrupt_setup() {
	
	SYSCFG->EXTICR[0] &= 0xFF0F; // Clear PA1
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
	if (keypad1.event) {
		keypad1.event--;
	}
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
/* Function to update leds in waveforms.					*/
/*------------------------------------------------*/

void update_leds(unsigned char count) {
	
	GPIOC->BSRR |= (~count & 0x0F) << 16;
	GPIOC->BSRR |= (count & 0x0F);
	
	
	//GPIOC->BSRR |= 0x0F << 16;	// clear bits
	//GPIOC->BSRR |= count; // write bits
}

/*------------------------------------------------*/
/* Reads the row of the pressed key.							*/
/*------------------------------------------------*/

int read_row() {
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00005500);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR |= (0x00000055);
	
	//small_delay();
	
	int input = GPIOB->IDR & 0xF0;
	switch(input) {
		case 0xE:
				return 0;
		case 0xD:
				return 1;
		case 0xB:
				return 2;
		case 0x7:
				return 3;
		default:
				return -1;
	}
}

/*------------------------------------------------*/
/* Reads the column of the pressed key.						*/
/*------------------------------------------------*/

int read_column() {
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00000055);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x0000FF00);
	GPIOB->PUPDR |= (0x00005500);
	
	//small_delay();
	
	int input = GPIOB->IDR & 0xF0;
	switch(input) {
		case 0xE:
				return 0;
		case 0xD:
				return 1;
		case 0xB:
				return 2;
		case 0x7:
				return 3;
		default:
				return -1;
	}
}

/*------------------------------------------------*/
/* External interrupt 1 function.									*/
/*------------------------------------------------*/

void EXTI1_IRQHandler() {
	EXTI->PR |= 0x0002;
	
	keypad1.row = read_row();
	keypad1.column = read_column();
	
	if (keypad1.row != -1 && keypad1.column != -1){
		keypad1.event = 4;
		update_leds(keypad1.keys[keypad1.row][keypad1.column]);
	}
	
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->PUPDR &= ~(0x0000FF00);
	GPIOB->PUPDR |= (0x00005500);
	
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER &= ~(0x00005500);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR &= (0x00000055);
	
	EXTI->PR |= 0x0002;
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
}

/*------------------------------------------------*/
/* Main program 																	*/
/*------------------------------------------------*/

int main (void) {
	
	pin_setup(); // Set up pins
	interrupt_setup(); // Set up interrupts

	unsigned char counter = 0;
	
	__enable_irq(); // enable interrupts
	
	while (1) {
		delay(); // delay function
		counter = (counter + 1) % 10; // rolls counter from 9 to 0
		// count(); // call function to increment counter
		if (keypad1.event == 0) {
			update_leds(counter);
		}
	} // repeat forever
}