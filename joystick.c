/*
 * joystick.c
 *
 * Created: 08.07.2025 18:35:11
 *  Author: yousef
 */ 
#include "joystick.h"
#include <avr/io.h>

void js_init(void){
	DDRA &= ~((1<<PA5)|(1<<PA6)|(1<<PA7)); //eingang
	PORTA |= (1<<PA7); // Aktiviere pullup widerstand
	ADMUX = (1<<REFS0); // VCC referenzspannung
	ADCSRA = (1<<ADEN) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); // ADC aktivieren und aud 128 setzen
}



uint16_t js_getHorizontal(void){
	// Auswahl des ADC-Kanals
	ADMUX = (ADMUX & 0xF0) | (1 << MUX2) | (1 << MUX0); // Kanal ADC5

	// messenstarten
	ADCSRA |= (1 << ADSC);

	// warte bis fertig
	while (ADCSRA & (1 << ADSC));

	
	return ADC;
}

uint16_t js_getVertical(void){
	
	ADMUX = (ADMUX & 0xF0) | (1 << MUX2) | (1 << MUX1); // Kanal ADC6

	
	ADCSRA |= (1 << ADSC);

	
	while (ADCSRA & (1 << ADSC));

	
	return ADC;
}

Direction js_getDirection(void){
	uint16_t h = js_getHorizontal();
	uint16_t v = js_getVertical();
	const uint16_t mid = 512;
	const uint16_t tol = 204;  //(1V / 5V) * 1024 = 204
	if(h < mid - tol) return JS_LEFT;
	if(h > mid + tol) return JS_RIGHT;
	if(v < mid - tol) return JS_DOWN;
	if(v > mid + tol) return JS_UP;
	return JS_NEUTRAL;
}

bool js_getButton(void){
	return !(PINA & (1<<PA7));
}








void os_waitForJoystickButtonInput(){
	while (!js_getButton()){
	}
}




void os_waitForNoJoystickButtonInput(){
	while (js_getButton()){
	}
}