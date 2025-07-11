/*! \file
 *  \brief Functions to draw premade things on the LED Panel
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */
#include "led_paneldriver.h"

#include "defines.h"
#include "util.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>


//! \brief Enable compare match interrupts for Timer 1
void panel_startTimer() {
    sbi(TIMSK1, OCIE1A);
}

//! \brief Disable compare match interrupts for Timer 1
void panel_stopTimer() {
    cbi(TIMSK1, OCIE1A);
}

//! \brief Initialization function of Timer 1
void panel_initTimer() {
    // Configuration TCCR1B register
    sbi(TCCR1B, WGM12); // Clear on timer compare match
    sbi(TCCR1B, CS12);  // Prescaler 256  1
    cbi(TCCR1B, CS11);  // Prescaler 256  0
    cbi(TCCR1B, CS10);  // Prescaler 256  0

    // Output Compare register 256*7 = 1792 tics => interrupt interval approx 0.0896 ms
    OCR1A = 0x0007;
}

static uint8_t currentRow = 0;
static uint8_t currentPlane = 0;
uint8_t frameBuffer[NUM_PLANES][NUM_DROWS][NUM_COLS];


//! \brief Initializes used ports of panel
void panel_init(){
	
		
		
		DDRA |= (1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3);  // Row select pins
		
		DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC6);               // CLK, LE, OE pins
		DDRD |= (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5);  // RGB data pins
		
		
		

	
	
//#error IMPLEMENT STH. HERE
}

void panel_latchEnable(){ 
	
	PORTC |= (1 << PC1); 
	}
	
void panel_latchDisable(){ 
	
	PORTC &= ~(1 << PC1); 
	}
	
void panel_outputEnable(){ 
	PORTC &= ~(1 << PC6); 
	}
	
void panel_outputDisable(){ 
	PORTC |= (1 << PC6); 
 }
 
void panel_setAddress(uint8_t row){
	
	PORTA = (PORTA & 0xF0) | (row & 0x0F); 
}

void panel_setOutput(uint8_t data){
	
	PORTD = (0b00111111 & data); 
}

void panel_CLK(){

	PORTC |= (1 << PC0); 
	PORTC &= ~(1 << PC0); 
}




//! \brief ISR to refresh LED panel, trigger 1 compare match interrupts
ISR(TIMER1_COMPA_vect) {
	
	panel_outputDisable();

	
	panel_setAddress(currentRow);
	

	for (uint8_t col = 0; col < NUM_COLS; col++) {
		
		uint8_t data = frameBuffer[currentPlane][currentRow][col];
		panel_setOutput(data);
		panel_CLK(); 
	}
	
	

	panel_latchEnable();
	panel_latchDisable();

	
	panel_outputEnable();

	
	currentRow++;
	if (currentRow >= NUM_DROWS) {
		currentRow = 0;
		currentPlane++;
		if (currentPlane >= NUM_PLANES) {
			currentPlane = 0;
		}
	}
	
	
//#error IMPLEMENT STH. HERE
}
