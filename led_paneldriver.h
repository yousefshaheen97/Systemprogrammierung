/*! \file
 *  \brief Low level functions to draw premade things on the LED Panel
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */
#ifndef _LED_PANELDRIVER_H
#define _LED_PANELDRIVER_H
#include <avr/io.h>
#include <stdbool.h>


//! Initializes registers
void panel_init();

//! Starts interrupts
void panel_startTimer(void);

//! Stops interrupts
void panel_stopTimer(void);

//! Initalizes interrupt timer
void panel_initTimer(void);


// additional helper functions for LED matrix control
void panel_latchEnable(void);
void panel_latchDisable(void);
void panel_outputEnable(void);
void panel_outputDisable(void);
void panel_setAddress(uint8_t row);
void panel_setOutput(uint8_t data);
void panel_CLK(void);


#define NUM_PLANES 3
#define NUM_DROWS 16
#define NUM_COLS 32




#endif
