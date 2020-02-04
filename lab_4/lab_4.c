/*==============================================================*/
/* Jonathan Aldridge and Landen Batte                           */
/* ELEC 3040/3050 - Lab 4                                       */
/* Count in two different directions, with buttons to control.  */
/*==============================================================*/

#include "STM32L1xx.h"

// function definitions
void delay();
void small_delay();
void count1();
void pin_setup();
void interrupt_setup();
void EXTI0_IRQHandler();
void EXTI1_IRQHandler();
void update_leds();

// global variables
unsigned char counter1;
unsigned char counter2;
unsigned char count2_dir;
unsigned char led8;
unsigned char led9;

/*------------------------------------------------*/
/* Initialize GPIO pins used in the program 			*/
/*------------------------------------------------*/

void pin_setup () {
	/* Configure PA0 as input pin to read push button */
	RCC->AHBENR |= 0x01; /* Enable GPIOA clock (bit 0) */
	GPIOA->MODER &= ~(0x0000000F); /* General purpose input mode */
	/* Configure PC8,PC9 as output pins to drive LEDs */
	RCC->AHBENR |= 0x04; /* Enable GPIOC clock (bit 2) */
	GPIOC->MODER &= ~(0x000FFFFF); /* Clear PC9 - PC0 mode bits */
	GPIOC->MODER |= (0x00055555); /* General purpose output mode*/
}

/*------------------------------------------------*/
/* Initialize interrupt pins used in the program  */
/*------------------------------------------------*/

void interrupt_setup() {
	SYSCFG->EXTICR[0] &= ~(0x000F); //Clear & set PA0
	
	SYSCFG->EXTICR[0] &= ~(0x00F0); // Clear PA1
	SYSCFG->EXTICR[0] |= 0x0010; // Set PA1
	
	EXTI->RTSR |= 0x0003; // Set EXTI0 & EXTI1 to rising-edge
	
	EXTI->IMR |= 0x0003; // Enable EXTI0 & EXTI1
	
	EXTI->PR |= 0x0003; // clear pending status
	
	NVIC_EnableIRQ(EXTI0_IRQn); // enable EXTI0 interrupt
	NVIC_EnableIRQ(EXTI1_IRQn); // enable EXTI1 interrupt
	
	NVIC_ClearPendingIRQ(EXTI0_IRQn); // clear pending flag for EXTI0
	NVIC_ClearPendingIRQ(EXTI1_IRQn); // clear pending flag for EXTI1
}



/*------------------------------------------------*/
/* Delay function - generates ~0.5 seconds of 		*/
/* delay. 																				*/
/*------------------------------------------------*/

void delay () {
	int i,j,n;
	for (i = 0; i < 10; i++) { // outer loop
		for (j = 0; j < 20000; j++) {	// inner loop
			n = j; // dummy operation for single-step test
		} // do nothing
	}
}

// Small delay method to attempt to fix some button debouncing
void small_delay() {
	int i;
	for (i = 0; i < 10000; i++) {
		asm("nop");
	}
}


/*------------------------------------------------*/
/* Counts through the program. 										*/
/*------------------------------------------------*/

void count1 () {
		counter1 = (counter1 + 1) % 10; // rolls counter from 9 to 0
}

void count2() {
	if (count2_dir){ // if counting up
		counter2 = (counter2 + 1) % 10; // rolls counter from 9 to 0
	}
	else{
		counter2 = (counter2 + (10 - 1)) % 10; //  rolls counter from 0 to 9
	}
}

/*------------------------------------------------*/
/* Function to update leds in waveforms.					*/
/*------------------------------------------------*/

void update_leds() {
	GPIOC->BSRR |= 0x0F << 16;	// clear bits
	GPIOC->BSRR |= counter1; // write bits
	
	GPIOC->BSRR |= 0xF0 << 16; // clear bits
	GPIOC->BSRR |= counter2 << 4; // write bits
}

/*------------------------------------------------*/
/* External interrupt 0 function.									*/
/*------------------------------------------------*/

void EXTI0_IRQHandler() {
	EXTI->PR |= 0x0001;
	count2_dir = 0;
	if (led8) {
		GPIOC->BSRR |= 0x0100 << 16; // update led8 to turn off
		led8 = 0;
	}
	else {
		GPIOC->BSRR |= 0x0100; //  update led8 to turn on
		led8 = 1;
	}
	small_delay();
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
}

/*------------------------------------------------*/
/* External interrupt 1 function.									*/
/*------------------------------------------------*/

void EXTI1_IRQHandler() {
	EXTI->PR |= 0x0002;
	count2_dir = 1;
	if(led9){
		GPIOC->BSRR |= 0x0200 << 16; // update led9 to turn off
		led9 = 0;
	}
	else {
		GPIOC->BSRR |= 0x0200; // update led9 to turn off
		led9 = 1;
	}
	small_delay();
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
}

/*------------------------------------------------*/
/* Main program 																	*/
/*------------------------------------------------*/

int main (void) {
	
	pin_setup(); // Set up pins
	interrupt_setup(); // Set up interrupts

	counter1 = 0;
	counter2 = 0;
	count2_dir = 1;
	GPIOC->BSRR |= 0x0100 << 16; // set LED8 to off
	led8 = 0;
	GPIOC->BSRR |= 0x0200; // set LED9 to on
	led9 = 1;
	
	__enable_irq(); // enable interrupts
	
	while (1) {
		delay(); // delay function
		count1(); // call function to increment counter1
		update_leds(); // update leds in waveforms
		delay(); // delay function
		count1(); // call function to increment counter1
		count2(); // call function to increment/decrement counter2
		update_leds(); // update leds in waveforms
	} // repeat forever
}