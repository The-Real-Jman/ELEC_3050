/*==============================================================*/
/* Jonathan Aldridge and Landen Batte */
/* ELEC 3040/3050 - Lab 8 */
/* Output a PWM signal based on keypad button press. */
/*==============================================================*/

#include "STM32L1xx.h"

/*------------------------------------------------*/
/* Global variable & structures. */
/*------------------------------------------------*/

// function definitions
void small_delay();
void pin_setup();
void interrupt_setup();
void timer_setup();
int read_row();
int read_column();
void EXTI1_IRQHandler();

// Define the keypad struct 
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

// --- Global variables ---

// detects if a key is pressed
int pressed = 0;

// Instantiates the keypad
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
/* Initialize GPIO pins used in the program. */
/*------------------------------------------------*/

void pin_setup () {
	/* Configure PA0 as input pin to read push button */
	RCC->AHBENR |= 0x01; /* Enable GPIOA clock (bit 0) */
	GPIOA->MODER &= ~(0x0000000C); /* General purpose input mode */
		
	RCC->AHBENR |= 0x02;
	GPIOB->MODER &= ~(0x0000FF00);
	GPIOB->MODER |= (0x00005500);
	GPIOB->ODR = 0;
	
	GPIOB->PUPDR &= ~(0x0000000FF);
	GPIOB->PUPDR |= (0x000000055);
	
	/* Configure PC[0 - 7] as output pins to drive LEDs */
	RCC->AHBENR |= 0x04; /* Enable GPIOC clock (bit 2) */
	GPIOC->MODER &= ~(0x0000FFFF); /* Clear PC7 - PC0 mode bits */
	GPIOC->MODER |= (0x00005555); /* General purpose output mode*/

	/* Change PA6 to altrnative function mode */
  MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER6, 0x00002000);
  
  /* Change the alternatice function to be the CC*/
  MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFRL6, 0x03000000);
}

/*------------------------------------------------*/
/* Initialize interrupt pins used in the program  */
/*------------------------------------------------*/

void interrupt_setup() {
	
	SYSCFG->EXTICR[0] &= 0xFF0F; // Clear PA1
	SYSCFG->EXTICR[0] |= 0x0010; // Set PA1
	
	EXTI->FTSR |= 0x0002; // Set EXTI1 to rising-edge
	
	EXTI->IMR |= 0x0002; // Enable EXTI1
	
	EXTI->PR |= 0x0002; // clear pending status

	NVIC_EnableIRQ(EXTI1_IRQn); // enable EXTI1 interrupt
	
	NVIC_ClearPendingIRQ(EXTI1_IRQn); // clear pending flag for EXTI1
}

/*------------------------------------------------*/
/* Initialize timers used in the program  */
/*------------------------------------------------*/

void timer_setup() {
  RCC->CR |= RCC_CR_HSION; // Turn on 16MHz HSI oscillator
  while ((RCC->CR & RCC_CR_HSIRDY) == 0); // Wait until HSI ready
  RCC->CFGR |= RCC_CFGR_SW_HSI; // Select HSI as system clock
  
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM10EN); //enable clock source
  TIM10->ARR = 99; //set auto reload. assumes 16MHz
  TIM10->PSC = 159; //set prescale.
  TIM10->CCR1 = 10; //Set compare value
  TIM10->CNT = 0;
  MODIFY_REG(TIM10->CCMR1, TIM_CCMR1_CC1S, 0x0000); // Capture compare select
  MODIFY_REG(TIM10->CCMR1, TIM_CCMR1_OC1M, 0x0060); // Active to inactive
  SET_BIT(TIM10->CCER, TIM_CCER_CC1E); // drive output pin 
  SET_BIT(TIM10->CR1, TIM_CR1_CEN); //enable counting
}

/*------------------------------------------------*/
/* Small delay function to fix keypad button */
/* debouncing. */
/*------------------------------------------------*/

void small_delay() {
	int i;
	for (i = 0; i < 100; i++) {
		asm("nop");
	}
}

/*------------------------------------------------*/
/* Reads the row of the pressed key. */
/*------------------------------------------------*/

int read_row() {
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00005500);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR |= (0x00000055);
	
	small_delay();
	
	int input = GPIOB->IDR & 0xF;
	switch(input) {
		case 0xE:
				return 0; // row 4
		case 0xD:
				return 1; // row 3
		case 0xB:
				return 2; // row 2
		case 0x7:
				return 3; // row 1
		default:
				return -1;
	}
}

/*------------------------------------------------*/
/* Reads the column of the pressed key.	*/
/*------------------------------------------------*/

int read_column() {
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00000055);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x0000FF00);
	GPIOB->PUPDR |= (0x00005500);
	
	small_delay();
	
	int input = GPIOB->IDR&0xF0;
	switch(input) {
		case 0xE0:
				return 0; // column 4
		case 0xD0:
				return 1; // column 3
		case 0xB0:
				return 2; // column 2
		case 0x70:
				return 3; // column 1
		default:
				return -1;
	}
}

/*------------------------------------------------*/
/* External interrupt 1 function. */
/*------------------------------------------------*/

void EXTI1_IRQHandler() {
	EXTI->PR |= 0x0002;
	
	// Find row pressed
	keypad1.row = read_row();
	// Find column pressed
	keypad1.column = read_column();
	
	// If a valid key was pressed
	if(keypad1.row != -1 && keypad1.column != -1){
		// Set event to the value of key pressed
		keypad1.event = keypad1.keys[keypad1.row] [keypad1.column];
		// Key was pressed
		pressed = 1;
		// Update LEDs
		GPIOC->ODR = keypad1.event;
	}
	
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->PUPDR &= ~(0x0000FF00);
	GPIOB->PUPDR |= (0x00005500);
	
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER &= ~(0x00005500);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR &= (0x00000055);
	
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	EXTI->PR |= 0x0002;
}

/*------------------------------------------------*/
/* Main function */
/*------------------------------------------------*/

int main (void) {
	
	pin_setup(); // Set up pins
	interrupt_setup(); // Set up interrupts
	timer_setup(); // Set up timers
	
	__enable_irq(); // enable interrupts
	
	while (1) {
		// Checks if a button was pressed and if that button was 0 - A
		if (keypad1.event < 11 && pressed == 1) {
			// Sets duty cycle based on key pressed
			TIM10->CCR1 = keypad1.event * (TIM10->ARR + 1) / 10;
			pressed = 0;
		}
	} // repeat forever
}